#include "cvc4_private.h"

#ifndef __CVC4__THEORY__MATRIX__THEORY_MATRIX_H
#define __CVC4__THEORY__MATRIX__THEORY_MATRIX_H

#include "theory/theory.h"

namespace CVC4 {
namespace theory {
namespace matrix {

class TheoryMatrix : public Theory {
public:

  /** Constructs a new instance of TheoryMatrix w.r.t. the provided contexts. */
  TheoryMatrix(context::Context* c,
               context::UserContext* u,
               OutputChannel& out,
               Valuation valuation,
               const LogicInfo& logicInfo);

  void check(Effort);

  bool processEquals(std::map<TNode, std::vector< std::vector<double> > > &,
                     std::map<std::pair<unsigned, unsigned>, std::vector<TNode> > &,
                     TNode &);

  std::string identify() const {
    return "THEORY_MATRIX";
  }

};/* class TheoryMatrix */

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__MATRIX__THEORY_MATRIX_H */
