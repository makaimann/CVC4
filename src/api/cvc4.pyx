# distutils: language = c++
# distutils: include_dirs = ../ ../include/
# distutils: extra_compile_args = -std=c++11
# distutils: libraries = cvc4 cvc4parser

from collections import Sequence

from libc.stdint cimport int32_t, int64_t, uint32_t, uint64_t

from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector

from CVC4 cimport DatatypeConstructorDecl as c_DatatypeConstructorDecl
from CVC4 cimport DatatypeDecl as c_DatatypeDecl
from CVC4 cimport DatatypeDeclSelfSort as c_DatatypeDeclSelfSort
from CVC4 cimport DatatypeSelectorDecl as c_DatatypeSelectorDecl
from CVC4 cimport Solver as c_Solver
from CVC4 cimport Sort as c_Sort
from CVC4 cimport Result as c_Result
from CVC4 cimport Term as c_Term

from kinds cimport Kind as c_Kind
from kinds cimport kind
from kinds import kind


# TODO: match order of classes to CVC4.pxd (i.e. alphabetical)

## TODO: Add lots of assertions
##       Add type declarations where possible

## TODO: Use PEP-8 spacing recommendations
# Limit linewidth to 79 characters
# Break before binary operators
# surround top level functions and classes with two spaces
# separate methods by one space
# use spaces in functions sparingly to separate logical blocks
# can omit spaces between unrelated oneliners


cdef class DatatypeConstructorDecl:
    cdef c_DatatypeConstructorDecl* cddc
    def __cinit__(self, str name):
        self.cddc = new c_DatatypeConstructorDecl(name.encode())

    def addSelector(self, DatatypeSelectorDecl stor):
        self.cddc.addSelector(stor.cdsd[0])

    def __str__(self):
        return self.cddc.toString().decode()

    def __repr__(self):
        return self.cddc.toString().decode()


cdef class DatatypeDecl:
    cdef c_DatatypeDecl* cdd
    def __cinit__(self, name, sorts_or_bool=None, isCoDatatype=False):

        # TODO: Decide if this is worth it
        ###### supporting three overloaded constructors at Python level #####
        # DatatypeDecl(const string &, bool)
        # DatatypeDecl(const string &, Sort, bool)
        # DatatypeDecl(const string &, const vector[Sort]&, bool)

        # TODO: Verify that this won't cause memory leaks
        # used if sorts_or_bool is supplied as a list of Sorts
        cdef vector[c_Sort] v

        # encode string as bytes
        name = name.encode()

        if isinstance(sorts_or_bool, bool):
            isCoDatatype = sorts_or_bool
            sorts_or_bool = None

        if sorts_or_bool is None:
            self.cdd = new c_DatatypeDecl(<const string &> name, <bint> isCoDatatype)
        elif isinstance(sorts_or_bool, Sort):
            self.cdd = new c_DatatypeDecl(<const string &> name, (<Sort?> sorts_or_bool).csort, <bint> isCoDatatype)
        elif isinstance(sorts_or_bool, list):
            for s in sorts_or_bool:
                v.push_back((<Sort?> s).csort)
            self.cdd = new c_DatatypeDecl(<const string &> name, <const vector[c_Sort]&> v, <bint> isCoDatatype)
        else:
            raise RuntimeError("Unhandled input types {}".format(list(map(type, (name, sorts_or_bool, isCoDatatype)))))

    def addConstructor(self, DatatypeConstructorDecl ctor):
        self.cdd.addConstructor(ctor.cddc[0])

    def isParametric(self):
        return self.cdd.isParametric()

    def __str__(self):
        return self.cdd.toString().decode()

    def __repr__(self):
        return self.cdd.toString().decode()


cdef class DatatypeDeclSelfSort:
    cdef c_DatatypeDeclSelfSort cddss
    def __cinit__(self):
        self.cddss = c_DatatypeDeclSelfSort()


cdef class DatatypeSelectorDecl:
    cdef c_DatatypeSelectorDecl* cdsd
    def __cinit__(self, str name, sort):
        if isinstance(sort, Sort):
            self.cdsd = new c_DatatypeSelectorDecl(<const string &> name.encode(), (<Sort?> sort).csort)
        elif isinstance(sort, DatatypeDeclSelfSort):
            self.cdsd = new c_DatatypeSelectorDecl(<const string &> name.encode(), (<DatatypeDeclSelfSort?> sort).cddss)

    def __str__(self):
        return self.cdsd.toString().decode()

    def __repr__(self):
        return self.cdsd.toString().decode()


