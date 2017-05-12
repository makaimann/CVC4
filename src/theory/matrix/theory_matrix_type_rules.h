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

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__MATRIX__THEORY_MATRIX_TYPE_RULES_H */
