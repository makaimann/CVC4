(set-logic ALL_SUPPORTED)
(set-info :status sat)
(set-option :produce-models true)
(set-option :sets-ext true)
(declare-fun A () (Set Bool))
(declare-fun B () (Set Bool))
(declare-fun universe () (Set Bool))
(assert (= (card A) 2))
(assert (= (card B) 2))
(assert (= universe (as univset (Set Bool))))
(check-sat)