cdef class Solver:
    cdef c_Solver* csolver

    def __cinit__(self):
        # TODO: pass options through
        self.csolver = new c_Solver(NULL)

    # TODO: Go through the functions below and update the sort construction

    def getBooleanSort(self):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.getBooleanSort()
        return sort

    def getIntegerSort(self):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.getIntegerSort()
        return sort

    def getRealSort(self):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.getRealSort()
        return sort

    def getRegExpSort(self):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.getRegExpSort()
        return sort

    def getRoundingmodeSort(self):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.getRoundingmodeSort()
        return sort

    def getStringSort(self):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.getStringSort()
        return sort

    def mkArraySort(self, Sort indexSort, Sort elemSort):
        cdef Sort sort = Sort()
        # TODO: Decide if there should be a null-check on inputs...
        sort.csort = self.csolver.mkArraySort(indexSort.csort, elemSort.csort)
        return sort

    def mkBitVectorSort(self, uint32_t size):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.mkBitVectorSort(size)
        return sort

    def mkFloatingPointSort(self, uint32_t exp, uint32_t sig):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.mkFloatingPointSort(exp, sig)
        return sort

    # FIXME: Complains about expecting at least one constructor?
    def mkDatatypeSort(self, DatatypeDecl dtypedecl):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.mkDatatypeSort(dtypedecl.cdd[0])
        return sort

    def mkFunctionSort(self, sorts, Sort codomain):

        cdef Sort sort = Sort()
        # populate a vector with dereferenced c_Sorts
        cdef vector[c_Sort] v

        if isinstance(sorts, Sort):
            sort.csort = self.csolver.mkFunctionSort((<Sort?> sorts).csort, codomain.csort)
        elif isinstance(sorts, list):
            for s in sorts:
                v.push_back((<Sort?>s).csort)

            sort.csort = self.csolver.mkFunctionSort(<const vector[c_Sort]&> v,
                                                      codomain.csort)
        return sort

    def mkParamSort(self, symbolname):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.mkParamSort(symbolname.encode())
        return sort

    def mkPredicateSort(self, sorts):
        cdef Sort sort = Sort()
        cdef vector[c_Sort] v
        for s in sorts:
            v.push_back((<Sort?> s).csort)
        sort.csort = self.csolver.mkPredicateSort(<const vector[c_Sort]&> v)
        return sort

    def mkRecordSort(self, fields):
        cdef Sort sort = Sort()
        cdef vector[pair[string, c_Sort]] v
        cdef pair[string, c_Sort] p
        for f in fields:
            name, sortarg = f
            name = name.encode()
            p = pair[string, c_Sort](<string?> name, (<Sort?> sortarg).csort)
            v.push_back(p)
        sort.csort = self.csolver.mkRecordSort(<const vector[pair[string, c_Sort]] &> v)
        return sort

    def mkSetSort(self, Sort elemSort):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.mkSetSort(elemSort.csort)
        return sort

    def mkUninterpretedSort(self, str name):
        cdef Sort sort = Sort()
        sort.csort = self.csolver.mkUninterpretedSort(name.encode())
        return sort

    def mkSortConstructorSort(self, str symbol, size_t arity):
        cdef Sort sort = Sort()
        sort.csort =self.csolver.mkSortConstructorSort(symbol.encode(), arity)
        return sort

    def mkTupleSort(self, sorts):
        cdef Sort sort = Sort()
        cdef vector[c_Sort] v
        for s in sorts:
            v.push_back((<Sort?> s).csort)
        sort.csort = self.csolver.mkTupleSort(v)
        return sort

    def mkTerm(self, kind k, *args):
        cdef Term term = Term()

        if len(args) == 1 and isinstance(args[0], Sort):
            term.cterm = self.csolver.mkTerm(k.k, (<Sort?> args[0]).csort)
            return term

        if isinstance(args[0], Sequence):
            if not len(args) == 1:
                raise RuntimeError("Expecting one sequence or multiple arguments, got {}".format(args))

            args = args[0]

        cdef vector[c_Term] v
        for a in args:
            v.push_back((<Term> a).cterm)

        term.cterm = self.csolver.mkTerm(k.k, <const vector[c_Term]&> v)

        return term

    def mkTrue(self):
        cdef Term term = Term()
        term.cterm = self.csolver.mkTrue()
        return term

    def mkFalse(self):
        cdef Term term = Term()
        term.cterm = self.csolver.mkFalse()
        return term

    def mkBoolean(self, bint val):
        cdef Term term = Term()
        term.cterm = self.csolver.mkBoolean(val)
        return term

    def mkPi(self):
        cdef Term term = Term()
        term.cterm = self.csolver.mkPi()
        return term

    def mkReal(self, val, den=None):
        # FIXME: the second argument (base) which I'm implicitly assuming to be 10
        #        here will not be supported going forward -- make this change
        cdef Term term = Term()
        if den is None:
            try:
                term.cterm = self.csolver.mkReal(str(val).encode(), 10)
            except ValueError as e:
                raise ValueError("Expecting a number or a string representing a number but got: {}".format(val))
        else:
            if not isinstance(val, int) or not isinstance(den, int):
                raise RuntimeError("Expecting integers when constructing a rational but got: {}".format((val, den)))
            term.cterm = self.csolver.mkReal("{}/{}".format(val, den).encode(), 10)
        return term

    # TODO: Look into this error
    #       Commented out in unit tests -- pretty sure it's not my fault
    def mkRegexpEmpty(self):
        cdef Term term = Term()
        term.cterm = self.csolver.mkRegexpEmpty()
        return term

    def mkRegexpSigma(self):
        cdef Term term = Term()
        term.cterm = self.csolver.mkRegexpSigma()
        return term

    def mkEmptySet(self, Sort s):
        cdef Term term = Term()
        term.cterm = self.csolver.mkEmptySet(s.csort)
        return term

    def mkSepNil(self, Sort sort):
        cdef Term term = Term()
        term.cterm = self.csolver.mkSepNil(sort.csort)
        return term

    def mkString(self, str_or_vec):
        cdef Term term = Term()
        cdef vector[unsigned] v
        if isinstance(str_or_vec, str):
            term.cterm = self.csolver.mkString(<string &> str_or_vec.encode())
        elif isinstance(str_or_vec, Sequence):
            for u in str_or_vec:
                if not isinstance(u, int):
                    raise ValueError("List should contain ints but got: {}".format(str_or_vec))
                v.push_back(<unsigned> u)
            term.cterm = self.csolver.mkString(<const vector[unsigned]&> v)
        else:
            raise ValueError("Expected string or vector of ASCII codes but got: {}".format(str_or_vec))
        return term

    def mkUniverseSet(self, Sort sort):
        cdef Term term = Term()
        term.cterm = self.csolver.mkUniverseSet(sort.csort)
        return term

    def mkBitVector(self, int size, val = None):
        cdef Term term = Term()
        if val is None:
            term.cterm = self.csolver.mkBitVector(size)
        elif isinstance(val, int):
            term.cterm = self.csolver.mkBitVector(size, val)
        else:
            raise RuntimeError("Unsupported value type: {}".format(type(val)))
        return term

    # TODO: Fill in a bunch of missing functions -- skipped mkConst (lots of overloaded versions)

    def mkVar(self, symbol_or_sort, sort=None):
        cdef Term term = Term()
        if sort is None:
            term.cterm = self.csolver.mkVar((<Sort?> symbol_or_sort).csort)
        else:
            term.cterm = self.csolver.mkVar((<str?> symbol_or_sort).encode(), (<Sort?> sort).csort)
        return term

    def mkBoundVar(self, symbol_or_sort, sort=None):
        cdef Term term = Term()
        if sort is None:
            term.cterm = self.csolver.mkBoundVar((<Sort?> symbol_or_sort).csort)
        else:
            term.cterm = self.csolver.mkBoundVar((<str?> symbol_or_sort).encode(), (<Sort?> sort).csort)
        return term

    def simplify(self, Term t):
        cdef Term term = Term()
        term.cterm = self.csolver.simplify(t.cterm)
        return term

    def assertFormula(self, Term term):
        self.csolver.assertFormula(term.cterm)

    def checkSat(self):
        cdef c_Result r = self.csolver.checkSat()
        name = r.toString().decode()
        explanation = ""
        if r.isSatUnknown():
            explanation = r.getUnknownExplanation().decode()
        return Result(name, explanation)

    def checkSatAssuming(self, assumptions):
        cdef c_Result r
        # used if assumptions is a list of terms
        cdef vector[c_Term] v
        if isinstance(assumptions, Term):
            r = self.csolver.checkSatAssuming((<Term?> assumptions).cterm)
        elif isinstance(assumptions, Sequence):
            for a in assumptions:
                v.push_back((<Term?> a).cterm)
            r = self.csolver.checkSatAssuming(<const vector[c_Term]&> v)
        else:
            raise RuntimeError("Expecting a list of Terms")

        name = r.toString().decode()
        explanation = ""
        if r.isSatUnknown():
            explanation = r.getUnknownExplanation().decode()
        return  Result(name, explanation)

    def checkValid(self):
        cdef c_Result r = self.csolver.checkValid()
        name = r.toString().decode()
        explanation = ""
        if r.isValidUnknown():
            explanation = r.getUnknownExplanation().decode()
        return Result(name, explanation)

    def checkValidAssuming(self, assumptions):
        cdef c_Result r
        # used if assumptions is a list of terms
        cdef vector[c_Term] v
        if isinstance(assumptions, Term):
            r = self.csolver.checkValidAssuming((<Term?> assumptions).cterm)
        elif isinstance(assumptions, Sequence):
            for a in assumptions:
                v.push_back((<Term?> a).cterm)
            r = self.csolver.checkValidAssuming(<const vector[c_Term]&> v)
        else:
            raise RuntimeError("Expecting a list of Terms")

        name = r.toString().decode()
        explanation = ""
        if r.isValidUnknown():
            explanation = r.getUnknownExplanation().decode()
        return Result(name, explanation)

    def declareConst(self, str symbol, Sort sort):
        cdef Term term = Term()
        term.cterm = self.csolver.declareConst(symbol.encode(), sort.csort)
        return term

    def declareFun(self, str symbol, sorts, Sort sort):
        cdef Term term = Term()
        cdef vector[c_Sort] v
        for s in sorts:
            v.push_back((<Sort?> s).csort)
        term.cterm = self.csolver.declareFun(symbol.encode(), <const vector[c_Sort]&> v, sort.csort)
        return term


