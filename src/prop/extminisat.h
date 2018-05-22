/*********************                                                        */
/*! \file extminisat.h
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the CVC4 project.
** Copyright (c) 2009-2018 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file COPYING in the top-level source
** directory for licensing information.\endverbatim
**
** \brief SAT Solver.
**
** Implementation of the unmodified minisat sat solver for cvc4 (bitvectors).
**/

#include "cvc4_private.h"

#ifndef __CVC4__PROP__EXTMINISAT_H
#define __CVC4__PROP__EXTMINISAT_H

#ifdef CVC4_USE_EXTMINISAT

#include "prop/sat_solver.h"

namespace Minisat {
  // don't want to clash with other minisat names
  class SimpSolver;
}

namespace CVC4 {
namespace prop {

class ExtMinisatSolver : public SatSolver
{
 private:
  std::unique_ptr<::Minisat::SimpSolver> d_solver;
  unsigned d_numVariables;
  bool d_okay;
  SatVariable d_true;
  SatVariable d_false;
 public:
  ExtMinisatSolver(StatisticsRegistry* registry, const std::string& name = "");

  ~ExtMinisatSolver() override;

  ClauseId addClause(SatClause& cluase, bool removable) override;
  ClauseId addXorClause(SatClause& clause, bool rhs, bool removable) override;

  bool nativeXor() override {return false;}

  SatVariable newVar(bool isTheoryAtom = false, bool preRegister = false, bool canErase = true) override;

  SatVariable trueVar() override;
  SatVariable falseVar() override;

  void markUnremovable(SatLiteral lit);

  void interrupt() override;

  SatValue solve() override;
  SatValue solve(long unsigned int&) override;
  SatValue solve(const std::vector<SatLiteral>& assumptions) override;

  bool ok() const override;
  SatValue value(SatLiteral l) override;
  SatValue modelValue(SatLiteral l) override;

  unsigned getAssertionLevel() const override;

  class Statistics {
  public:
    StatisticsRegistry* d_registry;
    IntStat d_statCallsToSolve;
    IntStat d_xorClausesAdded;
    IntStat d_clausesAdded;
    TimerStat d_solveTime;
    bool d_registerStats;
    Statistics(StatisticsRegistry* registry,
               const std::string& prefix);
    ~Statistics();
  };

  Statistics d_statistics;
};
}
}

#endif  // CVC4_USE_EXTMINISAT
#endif  // __CVC4__PROP__EXTMINISAT_H
