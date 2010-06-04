/*********************                                                        */
/*! \file command.h
 ** \verbatim
 ** Original author: mdeters
 ** Major contributors: cconway, dejan
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Implementation of the command pattern on SmtEngines.
 **
 ** Implementation of the command pattern on SmtEngines.  Command
 ** objects are generated by the parser (typically) to implement the
 ** commands in parsed input (see Parser::parseNextCommand()), or by
 ** client code.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__COMMAND_H
#define __CVC4__COMMAND_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "expr/expr.h"
#include "expr/type.h"
#include "util/result.h"
#include "util/sexpr.h"

namespace CVC4 {

class SmtEngine;
class Command;

inline std::ostream& operator<<(std::ostream&, const Command&) CVC4_PUBLIC;
std::ostream& operator<<(std::ostream&, const Command*) CVC4_PUBLIC;

/** The status an SMT benchmark can have */
enum BenchmarkStatus {
  /** Benchmark is satisfiable */
  SMT_SATISFIABLE,
  /** Benchmark is unsatisfiable */
  SMT_UNSATISFIABLE,
  /** The status of the benchmark is unknown */
  SMT_UNKNOWN
};

inline std::ostream& operator<<(std::ostream& out,
                                BenchmarkStatus status)
  CVC4_PUBLIC;

class CVC4_PUBLIC Command {
public:
  virtual void invoke(CVC4::SmtEngine* smt_engine) = 0;
  virtual ~Command() {};
  virtual void toStream(std::ostream&) const = 0;
  std::string toString() const;
};/* class Command */

class CVC4_PUBLIC EmptyCommand : public Command {
public:
  EmptyCommand(std::string name = "");
  void invoke(CVC4::SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
private:
  std::string d_name;
};/* class EmptyCommand */

class CVC4_PUBLIC AssertCommand : public Command {
public:
  AssertCommand(const BoolExpr& e);
  void invoke(CVC4::SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
protected:
  BoolExpr d_expr;
};/* class AssertCommand */

class CVC4_PUBLIC PushCommand : public Command {
public:
  void invoke(CVC4::SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
};/* class PushCommand */

class CVC4_PUBLIC PopCommand : public Command {
public:
  void invoke(CVC4::SmtEngine* smtEngine);
  void toStream(std::ostream& out) const;
};/* class PopCommand */


class CVC4_PUBLIC DeclarationCommand : public EmptyCommand {
public:
  DeclarationCommand(const std::string& id, const Type& t);
  DeclarationCommand(const std::vector<std::string>& ids, const Type& t);
  void toStream(std::ostream& out) const;
protected:
  std::vector<std::string> d_declaredSymbols;
  Type d_type;
};

class CVC4_PUBLIC CheckSatCommand : public Command {
public:
  CheckSatCommand(const BoolExpr& expr);
  void invoke(SmtEngine* smt);
  Result getResult();
  void toStream(std::ostream& out) const;
protected:
  BoolExpr d_expr;
  Result d_result;
};/* class CheckSatCommand */

class CVC4_PUBLIC QueryCommand : public Command {
public:
  QueryCommand(const BoolExpr& e);
  void invoke(SmtEngine* smt);
  Result getResult();
  void toStream(std::ostream& out) const;
protected:
  BoolExpr d_expr;
  Result d_result;
};/* class QueryCommand */

class CVC4_PUBLIC SetBenchmarkStatusCommand : public Command {
public:
  SetBenchmarkStatusCommand(BenchmarkStatus status);
  void invoke(SmtEngine* smt);
  void toStream(std::ostream& out) const;
protected:
  BenchmarkStatus d_status;
};/* class SetBenchmarkStatusCommand */


class CVC4_PUBLIC SetBenchmarkLogicCommand : public Command {
public:
  SetBenchmarkLogicCommand(std::string logic);
  void invoke(SmtEngine* smt);
  void toStream(std::ostream& out) const;
protected:
  std::string d_logic;
};/* class SetBenchmarkLogicCommand */

class CVC4_PUBLIC SetInfoCommand : public Command {
public:
  SetInfoCommand(std::string flag, SExpr& sexpr);
  void invoke(SmtEngine* smt);
  void toStream(std::ostream& out) const;
protected:
  std::string d_flag;
  SExpr d_sexpr;
};/* class SetInfoCommand */

class CVC4_PUBLIC CommandSequence : public Command {
public:
  CommandSequence();
  ~CommandSequence();
  void invoke(SmtEngine* smt);
  void addCommand(Command* cmd);
  void toStream(std::ostream& out) const;

  typedef std::vector<Command*>::iterator iterator;
  typedef std::vector<Command*>::const_iterator const_iterator;

  const_iterator begin() const { return d_commandSequence.begin(); }
  const_iterator end() const { return d_commandSequence.end(); }

