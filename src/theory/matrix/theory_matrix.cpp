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

  while(!done()) {
    // Get all the assertions
    Assertion assertion = get();
    TNode fact = assertion.assertion;

    Debug("matrix") << "TheoryMatrix::check(): processing " << fact << std::endl;

    // Do the work
    switch(fact.getKind()) {

    /* cases for all the theory's kinds go here... */

    default:
      Unhandled(fact.getKind());
    }
  }

}/* TheoryMatrix::check() */

}/* CVC4::theory::matrix namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