cdef class Sort:
    cdef c_Sort csort
    def __cinit__(self):
        # csort always set by Solver
        pass

    def __eq__(self, Sort other):
        return self.csort == other.csort

    def __ne__(self, Sort other):
        return self.csort != other.csort

    def __str__(self):
        return self.csort.toString().decode()

    def __repr__(self):
        return self.csort.toString().decode()

    def isBoolean(self):
        return self.csort.isBoolean()

    def isInteger(self):
        return self.csort.isInteger()

    def isReal(self):
        return self.csort.isReal()

    def isString(self):
        return self.csort.isString()

    def isRegExp(self):
        return self.csort.isRegExp()

    def isRoundingMode(self):
        return self.csort.isRoundingMode()

    def isBitVector(self):
        return self.csort.isBitVector()

    def isFloatingPoint(self):
        return self.csort.isFloatingPoint()

    def isDatatype(self):
        return self.csort.isDatatype()

    def isParametricDatatype(self):
        return self.csort.isParametricDatatype()

    def isFunction(self):
        return self.csort.isFunction()

    def isPredicate(self):
        return self.csort.isPredicate()

    def isTuple(self):
        return self.csort.isTuple()

    def isRecord(self):
        return self.csort.isRecord()

    def isArray(self):
        return self.csort.isArray()

    def isSet(self):
        return self.csort.isSet()

    def isUninterpretedSort(self):
        return self.csort.isUninterpretedSort()

    def isSortConstructor(self):
        return self.csort.isSortConstructor()

    def isFirstClass(self):
        return self.csort.isFirstClass()

    def isFunctionLike(self):
        return self.csort.isFunctionLike()

    def isUninterpretedSortParameterized(self):
        return self.csort.isUninterpretedSortParameterized()


