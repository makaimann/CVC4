/*********************                                                        */
/*! \file proof_skolem_cache.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Proof skolem cache utility
 **/

#include "cvc4_private.h"

#ifndef CVC4__EXPR__PROOF_SKOLEM_CACHE_H
#define CVC4__EXPR__PROOF_SKOLEM_CACHE_H

#include <string>

#include "expr/node.h"

namespace CVC4 {

/**
 * A manager for skolems that can be used in proofs. This is designed to be
 * a trusted interface to NodeManager::mkSkolem, where one
 * must provide a definition for the skolem they create in terms of a
 * predicate that the introduced variable is intended to witness.
 *
 * It is implemented by mapping terms to an attribute corresponding to their
 * "witness form" as described below. Hence, this class does not impact the
 * reference counting of skolem variables which may be deleted if they are not
 * used.
 */
class ProofSkolemCache
{
 public:
  ProofSkolemCache() {}
  ~ProofSkolemCache() {}
  /**
   * This makes a skolem of same type as bound variable v, (say its type is T),
   * whose definition is (witness ((v T)) pred). This definition is maintained
   * by this class.
   *
   * Notice that (exists ((v T)) pred) should be a valid formula. This fact
   * captures the reason for why the returned Skolem was introduced.
   *
   * Take as an example extensionality in arrays:
   *
   * (declare-fun a () (Array Int Int))
   * (declare-fun b () (Array Int Int))
   * (assert (not (= a b)))
   *
   * To witness the index where the arrays a and b are disequal, it is intended
   * we call this method on:
   *   Node k = mkSkolem( x, F )
   * where F is:
   *   (=> (not (= a b)) (not (= (select a x) (select b x))))
   * and x is a fresh bound variable of integer type. Internally, this will map
   * k to the term:
   *   (witness ((x Int)) (=> (not (= a b))
   *                          (not (= (select a x) (select b x)))))
   * A lemma generated by the array solver for extensionality may safely use
   * the skolem k in the standard way:
   *   (=> (not (= a b)) (not (= (select a k) (select b k))))
   * Furthermore, notice that the following lemma does not involve fresh
   * skolem variables and is valid according to the theory of arrays extended
   * with support for witness:
   *   (let ((w (witness ((x Int)) (=> (not (= a b))
   *                                   (not (= (select a x) (select b x)))))))
   *     (=> (not (= a b)) (not (= (select a w) (select b w)))))
   * This version of the lemma, which requires no explicit tracking of free
   * Skolem variables, can be obtained by a call to getWitnessForm(...)
   * below. We call this the "witness form" of the lemma above.
   *
   * @param v The bound variable of the same type of the Skolem to create.
   * @param pred The desired property of the Skolem to create, in terms of bound
   * variable v.
   * @param prefix The prefix of the name of the Skolem
   * @param comment Debug information about the Skolem
   * @param flags The flags for the Skolem (see NodeManager::mkSkolem)
   * @return The skolem whose witness form is registered by this class.
   */
  static Node mkSkolem(Node v,
                       Node pred,
                       const std::string& prefix,
                       const std::string& comment = "",
                       int flags = NodeManager::SKOLEM_DEFAULT);
  /**
   * Same as above, but where pred is an existential quantified formula
   * whose bound variable list contains v. For example, calling this method on:
   *   x, (exists ((x Int) (y Int)) (P x y))
   * will return:
   *   (witness ((x Int)) (exists ((y Int)) (P x y)))
   * If the variable v is not in the bound variable list of q, then null is
   * returned and an assertion failure is thrown.
   */
  static Node mkSkolemExists(Node v,
                             Node q,
                             const std::string& prefix,
                             const std::string& comment = "",
                             int flags = NodeManager::SKOLEM_DEFAULT);
  /**
   * Same as above, but for special case of (witness ((x T)) (= x t))
   * where T is the type of t. This skolem is unique for each t, which we
   * implement via an attribute on t. This attribute is used to ensure to
   * associate a unique skolem for each t.
   */
  static Node mkPurifySkolem(Node t,
                             const std::string& prefix,
                             const std::string& comment = "",
                             int flags = NodeManager::SKOLEM_DEFAULT);
  /** convert to witness form
   *
   * @param n The term or formula to convert to witness form described above
   * @return n in witness form.
   */
  static Node getWitnessForm(Node n);
  /** convert to Skolem form
   *
   * @param n The term or formula to convert to Skolem form described above
   * @return n in Skolem form.
   */
  static Node getSkolemForm(Node n);
  /** convert to witness form vector */
  static void convertToWitnessFormVec(std::vector<Node>& vec);
  /** convert to Skolem form vector */
  static void convertToSkolemFormVec(std::vector<Node>& vec);

 private:
  /** Convert to witness or skolem form */
  static Node convertInternal(Node n, bool toWitness);
  /** Get or make skolem attribute for witness term w */
  static Node getOrMakeSkolem(Node w,
                              const std::string& prefix,
                              const std::string& comment,
                              int flags);
};

}  // namespace CVC4

#endif /* CVC4__EXPR__PROOF_SKOLEM_CACHE_H */
