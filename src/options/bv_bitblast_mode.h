/*********************                                                        */
/*! \file bv_bitblast_mode.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Liana Hadarean, Alex Ozdemir, Mathias Preiner
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Bitblasting modes for bit-vector solver.
 **
 ** Bitblasting modes for bit-vector solver.
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__BV__BITBLAST_MODE_H
#define __CVC4__THEORY__BV__BITBLAST_MODE_H

#include <iosfwd>

namespace CVC4 {
namespace theory {
namespace bv {

/** Enumeration of bit-blasting modes */
enum BitblastMode {

  /**
   * Lazy bit-blasting that separates boolean reasoning
   * from term reasoning.
   */
  BITBLAST_MODE_LAZY,

  /**
   * Bit-blast eagerly to the bit-vector SAT solver.
   */
  BITBLAST_MODE_EAGER
};/* enum BitblastMode */

/** Enumeration of bit-vector equality slicer mode */
enum BvSlicerMode {

  /**
   * Force the slicer on. 
   */
  BITVECTOR_SLICER_ON, 

  /**
   * Slicer off. 
   */
  BITVECTOR_SLICER_OFF, 

  /**
   * Auto enable slicer if problem has only equalities.
   */
  BITVECTOR_SLICER_AUTO

};/* enum BvSlicerMode */

/** Enumeration of sat solvers that can be used. */
enum SatSolverMode
{
  SAT_SOLVER_MINISAT,
  SAT_SOLVER_CRYPTOMINISAT,
  SAT_SOLVER_CADICAL,
}; /* enum SatSolver */

/**
 * When the BV solver does eager bitblasting backed by CryptoMiniSat, proofs
 * can be written in a variety of formats.
 */
enum BvProofFormat
{
  /**
   * Write extended resolution proofs.
   */
  BITVECTOR_PROOF_ER,
  /**
   * Write DRAT proofs.
   */
  BITVECTOR_PROOF_DRAT,
  /**
   * Write LRAT proofs.
   */
  BITVECTOR_PROOF_LRAT,
};

/**
 * When the BV solver does eager bit-blasting backed by DRAT-producing SAT solvers, proofs
 * can be written in a variety of formats.
 */
enum BvOptimizeSatProof
{
  /**
   * Do not optimize the SAT proof.
   */
  BITVECTOR_OPTIMIZE_SAT_PROOF_NONE = 0,
  /**
   * Optimize the SAT proof, but do not shrink the formula
   */
  BITVECTOR_OPTIMIZE_SAT_PROOF_PROOF = 1,
  /**
   * Optimize the SAT proof and shrink the formula
   */
  BITVECTOR_OPTIMIZE_SAT_PROOF_FORMULA = 2,
};


}/* CVC4::theory::bv namespace */
}/* CVC4::theory namespace */

std::ostream& operator<<(std::ostream& out, theory::bv::BitblastMode mode);
std::ostream& operator<<(std::ostream& out, theory::bv::BvSlicerMode mode);
std::ostream& operator<<(std::ostream& out, theory::bv::SatSolverMode mode);
std::ostream& operator<<(std::ostream& out, theory::bv::BvProofFormat format);
std::ostream& operator<<(std::ostream& out, theory::bv::BvOptimizeSatProof level);

}/* CVC4 namespace */

#endif /* __CVC4__THEORY__BV__BITBLAST_MODE_H */