class Result:
    _name = None
    _explanation = None

    def __init__(self, name, explanation=""):
        assert name in {"sat", "unsat", "valid", "invalid", "unknown"}
        self._name = name
        self._explanation = explanation

    def __bool__(self):
        if self._name in {"sat", "valid"}:
            return True
        elif self._name in {"unsat", "invalid"}:
            return False
        elif self._name == "unknown":
            raise RuntimeError("Cannot interpret 'unknown' result as a Boolean")
        else:
            assert False, "Unhandled result=%s"%self._name

    def __eq__(self, other):
        if not isinstance(other, Result):
            return False

        return self._name == other._name

    def __ne__(self, other):
        if not isinstance(other, Result):
            return True

        return self._name != other._name

    def __str__(self):
        return self._name

    def __repr__(self):
        return self._name

    def isUnknown(self):
        return self._name == "unknown"

    @property
    def explanation(self):
        return self._explanation


cdef class Term:
    cdef c_Term cterm
    def __cinit__(self):
        # cterm always set in the Solver object
        pass

    def __eq__(self, Term other):
        return self.cterm == other.cterm

    def __ne__(self, Term other):
        return self.cterm != other.cterm

    def __str__(self):
        return self.cterm.toString().decode()

    def __repr__(self):
        return self.cterm.toString().decode()
