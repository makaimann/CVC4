/*********************  */
/*! \file extminisat.cpp
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
** Implementation of the unmodified minisat for cvc4 (bitvectors).
**/

#ifdef CVC4_USE_EXTMINISAT

#include "prop/extminisat.h"

#include "proof/clause_id.h"
#include "proof/sat_proof.h"

// include things from minisat
#include <minisat/simp/SimpSolver.h>
#include <minisat/core/SolverTypes.h>

namespace CVC4 {
namespace prop {

namespace {

::Minisat::Lit toInternalLit(SatLiteral lit)
{
  if (lit == undefSatLiteral)
    {
      return ::Minisat::lit_Undef;
    }
  return ::Minisat::mkLit(lit.getSatVariable(), lit.isNegated());
}

  SatValue toSatLiteralValue(::Minisat::lbool res)
  {
    if (res == ::Minisat::l_True) return SAT_VALUE_TRUE;
    if (res == ::Minisat::l_Undef) return SAT_VALUE_UNKNOWN;
    Assert(res == ::Minisat::l_False);
    return SAT_VALUE_FALSE;
  }

  SatValue toSatLiteralValue(bool res)
  {
    if (res) return SAT_VALUE_TRUE;
    return SAT_VALUE_FALSE;
  }

  void toInternalClause(SatClause& clause,
                        ::Minisat::vec<::Minisat::Lit>& internal_clause)
  {
    for (unsigned i = 0; i < clause.size(); ++i)
      {
        internal_clause.push(toInternalLit(clause[i]));
      }
    Assert(clause.size() == (unsigned)internal_clause.size());
  }

} // helper functions

ExtMinisatSolver::ExtMinisatSolver(StatisticsRegistry* registry,
                                   const std::string& name)
  : d_solver(new ::Minisat::SimpSolver()),
  d_numVariables(0),
  d_okay(true),
  d_statistics(registry, name)
{
  d_true = newVar();
  d_false = newVar();

  d_solver->addClause(::Minisat::mkLit(d_true, false));
  d_solver->addClause(::Minisat::mkLit(d_false, true));
}

ExtMinisatSolver::~ExtMinisatSolver() {}

ClauseId ExtMinisatSolver::addClause(SatClause& clause, bool removable){
  Debug("sat::extminisat") << "Add clause " << clause << "\n";

  if (!d_okay) {
    Debug("sat::extminisat") << "Solver unsat: not adding clause.\n";
    return ClauseIdError;
  }

  ++(d_statistics.d_clausesAdded);

  ::Minisat::vec<::Minisat::Lit> internal_clause;
  toInternalClause(clause, internal_clause);
  bool res = d_solver->addClause(internal_clause);
  d_okay &= res;
  return ClauseIdError;
}

bool ExtMinisatSolver::ok() const {
  return d_okay;
}

ClauseId ExtMinisatSolver::addXorClause(SatClause& clause,
                                        bool rhs,
                                        bool removable)
{
  Unreachable("Minisat does not support adding XOR clauses.");
}

SatVariable ExtMinisatSolver::newVar(bool isTheoryAtom, bool preRegister, bool canErase){
  d_solver->newVar();
  ++d_numVariables;
  Assert (d_numVariables == d_solver->nVars());
  return d_numVariables - 1;
}

SatVariable ExtMinisatSolver::trueVar() {
  return d_true;
}

SatVariable ExtMinisatSolver::falseVar() {
  return d_false;
}

void ExtMinisatSolver::markUnremovable(SatLiteral lit) {
  d_solver->setFrozen(::Minisat::var(toInternalLit(lit)), true);
}

void ExtMinisatSolver::interrupt(){
  d_solver->interrupt();
}

SatValue ExtMinisatSolver::solve()
{
  TimerStat::CodeTimer solveTimer(d_statistics.d_solveTime);
  ++d_statistics.d_statCallsToSolve;
  return toSatLiteralValue(d_solver->solve());
}

SatValue ExtMinisatSolver::solve(long unsigned int& resource){
  // Trace("limit") << "ExtMinisatSolver::solve(): have limit of " << resource << " conflicts" << std::endl;
  // TimerStat::CodeTimer solveTimer(d_statistics.d_solveTime);
  // ++d_statistics.d_statCallsToSolve;
  // if(resource == 0) {
  //   d_solver->budgetOff();
  // } else {
  //   d_solver->setConfBudget(resource);
  // }
  //  BVMinisat::vec<BVMinisat::Lit> empty;
  // unsigned long conflictsBefore = d_solver->conflicts;
  // SatValue result = toSatLiteralValue(d_solver->solveLimited());
  // d_solver->clearInterrupt();
  // resource = d_solver->conflicts - conflictsBefore;
  // Trace("limit") << "<ExtMinisatSolver::solve(): it took " << resource << " conflicts" << std::endl;

  Unreachable("Not sure if implementation above was correct.");
  return toSatLiteralValue(d_solver->solve());;
}

SatValue ExtMinisatSolver::solve(const std::vector<SatLiteral>& assumptions)
{
  TimerStat::CodeTimer codeTimer(d_statistics.d_solveTime);
  ::Minisat::vec<::Minisat::Lit> assumpts;
  for (const SatLiteral& lit : assumptions)
    {
      assumpts.push(toInternalLit(lit);
    }
  ++d_statistics.d_statCallsToSolve;
  return toSatLiteralValue(d_solver->solve(&assumpts));
}

SatValue ExtMinisatSolver::value(SatLiteral l){
  return toSatLiteralValue(d_solver->modelValue(toInternalLit(l)));
}

SatValue ExtMinisatSolver::modelValue(SatLiteral l) {
  return toSatLiteralValue(d_solver->modelValue(toInternalLit(l)));
}

unsigned ExtMinisatSolver::getAssertionLevel() const {
  Unreachable("Not sure if standard for minisat or not");
  return -1;
}

// TODO: double check if possible -- probably modified version only
// unsigned ExtMinisatSolver::getAssertionLevel() const {
//   return d_solver->getAssertionLevel();
// }

ExtMinisatSolver::Statistics::Statistics(StatisticsRegistry* registry,
                                            const std::string& prefix) :
  d_registry(registry),
  d_statCallsToSolve("theory::bv::"+prefix+"::extminisat::calls_to_solve", 0),
  d_xorClausesAdded("theory::bv::"+prefix+"::extminisat::xor_clauses", 0),
  d_clausesAdded("theory::bv::"+prefix+"::extminisat::clauses", 0),
  d_solveTime("theory::bv::"+prefix+"::extminisat::solve_time"),
  d_registerStats(!prefix.empty())
{
  if (!d_registerStats)
    return;

  d_registry->registerStat(&d_statCallsToSolve);
  d_registry->registerStat(&d_xorClausesAdded);
  d_registry->registerStat(&d_clausesAdded);
  d_registry->registerStat(&d_solveTime);
}

ExtMinisatSolver::Statistics::~Statistics() {
  if (!d_registerStats)
    return;
  d_registry->unregisterStat(&d_statCallsToSolve);
  d_registry->unregisterStat(&d_xorClausesAdded);
  d_registry->unregisterStat(&d_clausesAdded);
  d_registry->unregisterStat(&d_solveTime);
}

// probably only works with modified solver
// Statistics for ExtMinisatSolver -- same as MinisatSatSolver
// ExtMinisatSolver::Statistics::Statistics(StatisticsRegistry* registry) :
//   d_registry(registry),
//   d_statStarts("sat::starts"),
//   d_statDecisions("sat::decisions"),
//   d_statRndDecisions("sat::rnd_decisions"),
//   d_statPropagations("sat::propagations"),
//   d_statConflicts("sat::conflicts"),
//   d_statClausesLiterals("sat::clauses_literals"),
//   d_statLearntsLiterals("sat::learnts_literals"),
//   d_statMaxLiterals("sat::max_literals"),
//   d_statTotLiterals("sat::tot_literals")
// {
//   d_registry->registerStat(&d_statStarts);
//   d_registry->registerStat(&d_statDecisions);
//   d_registry->registerStat(&d_statRndDecisions);
//   d_registry->registerStat(&d_statPropagations);
//   d_registry->registerStat(&d_statConflicts);
//   d_registry->registerStat(&d_statClausesLiterals);
//   d_registry->registerStat(&d_statLearntsLiterals);
//   d_registry->registerStat(&d_statMaxLiterals);
//   d_registry->registerStat(&d_statTotLiterals);
// }

// ExtMinisatSolver::Statistics::~Statistics() {
//   d_registry->unregisterStat(&d_statStarts);
//   d_registry->unregisterStat(&d_statDecisions);
//   d_registry->unregisterStat(&d_statRndDecisions);
//   d_registry->unregisterStat(&d_statPropagations);
//   d_registry->unregisterStat(&d_statConflicts);
//   d_registry->unregisterStat(&d_statClausesLiterals);
//   d_registry->unregisterStat(&d_statLearntsLiterals);
//   d_registry->unregisterStat(&d_statMaxLiterals);
//   d_registry->unregisterStat(&d_statTotLiterals);
// }

// void ExtMinisatSolver::Statistics::init(::Minisat::SimpSolver* d_minisat){
//   d_statStarts.setData(d_minisat->starts);
//   d_statDecisions.setData(d_minisat->decisions);
//   d_statRndDecisions.setData(d_minisat->rnd_decisions);
//   d_statPropagations.setData(d_minisat->propagations);
//   d_statConflicts.setData(d_minisat->conflicts);
//   d_statClausesLiterals.setData(d_minisat->clauses_literals);
//   d_statLearntsLiterals.setData(d_minisat->learnts_literals);
//   d_statMaxLiterals.setData(d_minisat->max_literals);
//   d_statTotLiterals.setData(d_minisat->tot_literals);
// }

} // namespace prop
} // namespace CVC4
#endif
