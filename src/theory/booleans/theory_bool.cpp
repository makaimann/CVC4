/*********************                                                        */
/*! \file theory_bool.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Morgan Deters, Dejan Jovanovic, Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2017 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief The theory of booleans.
 **
 ** The theory of booleans.
 **/

#include "theory/theory.h"
#include "theory/booleans/theory_bool.h"
#include "theory/booleans/circuit_propagator.h"
#include "theory/valuation.h"
#include "smt_util/boolean_simplification.h"
#include "theory/substitutions.h"

#include <vector>
#include <stack>
#include "util/hash.h"

using namespace std;

namespace CVC4 {
namespace theory {
namespace booleans {

Theory::PPAssertStatus TheoryBool::ppAssert(TNode in, SubstitutionMap& outSubstitutions) {
  if (in.getKind() == kind::CONST_BOOLEAN && !in.getConst<bool>())
  {
    // If we get a false literal, we're in conflict
    return PP_ASSERT_STATUS_CONFLICT;
  }

  switch (in.getKind())
  {
    case kind::EQUAL:
    {
      if (in[0].isVar() && !in[1].hasSubterm(in[0]))
      {
        outSubstitutions.addSubstitution(in[0], in[1]);
        return PP_ASSERT_STATUS_SOLVED;
      }
      else if (in[1].isVar() && !in[0].hasSubterm(in[1]))
      {
        outSubstitutions.addSubstitution(in[1], in[0]);
        return PP_ASSERT_STATUS_SOLVED;
      }
      else
      {
        return PP_ASSERT_STATUS_UNSOLVED;
      }
      break;
    }
    case kind::NOT:
    {
      if (in[0].getKind() == kind::VARIABLE)
      {
        outSubstitutions.addSubstitution(
            in[0], NodeManager::currentNM()->mkConst<bool>(false));
      }
      else
      {
        return PP_ASSERT_STATUS_UNSOLVED;
      }
      break;
    }
    case kind::VARIABLE:
    {
      if (in.getKind() == kind::VARIABLE)
      {
        outSubstitutions.addSubstitution(
            in, NodeManager::currentNM()->mkConst<bool>(true));
      }
      else
      {
        return PP_ASSERT_STATUS_UNSOLVED;
      }
      break;
    }
    default:
    {
      return PP_ASSERT_STATUS_UNSOLVED;
      break;
    }
  }

  return PP_ASSERT_STATUS_SOLVED;
}

/*
void TheoryBool::check(Effort level) {
  if (done() && !fullEffort(level)) {
    return;
  }
  while (!done())
  {
    // Get all the assertions
    Assertion assertion = get();
    TNode fact = assertion.assertion;
  }
  if( Theory::fullEffort(level) ){
  }
}
*/

}/* CVC4::theory::booleans namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */
