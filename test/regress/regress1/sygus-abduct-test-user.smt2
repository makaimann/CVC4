; COMMAND-LINE: --produce-abducts
; SCRUBBER: grep -v -E '(\(define-fun)'
; EXIT: 0
(set-logic QF_UFLIRA)
(set-option :produce-abducts true)
(declare-fun n () Int)
(declare-fun m () Int)
(declare-fun x () Int)
(declare-fun y () Int)

(assert (>= n 1))
(assert (and (<= n x)(<= x (+ n 5))))
(assert (and (<= 1 y)(<= y m)))

; Generate a predicate A that is consistent with the above axioms (i.e.
; their conjunction is SAT), and is such that the conjunction of the above
; axioms, A and the conjecture below are UNSAT.
; The signature of A is below grammar.
(get-abduct A (< x y)

; the grammar for the abduct-to-synthesize
((Start Bool) (StartInt Int))
(
(Start Bool ((< StartInt StartInt)))
(StartInt Int (n m (+ StartInt StartInt) 0 1))
)

)
