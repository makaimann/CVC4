#include "theory/matrix/theory_matrix.h"
#include <ginac/ginac.h>

using namespace std;

namespace CVC4 {
namespace theory {
namespace matrix {

/** Constructs a new instance of TheoryMatrix w.r.t. the provided contexts. */
TheoryMatrix::TheoryMatrix(context::Context* c,
                           context::UserContext* u,
                           OutputChannel& out,
                           Valuation valuation,
                           const LogicInfo& logicInfo) :
    Theory(THEORY_MATRIX, c, u, out, valuation, logicInfo) {
}/* TheoryMatrix::TheoryMatrix() */

void TheoryMatrix::check(Effort level) {
  if (done() && !fullEffort(level)) {
    return;
  }

  TimerStat::CodeTimer checkTimer(d_checkTime);

  std::vector<TNode> assignmentTNodes;
  std::vector<TNode> otherAssertionTNodes;
  std::map<std::pair<unsigned, unsigned>, std::vector<TNode> > indexTNodes;
  std::map<TNode, GiNaC::matrix > matrices;
  std::map<unsigned, std::vector<TNode> > vectorindexTNodes;
  std::map<TNode, GiNaC::matrix > vectors;

  //build matrices
  for(assertions_iterator i = facts_begin(); i != facts_end(); ++i) {
    TNode fact = (*i).assertion;
    if(done()) {return;}
    
    switch(fact.getKind()) {

    case kind::EQUAL:
      {
        bool keep = processIndexAssignments(matrices, indexTNodes, vectors, vectorindexTNodes, fact);
        if(keep) {
          otherAssertionTNodes.push_back(fact);
        }
        break;
      }
    case kind::NOT:
      {
        //Handle not matrix index assignments
        if(fact[0].getKind() == kind::EQUAL) {
          if(fact[0].getNumChildren() > 0 && fact[0][0].getKind() == kind::MATRIX_INDEX) {
            //Just don't handle the not matrix assignments
            break;
          }
        }
        //End not matrix index assignments
      }
    default:
      otherAssertionTNodes.push_back(fact);
      //Unhandled(fact.getKind());
    }
  }

    
  //check for conflicts in matrix index assertions
  bool index_conflicts = false;
  NodeBuilder<> conjunction(kind::AND);
  std::map<std::pair<unsigned, unsigned>, std::vector<TNode> >::iterator it;
  for(it = indexTNodes.begin(); it != indexTNodes.end(); ++it) {
    std::vector<TNode> assertionSet = it->second;
    if(assertionSet.size() > 1) {
      index_conflicts = true;
      for(unsigned i = 0; i < assertionSet.size(); ++i) {
        conjunction << assertionSet[i];
      }
    }
  }

  std::map<unsigned, std::vector<TNode> >::iterator v_it;
  for(v_it = vectorindexTNodes.begin(); v_it != vectorindexTNodes.end(); ++v_it) {
    std::vector<TNode> assertionSet = v_it->second;
    if(assertionSet.size() > 1) {
      index_conflicts = true;
      for(unsigned i = 0; i < assertionSet.size(); ++i) {
        conjunction << assertionSet[i];
      }
    }
  }

  //return conflicts if multiple indices asserted to be different values
  if(index_conflicts) {
    Node conflict = conjunction;
    d_out->conflict(conflict);
    return;
  }

  
  //print matrix
  Debug("matrix") << "TheoryMatrix::check(): Printing parsed matrices " << endl;
  //print matrices here
  std::map<TNode, GiNaC::matrix >::iterator matrix_it;
  for(matrix_it = matrices.begin(); matrix_it != matrices.end(); ++matrix_it) {
    GiNaC::matrix m = matrix_it->second;
    for(unsigned int i = 0; i < m.rows(); ++i) {
      for(unsigned int j = 0; j < m.cols(); ++j) {
        Debug("matrix")  << m(i, j) << " ";

        //stop checking if incomplete matrix
        if(m(i, j) == 0x8000000000000)
        {
          Debug("matrix") << "\nExiting check because matrix currently incomplete" << std::endl;
          return;
        }
      }
      Debug("matrix") << endl;
    }
  }
  //end print matrix
  
  
  bool t_sat = true;
  for(unsigned i = 0; i < otherAssertionTNodes.size(); ++i) {
    TNode n = otherAssertionTNodes[i];
    // instead of enumerating outer cases,
    // check if it's a rank comparison then blow out cases
    if(n.getNumChildren() > 1 && n[0].getKind() == kind::MATRIX_RANK) {
      t_sat = checkRank(matrices, n);
    }
    else if (n.getNumChildren() > 1 && n[0].getKind() == kind::MATRIX_DET) {
      t_sat = checkDet(matrices, n);
    }
    else if (n.getKind() == kind::VECTOR_IN_RANGE) {
      t_sat = checkVrange(matrices, vectors, n);
    }
    else {
      Unhandled(n.getKind());
    }

    if(!t_sat) {
      NodeBuilder<> conjunction(kind::AND);
      // include all index assignments
      std::map<std::pair<unsigned, unsigned>, std::vector<TNode> >::iterator it;
      for(it = indexTNodes.begin(); it != indexTNodes.end(); ++it) {
        std::vector<TNode> assertionNode = it->second;
        conjunction << assertionNode[0];
      }
        
      // and include the rank assertion
      conjunction << n;
      //return conflicts if multiple indices asserted to be different values
      Node conflict = conjunction;
      d_out->conflict(conflict);
      return;
    }
    
  }/* for assertion in assertions */

  return;

}/* TheoryMatrix::check() */


bool TheoryMatrix::processIndexAssignments(std::map<TNode, GiNaC::matrix > & matrices,
                                           std::map<std::pair<unsigned, unsigned>, std::vector<TNode> > & indexTNodes,
                                           std::map<TNode, GiNaC::matrix > & vectors,
                                           std::map<unsigned, std::vector<TNode> > & vectorindexTNodes,
                                           TNode & n)
{
  Debug("matrix") << "n[0] = " << n[0] << endl;
  Debug("matrix") << "n[0].getKind() = " << n[0].getKind() << endl;
  //  cout << "with kind = " << n[0].getKind() << endl;
  if(n[0].getKind() == kind::MATRIX_TYPE) {
    //if assigning whole matrix
    if(n[1].getKind() == kind::CONST_MATRIX) {
      std::vector< std::vector<double> > dm = n[1].getConst<Matrix>().getDoubleValues();
      matrices[n[0]] = GiNaC::matrix(dm.size(), dm[0].size());
      for(unsigned i = 0; i < dm.size(); ++i) {
        for(unsigned j = 0; j < dm[0].size(); ++j) {
          matrices[n[0]](i, j) = dm[i][j];
        }
      }
      return false;
    }
  }
  //if indexing
  else if (n[0].getKind() == kind::MATRIX_INDEX) {
    MatrixIndex mindex = n[0].getOperator().getConst<MatrixIndex>();
    
    //already in matrices std::map
    if(matrices.count(n[0][0]) == 1) {
      // this should always just be a single value...
      if(n[1].getKind() == kind::CONST_MATRIX) {
        std::vector<std::vector<double> > val = n[1].getConst<Matrix>().getDoubleValues();
      
        // need to check if this element already assigned
        if(matrices[n[0][0]](mindex.row,mindex.col) != 0x8000000000000) {
          //do something because asserted to indices were different numbers
          if(matrices[n[0][0]](mindex.row, mindex.col) != val[0][0]) {
            cout << "Asserting multiple values for same indices" << endl;
            std::pair<unsigned, unsigned> p = std::make_pair(mindex.row, mindex.col);
            indexTNodes[p].push_back(n);
            return false;
          }
        }
        matrices[n[0][0]](mindex.row, mindex.col) = val[0][0];
        std::pair<unsigned, unsigned> p = std::make_pair(mindex.row, mindex.col);
        std::vector<TNode> assertionSet; assertionSet.push_back(n);
        indexTNodes.insert(std::make_pair(p, assertionSet));
        return false;
      }
      else if(n[1].getKind() == kind::MATRIX_INDEX) {
        MatrixIndex mindex2 = n[1].getOperator().getConst<MatrixIndex>();
        // ignore this case -- still wait until have all indices
        // this is just an extra clause added by CVC4 Equivalence Classes...
        //        GiNaC::symbol a("a");
        //        matrices[n[0][0]](mindex.row, mindex.col) = a;
        //        matrices[n[0][0]](mindex2.row, mindex2.col) = a;
        return false;
      }

    }
    else {
      std::vector<unsigned> dimensions = n[0][0].getType(true).getMatrixDim();
      GiNaC::matrix values(dimensions[0], dimensions[1]);
      for(unsigned i = 0; i < dimensions[0]; ++i) {
        for(unsigned j = 0; j < dimensions[1]; ++ j) {
          values(i,j) = 0x8000000000000;
        }
      }

      //insert into map
      matrices.insert(std::pair<TNode, GiNaC::matrix>(n[0][0], values));
      
      if(n[1].getKind() == kind::CONST_MATRIX) {
        std::vector<std::vector<double> > val = n[1].getConst<Matrix>().getDoubleValues();
        matrices[n[0][0]](mindex.row, mindex.col) = val[0][0];
        std::pair<unsigned, unsigned> p = std::make_pair(mindex.row, mindex.col);
        std::vector<TNode> assertionSet; assertionSet.push_back(n);
        indexTNodes.insert(std::make_pair(p, assertionSet));
        return false;
      }
      else if (n[1].getKind() == kind::MATRIX_INDEX) {
        MatrixIndex mindex2 = n[1].getOperator().getConst<MatrixIndex>();
        // Not what I should actually do...
        //        GiNaC::symbol a("a");
        //        matrices[n[0][0]](mindex.row, mindex.col) = a;
        //        matrices[n[0][0]](mindex2.row, mindex2.col) = a;
      }
    }
  }
  // handle vectors
  else if (n[0].getKind() == kind::VECTOR_TYPE) {
        //if assigning whole vector
    if(n[1].getKind() == kind::CONST_VECTOR) {
      std::vector<double> dv = n[1].getConst<Vector>().getDoubleValues();
      vectors[n[0]] = GiNaC::matrix(dv.size(), 1);
      for(unsigned i = 0; i < dv.size(); ++i) {
          vectors[n[0]](i, 0) = dv[i];
        }
      }
      return false;
  }
  else if (n[0].getKind() == kind::VECTOR_INDEX) {
    VectorIndex vindex = n[0].getOperator().getConst<VectorIndex>();
    // create if doesn't already exist
    if (vectors.count(n[0][0]) == 0) {
      unsigned length = n[0][0].getType(true).getVectorLength();
      GiNaC::matrix values(length, 1);
      for(unsigned i = 0; i < length; ++i) {
          values(i,0) = 0x8000000000000;
      }
      
      //insert into map
      vectors.insert(std::pair<TNode, GiNaC::matrix>(n[0][0], values));
     }
      
    if(n[1].getKind() == kind::CONST_VECTOR) {
      std::vector<double> val = n[1].getConst<Vector>().getDoubleValues();
      vectors[n[0][0]](vindex.index, 0) = val[0];
      std::vector<TNode> assertionSet; assertionSet.push_back(n);
      vectorindexTNodes.insert(std::make_pair(vindex.index, assertionSet));
      return false;
    }
    else if (n[1].getKind() == kind::VECTOR_INDEX) {
      VectorIndex vindex2 = n[1].getOperator().getConst<VectorIndex>();
      // Not what I should actually do...
      //        GiNaC::symbol a("a");
      //        matrices[n[0][0]](mindex.row, mindex.col) = a;
      //        matrices[n[0][0]](mindex2.row, mindex2.col) = a;
    }
  }
  else {
    return true;
  }
} /*processIndexAssignments*/

bool TheoryMatrix::checkRank(std::map<TNode, GiNaC::matrix > & matrices, TNode & n) {
  //get the constant matrix
  TNode n00 = n[0][0];
  GiNaC::matrix M = getMatrix(matrices, n00);
  unsigned r = M.rank();
  unsigned r_assertion = n[1].getConst<Rational>().getNumerator().getUnsignedInt();
  
  switch(n.getKind()) {
  case kind::MEQ: {
    return r == r_assertion;
  }
  case kind::MLE: {
    return r <= r_assertion;
  }
  case kind::MLT: {
    return r < r_assertion;
  }
  case kind::MGE: {
    return r >= r_assertion;
  }
  case kind::MGT: {
    return r > r_assertion;
  }
  default:
    Unhandled(n.getKind());
  }
} /* TheoryMatrix::checkRank */

bool TheoryMatrix::checkDet(std::map<TNode, GiNaC::matrix > & matrices, TNode & n) {
    //get the constant matrix
  TNode n00 = n[0][0];
  GiNaC::matrix M = getMatrix(matrices, n00);

  GiNaC::ex d = M.determinant();
  
  long d_assertion_long = n[1].getConst<Rational>().getDouble();

  GiNaC::ex d_assertion;
  d_assertion += d_assertion_long;

  switch(n.getKind()) {
  case kind::MEQ: {
    return d == d_assertion;
  }
  case kind::MLE: {
    return d <= d_assertion;
  }
  case kind::MLT: {
    return d < d_assertion;
  }
  case kind::MGE: {
    return d >= d_assertion;
  }
  case kind::MGT: {
    return d > d_assertion;
  }
  default:
    Unhandled(n.getKind());
  }
  
}/* TheoryMatrix::checkDet */

bool TheoryMatrix::checkVrange(std::map<TNode, GiNaC::matrix > & matrices, std::map<TNode, GiNaC::matrix > & vectors, TNode & n) {
  TNode n0 = n[0];
  GiNaC::matrix M = getMatrix(matrices, n0);
  
  GiNaC::matrix v;
  if(vectors.count(n[1]) == 1) {
    v = vectors[n[1]];
  }
  else if (n[1].getKind() == kind::CONST_VECTOR) {
    std::vector<double> vvals = n[1].getConst<Vector>().getDoubleValues();
    v = GiNaC::matrix(vvals.size(), 1);
    for(unsigned i = 0; i < vvals.size(); ++i) {
        v(i, 0) = vvals[i];
      }
  }
  else {
    std::cout << "Vector never assigned" << std::endl;
  }

  //create matrix of vars
  GiNaC::matrix vars(M.cols(), 1);
  for(unsigned i = 0; i < M.cols(); ++i) {
    std::stringstream sstm;
    sstm << "x" << i;
    std::string label = sstm.str();
    GiNaC::symbol x(label);
    vars(i, 0) = x;
  }

  //check if vector in range
  try {
    GiNaC::matrix sol = M.solve(vars, v);
    return true;
  }
  catch(...) {
    //if get any exception, return false
    return false;
  }
  
}

GiNaC::matrix TheoryMatrix::getMatrix(std::map<TNode, GiNaC::matrix > & matrices, TNode & n) {
  GiNaC::matrix M;
  if (matrices.count(n) == 1) {
    return matrices[n];
  }
  else if (n.getKind() == kind::CONST_MATRIX) {
    std::vector< std::vector<double> > Mvals = n.getConst<Matrix>().getDoubleValues();
    M = GiNaC::matrix(Mvals.size(), Mvals[0].size());
    for(unsigned i = 0; i < Mvals.size(); ++i) {
      for(unsigned j = 0; j < Mvals[0].size(); ++j) {
        M(i, j) = Mvals[i][j];
      }
    }
    return M;
  }
  else if (n.getKind() == kind::MATRIX_MULT) {
    // try a recursion here
    TNode n0 = n[0];
    TNode n1 = n[1];
    GiNaC::matrix M1 = getMatrix(matrices, n0);
    GiNaC::matrix M2 = getMatrix(matrices, n1);
    return M1.mul(M2);
  }

  else if (n.getKind() == kind::MATRIX_ADD) {
    TNode n0 = n[0];
    TNode n1 = n[1];
    GiNaC::matrix M1 = getMatrix(matrices, n0);
    GiNaC::matrix M2 = getMatrix(matrices, n1);

    return M1.add(M2);
  }
  else {
    cout << "Matrix was never assigned..." << endl;
  }
}

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
