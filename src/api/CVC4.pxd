# import dereference and increment operators
from cython.operator cimport dereference as deref, preincrement as inc
from libc.stdint cimport uint32_t, uint64_t
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from Kind cimport Kind

cdef extern from "cvc4cpp.cpp":
    pass

cdef extern from "cvc4cpp.h" namespace "CVC4":
    cdef cppclass Expr:
        pass
    cdef cppclass Datatype:
        pass
    cdef cppclass DataTypeConstructor:
        pass
    cdef cppclass ExprManager:
        pass
    cdef cppclass NodeManager:
        pass
    cdef cppclass SmtEngine:
        pass
    cdef cppclass Type:
        pass
    cdef cppclass Options:
        pass
    cdef cppclass Random:
        pass
    cdef cppclass Result:
        pass

cdef extern from "cvc4cpp.h" namespace "CVC4::api":
    cdef cppclass DatatypeDecl:
        DatatypeDecl(const string & name, bint isCoDatatype) except +
        DatatypeDecl(const string & name, Sort param, bint isCoDatatype) except +
        DatatypeDecl(const string & name, const vector[Sort]& params, bint isCoDatatype) except +
        string toString() except +
    cdef cppclass Sort:
        Sort(Type) except +
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
        bint isUninterpretedSortParameterized() except +
        string toString() except +
        Type getType() except +

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
        # TODO: determine if I should even have all of these? Easier to always use vector
        Term mkTerm(Kind kind) except +
        Term mkTerm(Kind kind, Sort sort) except +
        Term mkTerm(Kind kind, Term child) except +
        Term mkTerm(Kind kind, Term child1, Term child2) except +
        Term mkTerm(Kind kind, Term child1, Term child2, Term child3) except +
        Term mkTerm(Kind kind, const vector[Term]& children) except +
        # TODO: fill in missing mkTerm(OpTerm) functions
        Term mkTrue() except +
        Term mkFalse() except +
        Term mkBoolean(bint) except +
        # TODO: missing some here
        Term mkBitVector(uint32_t size) except +
        # Do we need this one
        Term mkBitVector(uint32_t size, uint32_t val) except +
        Term mkBitVector(uint32_t size, uint64_t val) except +
        # TODO: fill in missing functions
        Term declareConst(const string& symbol, Sort sort) except +
        # left out declareDatatype for now
        Term declareFun(const string& symbol, Sort sort) except +
        Term declareFun(const string& symbol, const vector[Sort] & sorts, Sort sort) except +
        # TODO: missing some more functions
        Term mkVar(const string & symbol, Sort sort) except +
        Term mkVar(Sort sort) except +

    cdef cppclass Term:
        Term();
        Term(const Expr&) except +
        bint operator==(const Term&) except +
        bint operator!=(const Term&) except +
        Kind getKind() except +
        Sort getSort() except +
        bint isNull() except +
        Term notTerm() except +
        Term andTerm(const Term& t) except +
        Term orTerm(const Term& t) except +
        Term xorTerm(const Term& t) except +
        Term iffTerm(const Term& t) except +
        Term impTerm(const Term& t) except +
        Term iteTerm(const Term& then_t, const Term& else_t) except +
        string toString() except +
        Expr getExpr() except +
