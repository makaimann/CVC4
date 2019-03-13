/*********************                                                        */
/*! \file smt_engine_check_proof.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Morgan Deters, Mark Laws, Guy Katz
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2018 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief [[ Add one-line brief description here ]]
 **
 ** [[ Add lengthier description here ]]
 ** \todo document this file
 **/

#include <sstream>
#include <string>

#include "base/configuration_private.h"
#include "base/cvc4_assert.h"
#include "base/output.h"
#include "smt/smt_engine.h"
#include "util/proof.h"
#include "util/statistics_registry.h"

#if (IS_LFSC_BUILD && IS_PROOFS_BUILD)
#include "lfscc.h"
#endif

using namespace CVC4;
using namespace std;

namespace CVC4 {
namespace proof {
extern const char *const plf_signatures;
}  // namespace proof
}  // namespace CVC4

void SmtEngine::checkProof() {

#if (IS_LFSC_BUILD && IS_PROOFS_BUILD)

  Chat() << "generating proof..." << endl;

  const Proof& pf = getProof();

  Chat() << "checking proof..." << endl;

  std::string logicString = d_logic.getLogicString();

  if (!(
        // Pure logics
        logicString == "QF_UF" ||
        logicString == "QF_AX" ||
        logicString == "QF_BV" ||
        // Non-pure logics
        logicString == "QF_AUF" ||
        logicString == "QF_UFBV" ||
        logicString == "QF_ABV" ||
        logicString == "QF_AUFBV"
        )) {
    // This logic is not yet supported
    Notice() << "Notice: no proof-checking for " << logicString << " proofs yet" << endl;
    return;
  }

  std::stringstream pfStream;
  pfStream << proof::plf_signatures << endl;
  pf.toStream(pfStream);
  lfscc_init();
  lfscc_check_file(pfStream, false, false, false, false, false, false, false);
  // FIXME: we should actually call lfscc_cleanup here, but lfscc_cleanup
  // segfaults on regress0/bv/core/bitvec7.smt
  //lfscc_cleanup();

#else /* (IS_LFSC_BUILD && IS_PROOFS_BUILD) */
  Unreachable("This version of CVC4 was built without proof support; cannot check proofs.");
#endif /* (IS_LFSC_BUILD && IS_PROOFS_BUILD) */
}
