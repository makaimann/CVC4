(set-info :smt-lib-version 2.5)
(set-logic QF_S)
(set-info :status unsat)
(set-option :strings-exp true)
(declare-fun var0 () String)
(assert (str.in.re var0 (re.* (re.* (str.to.re "0")))))
(assert (not (str.in.re var0 (re.union (re.+ (str.to.re "0")) (re.* (str.to.re "1"))))))
(check-sat)
