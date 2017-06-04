#include "theory/matrix/theory_matrix.h"

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
  std::map<TNode, std::vector< std::vector<double> > > matrices;

  while(!done()) {
    // Get all the assertions
    Assertion assertion = get();
    TNode fact = assertion.assertion;

    Debug("matrix") << "TheoryMatrix::check(): processing " << fact << std::endl;
    Debug("matrix") << "TheoryMatrix::check(): kind =  " << fact.getKind() << std::endl;

    // Do the work
    switch(fact.getKind()) {

    /* cases for all the theory's kinds go here... */
    case kind::EQUAL:
    {
      bool keep = processEquals(matrices, fact);
      if(keep) {
        otherAssertionTNodes.push_back(fact);
      }
      break;
    }
    default:
      Unhandled(fact.getKind());
    }
  }

  //print matrix
  std::cout << "TheoryMatrix::check(): Printing parsed matrices " << std::endl;
  //print matrices here
  std::map<TNode, std::vector< std::vector<double> > >::iterator it;
  for(it = matrices.begin(); it != matrices.end(); ++it) {
    // not sure if the following line works...
    //    Debug("matrix") << "Printing matrix " << getAttribute(it->first, expr::VarNameAttr(), name) << std::endl;
    std::vector< std::vector<double> > matrix = it->second;
    for(unsigned int i = 0; i < matrix.size(); ++i) {
      for(unsigned int j = 0; j < matrix[0].size(); ++j) {
        std::cout << matrix[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }

}/* TheoryMatrix::check() */


bool TheoryMatrix::processEquals(std::map<TNode, std::vector< std::vector<double> > > & matrices,
                                 TNode & n)
{
  Debug("matrix") << "n[0] = " << n[0] << std::endl;
  Debug("matrix") << "n[0].getKind() = " << n[0].getKind() << std::endl;
  //  std::cout << "with kind = " << n[0].getKind() << std::endl;
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
    //already in matrices map
    if(matrices.count(n[0][0]) == 1) {
      // need to check if this element already assigned
      if(matrices[n[0][0]][mindex.row][mindex.col] != 0x8000000000000) {
        //do something because asserted to indices were different numbers
        std::cout << "Asserting multiple values for same indices" << std::endl;
      }
      
      // this should always just be a single value...
      std::vector<std::vector<double> > val = n[1].getConst<Matrix>().getDoubleValues();
      matrices[n[0][0]][mindex.row][mindex.col] = val[0][0];
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
      return false;
    }
  }
  //else if (n[0].getKind() == kind::VECTOR_TYPE)
  else {
    return true;
  }
}

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
