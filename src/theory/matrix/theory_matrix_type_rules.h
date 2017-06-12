#include "cvc4_private.h"

#ifndef __CVC4__THEORY__MATRIX__THEORY_MATRIX_TYPE_RULES_H
#define __CVC4__THEORY__MATRIX__THEORY_MATRIX_TYPE_RULES_H

namespace CVC4 {
namespace theory {
namespace matrix {

class MatrixTypeRule {
public:

  /**
   * Compute the type for (and optionally typecheck) a term belonging
   * to the theory of matrix.
   *
   * @param check if true, the node's type should be checked as well
   * as computed.
   */
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check)
    throw (TypeCheckingExceptionPrivate) {

    // TODO: implement me!
    Unimplemented();

  }

};/* class MatrixTypeRule */


class ConstMatrixTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    std::vector<unsigned> dimensions = n.getConst<Matrix>().getDim();
    if (check) {
      if (dimensions[0] == 0 || dimensions[1] == 0) {
        throw TypeCheckingExceptionPrivate(n, "Invalid dimensions");
      }
    }
    return nodeManager->mkMatrixType(dimensions[0], dimensions[1]);
  }
};/* class ConstMatrixTypeRule*/


class ConstVectorTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    unsigned length = n.getConst<Vector>().getLength();
    if (check) {
      if (length <= 0) {
        throw TypeCheckingExceptionPrivate(n, "Invalid length");
      }
    }
    return nodeManager->mkVectorType(length);
  }
};/* class ConstVectorTypeRule*/


class MatrixVectorTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    TNode::iterator it = n.begin();
    TypeNode M = (*it).getType(check);
    ++it;
    TypeNode v = (*it).getType(check);
    if (check) {
      if (!M.isMatrix()) {
        throw TypeCheckingExceptionPrivate(n, "expecting Matrix as first term");
      }
      if (!v.isVector()) {
        throw TypeCheckingExceptionPrivate(n, "expecting Vector as second term");
      }
    }
    std::vector<unsigned> dimensions = M.getMatrixDim();
    unsigned length = v.getVectorLength();

    if (check) {
      if (dimensions[1] != length) {
        throw TypeCheckingExceptionPrivate(n, "Matrix columns and Vector length mismatch");
      }
    }

    // Figure out node type
    if (n.getKind() == kind::VECTOR_IN_RANGE) {
      return nodeManager->booleanType();
    }
    else if (n.getKind() == kind::MATRIX_VECTOR_MULT) {
      return nodeManager->mkVectorType(dimensions[0]);
    }
    else {
      // Might not be right exception but works for now.
      throw TypeCheckingExceptionPrivate(n, "Unhandled kind in type checking");
    }
  }
};/* class MatrixVectorTypeRule */


class VectorSameLengthTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    TypeNode v1 = n[0].getType(check);
    TypeNode v2 = n[1].getType(check);

    if (check) {
      if (!v1.isVector() || !v2.isVector()) {
        throw TypeCheckingExceptionPrivate(n, "expecting Vector terms");
      }

      if (v1.getVectorLength() != v2.getVectorLength()) {
        throw TypeCheckingExceptionPrivate(n, "expecting Vectors of the same length");
      }
    }
  }
};/* class VectorSameLengthTypeRule */


class MatrixMultTypeRule {
 public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {

    TypeNode t = n[0].getType(check);
    TypeNode t2 = n[1].getType(check);

    std::vector< unsigned > d1 = t.getMatrixDim();
    std::vector< unsigned > d2 = t.getMatrixDim();


    if(check) {
      if(!t.isMatrix() || !t2.isMatrix()) {
        throw TypeCheckingExceptionPrivate(n, "expecting Matrix terms");
      }
      
      if(d1[1] != d2[0]) {
        throw TypeCheckingExceptionPrivate(n, "Matrix 1 must have the same number of columns as Matrix 2 has rows.");
      }
    }

    return nodeManager->mkMatrixType(d1[1], d2[0]);
  }

}; /* class MatrixMultTypeRule */


class MatrixSameDimTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    TNode::iterator it = n.begin();
    TypeNode t = (*it).getType(check);
    if (check) {
      if (!t.isMatrix()) {
        throw TypeCheckingExceptionPrivate(n, "expecting Matrix terms");
      }
      TNode::iterator it_end = n.end();
      for (++it; it != it_end; ++it) {
        if((*it).getType(check) != t) {
          throw TypeCheckingExceptionPrivate(n, "expecting Matrices of same dimension");
        }
      }
    }
    return t;
  }
};/* class MatrixSameDimTypeRule */


class MatrixRankTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    return nodeManager->integerType();
  }
};/* class MatrixRankTypeRule */


class MatrixDetTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    return nodeManager->realType();
  }
};/* class MatrixDetTypeRule */


class ComparisonTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    return nodeManager->booleanType();
  }
}; /* class ComparisonTypeRule */


class MatrixIndexTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    MatrixIndex mindex = n.getOperator().getConst<MatrixIndex>();

    if (check) {
      TypeNode t = n[0].getType(check);
      if (!t.isMatrix()) {
        throw TypeCheckingExceptionPrivate(n, "expecting a Matrix term");
      }
      std::vector<unsigned> dimensions = t.getMatrixDim();
      if (mindex.row < 0 || mindex.col < 0 || mindex.row >= dimensions[0] || mindex.col >= dimensions[1]) {
        throw TypeCheckingExceptionPrivate(n, "Index and matrix dimension mismatch");
      }
    }
    //Not sure if I should use reals... for now just calling a 1d matrix
    return nodeManager->mkMatrixType(1, 1);
  }
};/* class MatrixIndexTypeRule */


class MatrixIndexOpTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    Assert(n.getKind() == kind::MATRIX_INDEX_OP);
    return nodeManager->builtinOperatorType();
  }
};/* class MatrixIndexOpTypeRule */


class VectorIndexTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    VectorIndex vindex = n.getOperator().getConst<VectorIndex>();

    if (check) {
      TypeNode t = n[0].getType(check);
      if (!t.isVector()) {
        throw TypeCheckingExceptionPrivate(n, "expecting a Vector term");
      }
      unsigned length = t.getVectorLength();
      if (vindex.index < 0 || vindex.index >= length) {
        throw TypeCheckingExceptionPrivate(n, "expecting a Vector term");
      }
    }

    return nodeManager->mkVectorType(1);
  }
};/* class VectorIndexTypeRule */


class VectorIndexOpTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n,
                                     bool check) {
    Assert(n.getKind() == kind::VECTOR_INDEX_OP);
    return nodeManager->builtinOperatorType();
  }
};/* class VectorIndexOpTypeRule */


}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__MATRIX__THEORY_MATRIX_TYPE_RULES_H */
