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

  //below is the main engine...trying without it now
  /*
  while(!done()) {
    // Get all the assertions
    Assertion assertion = get();
    TNode fact = assertion.assertion;

    Debug("matrix") << "TheoryMatrix::check(): processing " << fact << endl;
    Debug("matrix") << "TheoryMatrix::check(): kind =  " << fact.getKind() << endl;

    // Do the work
    switch(fact.getKind()) {

    // cases for all the theory's kinds go here...
    case kind::EQUAL:
    {
      bool keep = processIndexAssignments(matrices, indexTNodes, fact);
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
      Unhandled(fact.getKind());
    }
  }
  */

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
  cout << "TheoryMatrix::check(): Printing parsed matrices " << endl;
  //print matrices here
  std::map<TNode, GiNaC::matrix >::iterator matrix_it;
  for(matrix_it = matrices.begin(); matrix_it != matrices.end(); ++matrix_it) {
    // not sure if the following line works...
    //    Debug("matrix") << "Printing matrix " << getAttribute(it->first, expr::VarNameAttr(), name) << endl;
    GiNaC::matrix m = matrix_it->second;
    for(unsigned int i = 0; i < m.rows(); ++i) {
      for(unsigned int j = 0; j < m.cols(); ++j) {
        std::cout << m(i, j) << " ";

        //stop checking if incomplete matrix
        if(m(i, j) == 0x8000000000000)
        {
          std::cout << "\nExiting check because matrix currently incomplete" << std::endl;
          return;
        }
      }
      cout << endl;
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
    else {
      Unhandled(n.getKind());
    }
/*    
    switch(n.getKind()) {
    case kind::EQUAL: {
      if(n[1].getKind() == kind::MATRIX_RANK) {
        //get the constant matrix
        GiNaC::matrix M;
        if(matrices.count(n[1][0]) == 1) {
          M = matrices[n[1][0]];
        }
        else if (n[1][0].getKind() == kind::CONST_MATRIX) {
          std::vector< std::vector<double> > Mvals = n[1][0].getConst<Matrix>().getDoubleValues();
          M = GiNaC::matrix(Mvals.size(), Mvals[0].size());
          for(unsigned i = 0; i < Mvals.size(); ++i) {
            for(unsigned j = 0; j < Mvals[0].size(); ++j) {
              M(i, j) = Mvals[i][j];
            }
          }
        }
        else {
          std::cout << "Matrix never assigned" << std::endl;
        }
        unsigned r = M.rank();
        if (r != n[0].getConst<Rational>().getNumerator().getUnsignedInt()) {
          t_conflict = true;
        }
      }
      break;
    }
    case kind::LEQ: {

    }
    case kind::LT: {

    }
    case kind::GEQ: {

    }
    case kind::GT: {

    }
    default:
      break;
    }
*/
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
    //TODO: just create the matrix up above if doesn't exist
    //--> then the rest of the code can be the same instead of in cases
    // matrix hasn't been created yet
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
    if (vectors.count(n[0][0]) == 1) {
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

  /*
void TheoryMatrix::processMatrixPredicates(std::vector<TNode> assertions) {
  for(unsigned i = 0; i < assertions.size(); ++i) {
    TNode n = assertions[i];
    switch(n.getKind()) {
    case kind::EQUAL:
      break;
    case kind::LEQ:
      break;
    case kind::
    }
  }
}
  */

  /*
unsigned TheoryMatrix::computeRank(std::vector<std::vector <double> > & M) {
  unsigned rows = M.size();
  unsigned cols = M[0].size();
  GiNaC::matrix GM(rows, cols);
  for(unsigned i = 0; i < rows; ++i) {
    for(unsigned j = 0; j < cols; ++j) {
      GM(i, j) = M[i][j];
    }
  }
  return GM.rank();
}
  */

bool TheoryMatrix::checkRank(std::map<TNode, GiNaC::matrix > & matrices, TNode & n) {
  //get the constant matrix
  GiNaC::matrix M;
  if(matrices.count(n[0][0]) == 1) {
    M = matrices[n[0][0]];
  }
  else if (n[0][0].getKind() == kind::CONST_MATRIX) {
    std::vector< std::vector<double> > Mvals = n[0][0].getConst<Matrix>().getDoubleValues();
    M = GiNaC::matrix(Mvals.size(), Mvals[0].size());
    for(unsigned i = 0; i < Mvals.size(); ++i) {
      for(unsigned j = 0; j < Mvals[0].size(); ++j) {
        M(i, j) = Mvals[i][j];
      }
    }
  }
  else {
    std::cout << "Matrix never assigned" << std::endl;
  }
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
  GiNaC::matrix M;
  if(matrices.count(n[0][0]) == 1) {
    M = matrices[n[0][0]];
  }
  else if (n[0][0].getKind() == kind::CONST_MATRIX) {
    std::vector< std::vector<double> > Mvals = n[0][0].getConst<Matrix>().getDoubleValues();
    M = GiNaC::matrix(Mvals.size(), Mvals[0].size());
    for(unsigned i = 0; i < Mvals.size(); ++i) {
      for(unsigned j = 0; j < Mvals[0].size(); ++j) {
        M(i, j) = Mvals[i][j];
      }
    }
  }
  else {
    std::cout << "Matrix never assigned" << std::endl;
  }

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

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
