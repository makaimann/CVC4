/*********************                                                        */
/*! \file theory_arith_type_rules.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Dejan Jovanovic, Morgan Deters, Christopher L. Conway
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2018 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief [[ Add brief comments here ]]
 **
 ** [[ Add file-specific comments here ]]
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__ARITH__THEORY_ARITH_TYPE_RULES_H
#define __CVC4__THEORY__ARITH__THEORY_ARITH_TYPE_RULES_H

namespace CVC4 {
namespace theory {
namespace arith {


class ArithConstantTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    Assert(n.getKind() == kind::CONST_RATIONAL);
    if(n.getConst<Rational>().isIntegral()){
      return nodeManager->integerType();
    }else{
      return nodeManager->realType();
    }
  }
};/* class ArithConstantTypeRule */

class ArithOperatorTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    TypeNode integerType = nodeManager->integerType();
    TypeNode realType = nodeManager->realType();
    TNode::iterator child_it = n.begin();
    TNode::iterator child_it_end = n.end();
    bool isInteger = true;
    for(; child_it != child_it_end; ++child_it) {
      TypeNode childType = (*child_it).getType(check);
      if (!childType.isInteger()) {
        isInteger = false;
        if( !check ) { // if we're not checking, nothing left to do
          break;
        }
      }
      if( check ) {
        if(!childType.isReal()) {
          throw TypeCheckingExceptionPrivate(n, "expecting an arithmetic subterm");
        }
      }
    }
    switch(Kind k = n.getKind()) {
      case kind::TO_REAL:
        return realType;
      case kind::TO_INTEGER:
        return integerType;
      default: {
        bool isDivision = k == kind::DIVISION || k == kind::DIVISION_TOTAL;
        return (isInteger && !isDivision ? integerType : realType);
      }
    }
  }
};/* class ArithOperatorTypeRule */

class IntOperatorTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    TNode::iterator child_it = n.begin();
    TNode::iterator child_it_end = n.end();
    if(check) {
      for(; child_it != child_it_end; ++child_it) {
        TypeNode childType = (*child_it).getType(check);
        if (!childType.isInteger()) {
          throw TypeCheckingExceptionPrivate(n, "expecting an integer subterm");
        }
      }
    }
    return nodeManager->integerType();
  }
};/* class IntOperatorTypeRule */

class RealOperatorTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    TNode::iterator child_it = n.begin();
    TNode::iterator child_it_end = n.end();
    if(check) {
      for(; child_it != child_it_end; ++child_it) {
        TypeNode childType = (*child_it).getType(check);
        if (!childType.isReal()) {
          throw TypeCheckingExceptionPrivate(n, "expecting a real subterm");
        }
      }
    }
    return nodeManager->realType();
  }
};/* class RealOperatorTypeRule */

class ArithPredicateTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    if( check ) {
      TypeNode lhsType = n[0].getType(check);
      if (!lhsType.isReal()) {
        throw TypeCheckingExceptionPrivate(n, "expecting an arithmetic term on the left-hand-side");
      }
      TypeNode rhsType = n[1].getType(check);
      if (!rhsType.isReal()) {
        throw TypeCheckingExceptionPrivate(n, "expecting an arithmetic term on the right-hand-side");
      }
    }
    return nodeManager->booleanType();
  }
};/* class ArithPredicateTypeRule */

class ArithUnaryPredicateTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    if( check ) {
      TypeNode t = n[0].getType(check);
      if (!t.isReal()) {
        throw TypeCheckingExceptionPrivate(n, "expecting an arithmetic term");
      }
    }
    return nodeManager->booleanType();
  }
};/* class ArithUnaryPredicateTypeRule */

class IntUnaryPredicateTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    if( check ) {
      TypeNode t = n[0].getType(check);
      if (!t.isInteger()) {
        throw TypeCheckingExceptionPrivate(n, "expecting an integer term");
      }
    }
    return nodeManager->booleanType();
  }
};/* class IntUnaryPredicateTypeRule */

class RealNullaryOperatorTypeRule {
public:
  inline static TypeNode computeType(NodeManager* nodeManager, TNode n, bool check)
  {
    // for nullary operators, we only computeType for check=true, since they are given TypeAttr() on creation
    Assert(check);
    TypeNode realType = n.getType();
    if(realType!=NodeManager::currentNM()->realType()) {
      throw TypeCheckingExceptionPrivate(n, "expecting real type");
    }
    return realType;
  }
};/* class RealNullaryOperatorTypeRule */

class DivisibleOpTypeRule
{
 public:
  inline static TypeNode computeType(NodeManager* nodeManager,
                                     TNode n,
                                     bool check)
  {
    Assert(n.getKind() == kind::DIVISIBLE_OP);
    return nodeManager->builtinOperatorType();
  }
}; /* class DivisibleOpTypeRule */

}/* CVC4::theory::arith namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__ARITH__THEORY_ARITH_TYPE_RULES_H */
