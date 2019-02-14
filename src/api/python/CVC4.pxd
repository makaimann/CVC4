# import dereference and increment operators
from cython.operator cimport dereference as deref, preincrement as inc
from libc.stdint cimport int32_t, int64_t, uint32_t, uint64_t
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from kinds cimport Kind

cdef extern from "cvc4cpp.cpp":
    pass

cdef extern from "cvc4cpp.h" namespace "CVC4":
    cdef cppclass Options:
        pass
    cdef cppclass RoundingMode:
        pass


cdef extern from "cvc4cpp.h" namespace "CVC4::api":
    cdef cppclass Datatype:
        Datatype() except +
        DatatypeConstructor operator[](const string& name) except +
        DatatypeConstructor getConstructor(const string& name) except +
        OpTerm getConstructorTerm(const string& name) except +
        size_t getNumConstructors() except +
        bint isParametric() except +
        string toString() except +
        cppclass const_iterator:
            const_iterator() except +
            bint operator==(const const_iterator& it) except +
            bint operator!=(const const_iterator& it) except +
            const_iterator& operator++();
            const DatatypeConstructor& operator*() except +
        const_iterator begin() except +
        const_iterator end() except +


    cdef cppclass DatatypeConstructor:
        DatatypeConstructor() except +
        DatatypeSelector operator[](const string& name) except +
        DatatypeSelector getSelector(const string& name) except +
        OpTerm getSelectorTerm(const string& name) except +
        string toString() except +
        cppclass const_iterator:
            const_iterator() except +
            bint operator==(const const_iterator& it) except +
            bint operator!=(const const_iterator& it) except +
            const_iterator& operator++();
            # TODO: Check if the const return type is enforced
            const DatatypeSelector& operator*() except +
        const_iterator begin() except +
        const_iterator end() except +


    cdef cppclass DatatypeConstructorDecl:
        DatatypeConstructorDecl(const string& name) except +
        void addSelector(const DatatypeSelectorDecl& stor) except +
        string toString() except +


    cdef cppclass DatatypeDecl:
        DatatypeDecl(const string& name, bint isCoDatatype) except +
        DatatypeDecl(const string& name, Sort param, bint isCoDatatype) except +
        DatatypeDecl(const string& name, const vector[Sort]& params, bint isCoDatatype) except +
        void addConstructor(const DatatypeConstructorDecl& ctor) except +
        bint isParametric() except +
        string toString() except +


    cdef cppclass DatatypeDeclSelfSort:
        DatatypeDeclSelfSort() except +


    cdef cppclass DatatypeSelector:
        DatatypeSelector() except +
        string toString() except +


    cdef cppclass DatatypeSelectorDecl:
        DatatypeSelectorDecl(const string& name, Sort sort) except +
        DatatypeSelectorDecl(const string& name, DatatypeDeclSelfSort sort) except +
        string toString() except +


    cdef cppclass OpTerm:
        OpTerm() except +
        bint operator==(const OpTerm&) except +
        bint operator!=(const OpTerm&) except +
        Kind getKind() except +
        Sort getSort() except +
        bint isNull() except +
        string toString() except +


    cdef cppclass Result:
    # Note: don't even need constructor
        bint isSat() except +
        bint isUnsat() except +
        bint isSatUnknown() except +
        bint isValid() except +
        bint isInvalid() except +
        bint isValidUnknown() except +
        string getUnknownExplanation() except +
        string toString() except +


    cdef cppclass Solver:
        Solver(Options*) except +
        Sort getBooleanSort() except +
        Sort getIntegerSort() except +
        Sort getRealSort() except +
        Sort getRegExpSort() except +
        Sort getRoundingmodeSort() except +
        Sort getStringSort() except +
        Sort mkArraySort(Sort indexSort, Sort elemSort) except +
        Sort mkBitVectorSort(uint32_t size) except +
        Sort mkFloatingPointSort(uint32_t exp, uint32_t sig) except +
        Sort mkDatatypeSort(DatatypeDecl dtypedecl) except +
        Sort mkFunctionSort(Sort domain, Sort codomain) except +
        Sort mkFunctionSort(const vector[Sort]& sorts, Sort codomain) except +
        Sort mkParamSort(const string& symbol) except +
        Sort mkPredicateSort(const vector[Sort]& sorts) except +
        Sort mkRecordSort(const vector[pair[string, Sort]]& fields) except +
        Sort mkSetSort(Sort elemSort) except +
        Sort mkUninterpretedSort(const string& symbol) except +
        Sort mkSortConstructorSort(const string& symbol, size_t arity) except +
        Sort mkTupleSort(const vector[Sort]& sorts) except +
        Term mkTerm(Kind kind) except +
        Term mkTerm(Kind kind, const vector[Term]& children) except +
        Term mkTerm(Kind kind, OpTerm child, const vector[Term]& children) except +
        OpTerm mkOpTerm(Kind kind, Kind k) except +
        OpTerm mkOpTerm(Kind kind, const string& arg) except +
        OpTerm mkOpTerm(Kind kind, uint32_t arg) except +
        OpTerm mkOpTerm(Kind kind, uint32_t arg1, uint32_t arg2) except +
        Term mkTrue() except +
        Term mkFalse() except +
        Term mkBoolean(bint val) except +
        Term mkPi() except +
        Term mkReal(const string& s) except +
        Term mkRegexpEmpty() except +
        Term mkRegexpSigma() except +
        Term mkEmptySet(Sort s) except +
        Term mkSepNil(Sort sort) except +
        Term mkString(const string& s) except +
        Term mkString(const vector[unsigned]& s) except +
        Term mkUniverseSet(Sort sort) except +
        Term mkBitVector(uint32_t size) except +
        Term mkBitVector(uint32_t size, uint64_t val) except +
        # TODO: fill in missing functions (skipped all the overloaded mkConsts)
        # Term mkConst(RoundingMode rm) except +
        # Term mkConst(Kind kind, Sort arg) except +
        # Term mkConst(Kind kind, Sort arg1, int32_t arg2) except +
        # Term mkConst(Kind kind, bool arg) except +
        # Term mkConst(Kind kind, const string& arg) except +
        # Term mkConst(Kind, kind, const string& arg1, uint32_t arg2) except + # default arg2=10
        # Term mkConst(Kind kind, int64_t arg)
        # Term mkConst(Kind kind, int64_t arg1, int64_t arg2)
        # Term mkConst(Kind kind, uint32_t arg1, uint32_t arg2, Term arg3) except +

        Term mkVar(const string& symbol, Sort sort) except +
        Term mkVar(Sort sort) except +
        Term mkBoundVar(const string& symbol, Sort sort) except +
        Term mkBoundVar(Sort sort) except +
        Term simplify(const Term& t) except +
        void assertFormula(Term term) except +
        Result checkSat() except +
        # TODO: verify that this works with a vector of size 1
        # Result checkSatAssuming(Term assumption) except +
        Result checkSatAssuming(const vector[Term]& assumptions) except +
        Result checkValid() except +
        # TODO: ditto above
        # Result checkValidAssuming(Term assumption) except +
        Result checkValidAssuming(const vector[Term]& assumptions) except +
        # TODO: Missing some functions
        Term declareConst(const string& symbol, Sort sort) except +
        Sort declareDatatype(const string& symbol, const vector[DatatypeConstructorDecl]& ctors)
        Term declareFun(const string& symbol, Sort sort) except +
        Term declareFun(const string& symbol, const vector[Sort]& sorts, Sort sort) except +
        # TODO: add these to cvc4.pyx
        Sort declareSort(const string& symbol, uint32_t arity) except +
        Term defineFun(const string& symbol, const vector[Term]& bound_vars,
                       Sort sort, Term term) except +
        Term defineFun(Term fun, const vector[Term]& bound_vars, Term term) except +
        Term defineFunRec(const string& symbol, const vector[Term]& bound_vars,
                          Sort sort, Term term) except +
        Term defineFunRec(Term fun, const vector[Term]& bound_vars,
                          Term term) except +
        # TODO: Decide if this is worth supporting
        Term defineFunsRec(vector[Term]& funs, vector[vector[Term]]& bound_vars,
                           vector[Term]& terms) except +
        # not supporting echo
        # TODO: Add these to cvc4.pyx
        vector[Term] getAssertions() except +
        vector[pair[Term, Term]] getAssignment() except +
        string getInfo(const string& flag) except +
        string getOption(string& option) except +
        vector[Term] getUnsatAssumptions() except +
        vector[Term] getUnsatCore() except +
        Term getValue(Term term) except +
        vector[Term] getValue(const vector[Term]& terms) except +
        void pop(uint32_t nscopes) except +
        void push(uint32_t nscopes) except +
        void reset() except +
        void resetAssertions() except +
        void setInfo(string& keyword, const string& value) except +
        void setLogic(const string& logic) except +
        void setOption(const string& option, const string& value) except +


    cdef cppclass Sort:
        Sort() except +
        bint operator==(const Sort&) except +
        bint operator!=(const Sort&) except +
        bint isBoolean() except +
        bint isInteger() except +
        bint isReal() except +
        bint isString() except +
        bint isRegExp() except +
        bint isRoundingMode() except +
        bint isBitVector() except +
        bint isFloatingPoint() except +
        bint isDatatype() except +
        bint isParametricDatatype() except +
        bint isFunction() except +
        bint isPredicate() except +
        bint isTuple() except +
        bint isRecord() except +
        bint isArray() except +
        bint isSet() except +
        bint isUninterpretedSort() except +
        bint isSortConstructor() except +
        bint isFirstClass() except +
        bint isFunctionLike() except +
        Datatype getDatatype() except +
        Sort instantiate(const vector[Sort]& params) except +
        bint isUninterpretedSortParameterized() except +
        string toString() except +

    cdef cppclass Term:
        Term()
        bint operator==(const Term&) except +
        bint operator!=(const Term&) except +
        Kind getKind() except +
        Sort getSort() except +
        bint isNull() except +
        Term notTerm() except +
        Term andTerm(const Term& t) except +
        Term orTerm(const Term& t) except +
        Term xorTerm(const Term& t) except +
        Term eqTerm(const Term& t) except +
        Term impTerm(const Term& t) except +
        Term iteTerm(const Term& then_t, const Term& else_t) except +
        string toString() except +
        cppclass const_iterator:
            const_iterator() except +
            bint operator==(const const_iterator& it) except +
            bint operator!=(const const_iterator& it) except +
            const_iterator& operator++();
            Term operator*() except +
        const_iterator begin() except +
        const_iterator end() except +


cdef extern from "cvc4cpp.h" namespace "CVC4::api::RoundingMode":
    cdef RoundingMode ROUND_NEAREST_TIES_TO_EVEN,
    cdef RoundingMode ROUND_TOWARD_POSITIVE,
    cdef RoundingMode ROUND_TOWARD_NEGATIVE,
    cdef RoundingMode ROUND_TOWARD_ZERO,
    cdef RoundingMode ROUND_NEAREST_TIES_TO_AWAY
