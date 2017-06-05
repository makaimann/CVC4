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
  std::map<TNode, std::vector< std::vector<double> > > matrices;

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

    
  //check for conflicts in index assertions
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

  //return conflicts if multiple indices asserted to be different values
  if(index_conflicts) {
    Node conflict = conjunction;
    d_out->conflict(conflict);
    return;
  }

  //print matrix
  cout << "TheoryMatrix::check(): Printing parsed matrices " << endl;
  //print matrices here
  std::map<TNode, std::vector< std::vector<double> > >::iterator matrix_it;
  for(matrix_it = matrices.begin(); matrix_it != matrices.end(); ++matrix_it) {
    // not sure if the following line works...
    //    Debug("matrix") << "Printing matrix " << getAttribute(it->first, expr::VarNameAttr(), name) << endl;
    std::vector< std::vector<double> > m = matrix_it->second;
    for(unsigned int i = 0; i < m.size(); ++i) {
      for(unsigned int j = 0; j < m[0].size(); ++j) {
        std::cout << m[i][j] << " ";

        //stop checking if incomplete matrix
        if(m[i][j] == 0x8000000000000)
        {
          std::cout << "\nExiting check because matrix currently incomplete" << std::endl;
          return;
        }
      }
      cout << endl;
    }
  }
  //end print matrix

  bool t_conflict = false;
  for(unsigned i = 0; i < otherAssertionTNodes.size(); ++i) {
    TNode n = otherAssertionTNodes[i];
    switch(n.getKind()) {
    case kind::EQUAL: {
      if(n[1].getKind() == kind::MATRIX_RANK) {
        //get the constant matrix
        std::vector< std::vector<double> > M;
        if(matrices.count(n[1][0]) == 1) {
          M = matrices[n[1][0]];
        }
        else if (n[1][0].getKind() == kind::CONST_MATRIX) {
          M = n[1][0].getConst<Matrix>().getDoubleValues();
        }
        else {
          std::cout << "Matrix never assigned" << std::endl;
        }
        unsigned r = computeRank(M);
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

    if(t_conflict) {
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


bool TheoryMatrix::processIndexAssignments(std::map<TNode, std::vector< std::vector<double> > > & matrices,
                                 std::map<std::pair<unsigned, unsigned>, std::vector<TNode> > & indexTNodes,
                                 TNode & n)
{
  Debug("matrix") << "n[0] = " << n[0] << endl;
  Debug("matrix") << "n[0].getKind() = " << n[0].getKind() << endl;
  //  cout << "with kind = " << n[0].getKind() << endl;
  if(n[0].getKind() == kind::MATRIX_TYPE) {
    //if assigning whole matrix
    if(n[1].getKind() == kind::CONST_MATRIX) {
      matrices[n[0]] = n[1].getConst<Matrix>().getDoubleValues();
      return false;
    }
  }
  //if indexing
  else if (n[0].getKind() == kind::MATRIX_INDEX) {
    MatrixIndex mindex = n[0].getOperator().getConst<MatrixIndex>();
    //already in matrices std::map
    if(matrices.count(n[0][0]) == 1) {
      // this should always just be a single value...
      std::vector<std::vector<double> > val = n[1].getConst<Matrix>().getDoubleValues();
      
      // need to check if this element already assigned
      if(matrices[n[0][0]][mindex.row][mindex.col] != 0x8000000000000) {
        //do something because asserted to indices were different numbers
        if(matrices[n[0][0]][mindex.row][mindex.col] != val[0][0]) {
          cout << "Asserting multiple values for same indices" << endl;
          std::pair<unsigned, unsigned> p = std::make_pair(mindex.row, mindex.col);
          indexTNodes[p].push_back(n);
          return false;
        }
      }
      matrices[n[0][0]][mindex.row][mindex.col] = val[0][0];
      std::pair<unsigned, unsigned> p = std::make_pair(mindex.row, mindex.col);
      std::vector<TNode> assertionSet; assertionSet.push_back(n);
      indexTNodes.insert(std::make_pair(p, assertionSet));
      return false;
    }
    // matrix hasn't been created yet
    else {
      std::vector< std::vector<double> > values;
      std::vector<unsigned> dimensions = n[0][0].getType(true).getMatrixDim();
      for(unsigned i = 0; i < dimensions[0]; ++i) {
        std::vector<double> v(dimensions[1], 0x8000000000000);
        values.push_back(v);
      }
      matrices.insert(std::pair<TNode, std::vector< std::vector<double> > >(n[0][0], values));
      //TODO: Add an element
      std::vector<std::vector<double> > val = n[1].getConst<Matrix>().getDoubleValues();
      matrices[n[0][0]][mindex.row][mindex.col] = val[0][0];
      std::pair<unsigned, unsigned> p = std::make_pair(mindex.row, mindex.col);
      std::vector<TNode> assertionSet; assertionSet.push_back(n);
      indexTNodes.insert(std::make_pair(p, assertionSet));
      return false;
    }
  }
  //else if (n[0].getKind() == kind::STD::VECTOR_TYPE)
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

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
