(set-logic QF_ALL)
(set-info :status sat)
(set-option :sets-ext true)
(declare-fun A () (Set Bool))
(declare-fun x () Bool)
(assert (member (member x A) A))
(assert (> (card A) 1))
(declare-fun U () (Set Bool))
(assert (= U (as univset (Set Bool))))
(check-sat)
