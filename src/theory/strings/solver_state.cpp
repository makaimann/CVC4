/*********************                                                        */
/*! \file solver_state.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Implementation of the solver state of the theory of strings.
 **/

#include "theory/strings/solver_state.h"

#include "theory/strings/theory_strings_utils.h"

using namespace std;
using namespace CVC4::context;
using namespace CVC4::kind;

namespace CVC4 {
namespace theory {
namespace strings {

EqcInfo::EqcInfo(context::Context* c)
    : d_lengthTerm(c),
      d_codeTerm(c),
      d_cardinalityLemK(c),
      d_normalizedLength(c),
      d_prefixC(c),
      d_suffixC(c)
{
}

Node EqcInfo::addEndpointConst(Node t, Node c, bool isSuf)
{
  // check conflict
  Node prev = isSuf ? d_suffixC : d_prefixC;
  if (!prev.isNull())
  {
    Trace("strings-eager-pconf-debug") << "Check conflict " << prev << ", " << t
                                       << " post=" << isSuf << std::endl;
    Node prevC = utils::getConstantEndpoint(prev, isSuf);
    Assert(!prevC.isNull());
    Assert(prevC.getKind() == CONST_STRING);
    if (c.isNull())
    {
      c = utils::getConstantEndpoint(t, isSuf);
      Assert(!c.isNull());
    }
    Assert(c.getKind() == CONST_STRING);
    bool conflict = false;
    // if the constant prefixes are different
    if (c != prevC)
    {
      // conflicts between constants should be handled by equality engine
      Assert(!t.isConst() || !prev.isConst());
      Trace("strings-eager-pconf-debug")
          << "Check conflict constants " << prevC << ", " << c << std::endl;
      const String& ps = prevC.getConst<String>();
      const String& cs = c.getConst<String>();
      unsigned pvs = ps.size();
      unsigned cvs = cs.size();
      if (pvs == cvs || (pvs > cvs && t.isConst())
          || (cvs > pvs && prev.isConst()))
      {
        // If equal length, cannot be equal due to node check above.
        // If one is fully constant and has less length than the other, then the
        // other will not fit and we are in conflict.
        conflict = true;
      }
      else
      {
        const String& larges = pvs > cvs ? ps : cs;
        const String& smalls = pvs > cvs ? cs : ps;
        if (isSuf)
        {
          conflict = !larges.hasSuffix(smalls);
        }
        else
        {
          conflict = !larges.hasPrefix(smalls);
        }
      }
      if (!conflict && (pvs > cvs || prev.isConst()))
      {
        // current is subsumed, either shorter prefix or the other is a full
        // constant
        return Node::null();
      }
    }
    else if (!t.isConst())
    {
      // current is subsumed since the other may be a full constant
      return Node::null();
    }
    if (conflict)
    {
      Trace("strings-eager-pconf")
          << "Conflict for " << prevC << ", " << c << std::endl;
      std::vector<Node> ccs;
      Node r[2];
      for (unsigned i = 0; i < 2; i++)
      {
        Node tp = i == 0 ? t : prev;
        if (tp.getKind() == STRING_IN_REGEXP)
        {
          ccs.push_back(tp);
          r[i] = tp[0];
        }
        else
        {
          r[i] = tp;
        }
      }
      if (r[0] != r[1])
      {
        ccs.push_back(r[0].eqNode(r[1]));
      }
      Assert(!ccs.empty());
      Node ret =
          ccs.size() == 1 ? ccs[0] : NodeManager::currentNM()->mkNode(AND, ccs);
      Trace("strings-eager-pconf")
          << "String: eager prefix conflict: " << ret << std::endl;
      return ret;
    }
  }
  if (isSuf)
  {
    d_suffixC = t;
  }
  else
  {
    d_prefixC = t;
  }
  return Node::null();
}

SolverState::SolverState(context::Context* c,
                         eq::EqualityEngine& ee,
                         Valuation& v)
    : d_context(c),
      d_ee(ee),
      d_valuation(v),
      d_conflict(c, false),
      d_pendingConflict(c)
{
}
SolverState::~SolverState()
{
  for (std::pair<const Node, EqcInfo*>& it : d_eqcInfo)
  {
    delete it.second;
  }
}

Node SolverState::getRepresentative(Node t) const
{
  if (d_ee.hasTerm(t))
  {
    return d_ee.getRepresentative(t);
  }
  return t;
}

bool SolverState::hasTerm(Node a) const { return d_ee.hasTerm(a); }

bool SolverState::areEqual(Node a, Node b) const
{
  if (a == b)
  {
    return true;
  }
  else if (hasTerm(a) && hasTerm(b))
  {
    return d_ee.areEqual(a, b);
  }
  return false;
}

bool SolverState::areDisequal(Node a, Node b) const
{
  if (a == b)
  {
    return false;
  }
  else if (hasTerm(a) && hasTerm(b))
  {
    Node ar = d_ee.getRepresentative(a);
    Node br = d_ee.getRepresentative(b);
    return (ar != br && ar.isConst() && br.isConst())
           || d_ee.areDisequal(ar, br, false);
  }
  Node ar = getRepresentative(a);
  Node br = getRepresentative(b);
  return ar != br && ar.isConst() && br.isConst();
}

eq::EqualityEngine* SolverState::getEqualityEngine() const { return &d_ee; }

EqcInfo* SolverState::getOrMakeEqcInfo(Node eqc, bool doMake)
{
  std::map<Node, EqcInfo*>::iterator eqc_i = d_eqcInfo.find(eqc);
  if (eqc_i != d_eqcInfo.end())
  {
    return eqc_i->second;
  }
  if (doMake)
  {
    EqcInfo* ei = new EqcInfo(d_context);
    d_eqcInfo[eqc] = ei;
    return ei;
  }
  return nullptr;
}

void SolverState::addEndpointsToEqcInfo(Node t, Node concat, Node eqc)
{
  Assert(concat.getKind() == STRING_CONCAT
         || concat.getKind() == REGEXP_CONCAT);
  EqcInfo* ei = nullptr;
  // check each side
  for (unsigned r = 0; r < 2; r++)
  {
    unsigned index = r == 0 ? 0 : concat.getNumChildren() - 1;
    Node c = utils::getConstantComponent(concat[index]);
    if (!c.isNull())
    {
      if (ei == nullptr)
      {
        ei = getOrMakeEqcInfo(eqc);
      }
      Trace("strings-eager-pconf-debug")
          << "New term: " << concat << " for " << t << " with prefix " << c
          << " (" << (r == 1) << ")" << std::endl;
      setPendingConflictWhen(ei->addEndpointConst(t, c, r == 1));
    }
  }
}

Node SolverState::getLengthExp(Node t, std::vector<Node>& exp, Node te)
{
  Assert(areEqual(t, te));
  Node lt = utils::mkNLength(te);
  if (hasTerm(lt))
  {
    // use own length if it exists, leads to shorter explanation
    return lt;
  }
  EqcInfo* ei = getOrMakeEqcInfo(t, false);
  Node lengthTerm = ei ? ei->d_lengthTerm : Node::null();
  if (lengthTerm.isNull())
  {
    // typically shouldnt be necessary
    lengthTerm = t;
  }
  Debug("strings") << "SolverState::getLengthTerm " << t << " is " << lengthTerm
                   << std::endl;
  if (te != lengthTerm)
  {
    exp.push_back(te.eqNode(lengthTerm));
  }
  return Rewriter::rewrite(
      NodeManager::currentNM()->mkNode(STRING_LENGTH, lengthTerm));
}

Node SolverState::getLength(Node t, std::vector<Node>& exp)
{
  return getLengthExp(t, exp, t);
}

void SolverState::setConflict() { d_conflict = true; }
bool SolverState::isInConflict() const { return d_conflict; }

void SolverState::setPendingConflictWhen(Node conf)
{
  if (!conf.isNull() && d_pendingConflict.get().isNull())
  {
    d_pendingConflict = conf;
  }
}

Node SolverState::getPendingConflict() const { return d_pendingConflict; }

std::pair<bool, Node> SolverState::entailmentCheck(TheoryOfMode mode, TNode lit)
{
  return d_valuation.entailmentCheck(mode, lit);
}

void SolverState::separateByLength(const std::vector<Node>& n,
                                   std::vector<std::vector<Node> >& cols,
                                   std::vector<Node>& lts)
{
  unsigned leqc_counter = 0;
  std::map<Node, unsigned> eqc_to_leqc;
  std::map<unsigned, Node> leqc_to_eqc;
  std::map<unsigned, std::vector<Node> > eqc_to_strings;
  NodeManager* nm = NodeManager::currentNM();
  for (const Node& eqc : n)
  {
    Assert(d_ee.getRepresentative(eqc) == eqc);
    EqcInfo* ei = getOrMakeEqcInfo(eqc, false);
    Node lt = ei ? ei->d_lengthTerm : Node::null();
    if (!lt.isNull())
    {
      lt = nm->mkNode(STRING_LENGTH, lt);
      Node r = d_ee.getRepresentative(lt);
      if (eqc_to_leqc.find(r) == eqc_to_leqc.end())
      {
        eqc_to_leqc[r] = leqc_counter;
        leqc_to_eqc[leqc_counter] = r;
        leqc_counter++;
      }
      eqc_to_strings[eqc_to_leqc[r]].push_back(eqc);
    }
    else
    {
      eqc_to_strings[leqc_counter].push_back(eqc);
      leqc_counter++;
    }
  }
  for (const std::pair<const unsigned, std::vector<Node> >& p : eqc_to_strings)
  {
    cols.push_back(std::vector<Node>());
    cols.back().insert(cols.back().end(), p.second.begin(), p.second.end());
    lts.push_back(leqc_to_eqc[p.first]);
  }
}

}  // namespace strings
}  // namespace theory
}  // namespace CVC4
