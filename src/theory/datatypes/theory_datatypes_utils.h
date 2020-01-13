/*********************                                                        */
/*! \file theory_datatypes_utils.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Util functions for theory datatypes.
 **
 ** Util functions for theory datatypes.
 **/

#include "cvc4_private.h"

#ifndef CVC4__THEORY__STRINGS__THEORY_DATATYPES_UTILS_H
#define CVC4__THEORY__STRINGS__THEORY_DATATYPES_UTILS_H

#include <vector>

#include "expr/node.h"
#include "expr/node_manager_attributes.h"

namespace CVC4 {
namespace theory {

// ----------------------- sygus datatype attributes
/** sygus var num */
struct SygusVarNumAttributeId
{
};
typedef expr::Attribute<SygusVarNumAttributeId, uint64_t> SygusVarNumAttribute;

/**
 * Attribute true for enumerators whose current model values were registered by
 * the datatypes sygus solver, and were not excluded by sygus symmetry breaking.
 * This is set by the datatypes sygus solver during LAST_CALL effort checks for
 * each active sygus enumerator.
 */
struct SygusSymBreakOkAttributeId
{
};
typedef expr::Attribute<SygusSymBreakOkAttributeId, bool>
    SygusSymBreakOkAttribute;

/** sygus var free
 *
 * This attribute is used to mark whether sygus operators have free occurrences
 * of variables from the formal argument list of the function-to-synthesize.
 *
 * We store three possible cases for sygus operators op:
 * (1) op.getAttribute(SygusVarFreeAttribute())==Node::null()
 * In this case, op has no free variables from the formal argument list of the
 * function-to-synthesize.
 * (2) op.getAttribute(SygusVarFreeAttribute())==v, where v is a bound variable.
 * In this case, op has exactly one free variable, v.
 * (3) op.getAttribute(SygusVarFreeAttribute())==op
 * In this case, op has an arbitrary set (cardinality >1) of free variables from
 * the formal argument list of the function to synthesize.
 *
 * This attribute is used to compute applySygusArgs below.
 */
struct SygusVarFreeAttributeId
{
};
typedef expr::Attribute<SygusVarFreeAttributeId, Node> SygusVarFreeAttribute;
// ----------------------- end sygus datatype attributes

namespace datatypes {
namespace utils {

/** get instantiate cons
 *
 * This returns the term C( sel^{C,1}( n ), ..., sel^{C,m}( n ) ),
 * where C is the index^{th} constructor of datatype dt.
 */
Node getInstCons(Node n, const Datatype& dt, int index);
/** is instantiation cons
 *
 * If this method returns a value >=0, then that value, call it index,
 * is such that n = C( sel^{C,1}( t ), ..., sel^{C,m}( t ) ),
 * where C is the index^{th} constructor of dt.
 */
int isInstCons(Node t, Node n, const Datatype& dt);
/** is tester
 *
 * This method returns a value >=0 if n is a tester predicate. The return
 * value indicates the constructor index that the tester n is for. If this
 * method returns a value >=0, then it updates a to the argument that the
 * tester n applies to.
 */
int isTester(Node n, Node& a);
/** is tester, same as above but does not update an argument */
int isTester(Node n);
/**
 * Get the index of a constructor or tester in its datatype, or the
 * index of a selector in its constructor.  (Zero is always the
 * first index.)
 */
unsigned indexOf(Node n);
/** make tester is-C( n ), where C is the i^{th} constructor of dt */
Node mkTester(Node n, int i, const Datatype& dt);
/** make tester split
 *
 * Returns the formula (OR is-C1( n ) ... is-Ck( n ) ), where C1...Ck
 * are the constructors of n's type (dt).
 */
Node mkSplit(Node n, const Datatype& dt);
/** returns true iff n is a constructor term with no datatype children */
bool isNullaryApplyConstructor(Node n);
/** returns true iff c is a constructor with no datatype children */
bool isNullaryConstructor(const DatatypeConstructor& c);
/** check clash
 *
 * This method returns true if and only if n1 and n2 have a skeleton that has
 * conflicting constructors at some term position.
 * Examples of terms that clash are:
 *   C( x, y ) and D( z )
 *   C( D( x ), y ) and C( E( x ), y )
 * Examples of terms that do not clash are:
 *   C( x, y ) and C( D( x ), y )
 *   C( D( x ), y ) and C( x, E( z ) )
 *   C( x, y ) and z
 */
bool checkClash(Node n1, Node n2, std::vector<Node>& rew);

// ------------------------ sygus utils

/** get operator kind for sygus builtin
 *
 * This returns the Kind corresponding to applications of the operator op
 * when building the builtin version of sygus terms. This is used by the
 * function mkSygusTerm.
 */
Kind getOperatorKindForSygusBuiltin(Node op);
/** make sygus term
 *
 * This function returns a builtin term f( children[0], ..., children[n] )
 * where f is the builtin op that the i^th constructor of sygus datatype dt
 * encodes. If doBetaReduction is true, then lambdas are eagerly eliminated
 * via beta reduction.
 */
Node mkSygusTerm(const Datatype& dt,
                 unsigned i,
                 const std::vector<Node>& children,
                 bool doBetaReduction = true);
/**
 * n is a builtin term that is an application of operator op.
 *
 * This returns an n' such that (eval n args) is n', where n' is a instance of
 * n for the appropriate substitution.
 *
 * For example, given a function-to-synthesize with formal argument list (x,y),
 * say we have grammar:
 *   A -> A+A | A+x | A+(x+y) | y
 * These lead to constructors with sygus ops:
 *   C1 / (lambda w1 w2. w1+w2)
 *   C2 / (lambda w1. w1+x)
 *   C3 / (lambda w1. w1+(x+y))
 *   C4 / y
 * Examples of calling this function:
 *   applySygusArgs( dt, C1, (APPLY_UF (lambda w1 w2. w1+w2) t1 t2), { 3, 5 } )
 *     ... returns (APPLY_UF (lambda w1 w2. w1+w2) t1 t2).
 *   applySygusArgs( dt, C2, (APPLY_UF (lambda w1. w1+x) t1), { 3, 5 } )
 *     ... returns (APPLY_UF (lambda w1. w1+3) t1).
 *   applySygusArgs( dt, C3, (APPLY_UF (lambda w1. w1+(x+y)) t1), { 3, 5 } )
 *     ... returns (APPLY_UF (lambda w1. w1+(3+5)) t1).
 *   applySygusArgs( dt, C4, y, { 3, 5 } )
 *     ... returns 5.
 * Notice the attribute SygusVarFreeAttribute is applied to C1, C2, C3, C4,
 * to cache the results of whether the evaluation of this constructor needs
 * a substitution over the formal argument list of the function-to-synthesize.
 */
Node applySygusArgs(const Datatype& dt,
                    Node op,
                    Node n,
                    const std::vector<Node>& args);
/**
 * Get the builtin sygus operator for constructor term n of sygus datatype
 * type. For example, if n is the term C_+( d1, d2 ) where C_+ is a sygus
 * constructor whose sygus op is the builtin operator +, this method returns +.
 */
Node getSygusOpForCTerm(Node n);

// ------------------------ end sygus utils

}  // namespace utils
}  // namespace datatypes
}  // namespace theory
}  // namespace CVC4

#endif