  iterator begin() { return d_commandSequence.begin(); }
  iterator end() { return d_commandSequence.end(); }

private:
  /** All the commands to be executed (in sequence) */
  std::vector<Command*> d_commandSequence;
  /** Next command to be executed */
  unsigned int d_index;
};/* class CommandSequence */

}/* CVC4 namespace */

/* =========== inline function definitions =========== */

#include "smt/smt_engine.h"

namespace CVC4 {

inline std::ostream& operator<<(std::ostream& out, const Command& c) {
  c.toStream(out);
  return out;
}

/* class Command */

inline std::string Command::toString() const {
  std::stringstream ss;
  toStream(ss);
  return ss.str();
}

/* class EmptyCommand */

inline EmptyCommand::EmptyCommand(std::string name) :
  d_name(name) {
}

inline void EmptyCommand::invoke(SmtEngine* smtEngine) {
}

inline void EmptyCommand::toStream(std::ostream& out) const {
  out << "EmptyCommand(" << d_name << ")";
}

/* class AssertCommand */

inline AssertCommand::AssertCommand(const BoolExpr& e) :
  d_expr(e) {
}

inline void AssertCommand::invoke(SmtEngine* smtEngine) {
  smtEngine->assertFormula(d_expr);
}

inline void AssertCommand::toStream(std::ostream& out) const {
  out << "Assert(" << d_expr << ")";
}

/* class CheckSatCommand */

inline CheckSatCommand::CheckSatCommand(const BoolExpr& expr) :
  d_expr(expr) {
}

inline void CheckSatCommand::invoke(SmtEngine* smtEngine) {
  d_result = smtEngine->checkSat(d_expr);
}

inline Result CheckSatCommand::getResult() {
  return d_result;
}

/* class QueryCommand */

inline QueryCommand::QueryCommand(const BoolExpr& e) :
  d_expr(e) {
}

inline void QueryCommand::invoke(CVC4::SmtEngine* smtEngine) {
  d_result = smtEngine->query(d_expr);
}

inline Result QueryCommand::getResult() {
  return d_result;
}

inline void QueryCommand::toStream(std::ostream& out) const {
  out << "Query(";
  d_expr.printAst(out, 0);
  out << ")";
}

/* class CommandSequence */

inline CommandSequence::CommandSequence() :
  d_index(0) {
}

inline void CommandSequence::addCommand(Command* cmd) {
  d_commandSequence.push_back(cmd);
}

/* class DeclarationCommand */

inline DeclarationCommand::DeclarationCommand(const std::string& id, const Type& t) :
  d_type(t)
{
  d_declaredSymbols.push_back(id);
}

inline DeclarationCommand::DeclarationCommand(const std::vector<std::string>& ids, const Type& t) :
  d_declaredSymbols(ids),
  d_type(t)
{
}

/* class SetBenchmarkStatusCommand */

inline SetBenchmarkStatusCommand::SetBenchmarkStatusCommand(BenchmarkStatus status) :
  d_status(status) {
}

inline void SetBenchmarkStatusCommand::invoke(SmtEngine* smt) {
  // FIXME: TODO: something to be done with the status
}

inline void SetBenchmarkStatusCommand::toStream(std::ostream& out) const {
  out << "SetBenchmarkStatus(" << d_status << ")";
}

/* class SetBenchmarkLogicCommand */

inline SetBenchmarkLogicCommand::SetBenchmarkLogicCommand(std::string logic) :
  d_logic(logic) {
}

inline void SetBenchmarkLogicCommand::invoke(SmtEngine* smt) {
  // FIXME: TODO: something to be done with the logic
}

inline void SetBenchmarkLogicCommand::toStream(std::ostream& out) const {
  out << "SetBenchmarkLogic(" << d_logic << ")";
}

inline SetInfoCommand::SetInfoCommand(std::string flag, SExpr& sexpr) :
  d_flag(flag),
  d_sexpr(sexpr) {
}

inline void SetInfoCommand::invoke(SmtEngine* smt) { }

inline void SetInfoCommand::toStream(std::ostream& out) const {
  out << "SetInfo(" << d_flag << ", " << d_sexpr << ")";
}

/* output stream insertion operator for benchmark statuses */
inline std::ostream& operator<<(std::ostream& out,
                                BenchmarkStatus status) {
  switch(status) {

  case SMT_SATISFIABLE:
    return out << "sat";

  case SMT_UNSATISFIABLE:
    return out << "unsat";

  case SMT_UNKNOWN:
    return out << "unknown";

  default:
    return out << "SetBenchmarkStatusCommand::[UNKNOWNSTATUS!]";
  }
}

}/* CVC4 namespace */

#endif /* __CVC4__COMMAND_H */
