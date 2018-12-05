# distutils: language = c++
# distutils: include_dirs = ../ ../include/
# distutils: extra_compile_args = -std=c++11
# distutils: libraries = cvc4 cvc4parser

from CVC4 cimport DatatypeDecl as c_DatatypeDecl
from CVC4 cimport Solver as c_Solver
from CVC4 cimport Sort as c_Sort
from CVC4 cimport Term as c_Term

from kinds cimport kind as Kind
from kinds import kind as Kind
from Kind cimport Kind as c_Kind

from cython.operator cimport dereference as dref

from libc.stdint cimport uint32_t, uint64_t

from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector

from collections import Sequence


## TODO: Add lots of assertions
##       Add type declarations where possible

################# !!!!!IMPORTANT!!!! ###############
# FIXME
# TODO: Fix all the Term/Sort constructions
#       using the sort.s = &s   approach invokes a move/copy constructor that doesn't copy all the data correctly!
#
# For now, leaving most things as are in case Aina/Mathias have a better way to do it
# Just going to get the term building to work

########################### Convenient Decorators #################################

def check_for_null(f):
    def run_if_not_null(self, *args):
        if self.__isnull__():
            raise RuntimeError("Tried to call a method of a NULL sort")
        else:
            return f(self, *args)
    return run_if_not_null

cdef class Sort:
    cdef c_Sort* s
    def __cinit__(self):
        self.s = NULL

    def __eq__(self, Sort other):
        if self.__isnull__() and other.__isnull__():
            return True
        elif self.__isnull__() or other.__isnull__():
            return False
        else:
            return self.s[0] == other.s[0]

    def __ne__(self, Sort other):
        # TODO: Decide on semantics -- might be better to return True on null
        return not self.__eq__(other)

    def __isnull__(self):
        return self.s == NULL

    def __str__(self):
        return self.s.toString().decode()

    def __repr__(self):
        return self.s.toString().decode()

    @check_for_null
    def isBoolean(self):
        return self.s.isBoolean()

    @check_for_null
    def isInteger(self):
        return self.s.isInteger()

    @check_for_null
    def isReal(self):
        return self.s.isReal()

    @check_for_null
    def isString(self):
        return self.s.isString()

    @check_for_null
    def isRegExp(self):
        return self.s.isRegExp()

    @check_for_null
    def isRoundingMode(self):
        return self.s.isRoundingMode()

    @check_for_null
    def isBitVector(self):
        return self.s.isBitVector()

    @check_for_null
    def isFloatingPoint(self):
        return self.s.isFloatingPoint()

    @check_for_null
    def isDatatype(self):
        return self.s.isDatatype()

    @check_for_null
    def isParametricDatatype(self):
        return self.s.isParametricDatatype()

    @check_for_null
    def isFunction(self):
        return self.s.isFunction()

    @check_for_null
    def isPredicate(self):
        return self.s.isPredicate()

    @check_for_null
    def isTuple(self):
        return self.s.isTuple()

    @check_for_null
    def isRecord(self):
        return self.s.isRecord()

    @check_for_null
    def isArray(self):
        return self.s.isArray()

    @check_for_null
    def isSet(self):
        return self.s.isSet()

    @check_for_null
    def isUninterpretedSort(self):
        return self.s.isUninterpretedSort()

    @check_for_null
    def isSortConstructor(self):
        return self.s.isSortConstructor()

    @check_for_null
    def isFirstClass(self):
        return self.s.isFirstClass()

    @check_for_null
    def isFunctionLike(self):
        return self.s.isFunctionLike()

    @check_for_null
    def isUninterpretedSortParameterized(self):
        return self.s.isUninterpretedSortParameterized()


cdef class DatatypeDecl:
    cdef c_DatatypeDecl* d
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
            self.d = new c_DatatypeDecl(<const string &> name, <bint> isCoDatatype)
        elif isinstance(sorts_or_bool, Sort):
            self.d = new c_DatatypeDecl(<const string &> name, (<Sort?> sorts_or_bool).s[0], <bint> isCoDatatype)
        elif isinstance(sorts_or_bool, list):
            for s in sorts_or_bool:
                v.push_back((<Sort?> s).s[0])
            self.d = new c_DatatypeDecl(<const string &> name, <const vector[c_Sort]&> v, <bint> isCoDatatype)
        else:
            raise RuntimeError("Unhandled input types {}".format(list(map(type, (name, sorts_or_bool, isCoDatatype)))))

    def __str__(self):
        return self.d.toString().decode()

    def __repr__(self):
        return self.d.toString().decode()


cdef class Solver:
    cdef c_Solver* s

    def __cinit__(self):
        # TODO: pass options through
        self.s = new c_Solver(NULL)

    # TODO: Go through the functions below and update the sort construction

    def getBooleanSort(self):
        cdef Sort sort = Sort()

        # TODO: Determine if this is the best way
        # Other attempts at solving this problem
        # all still needed a nullary constructor because of
        # the generated c++ which had a Sort <varname>; declaration

        # got a segfault on some of these options even with
        # the nullary constructor
        # probably because I didn't define a move/copy constructor
        # so it tries to dereference at some point
        # s = new Sort(self.s.getBooleanSort().getType())
        # val.s[0] = self.s.getBooleanSort()
        # val.s = new Sort(dref(self.s).getBooleanSort().getType())

        sort.s = new c_Sort(self.s.getBooleanSort().getType())
        return sort

    def getIntegerSort(self):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.getIntegerSort().getType())
        return sort

    def getRealSort(self):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.getRealSort().getType())
        return sort

    def getRegExpSort(self):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.getRegExpSort().getType())
        return sort

    def getRoundingmodeSort(self):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.getRoundingmodeSort().getType())
        return sort

    def getStringSort(self):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.getStringSort().getType())
        return sort

    def mkArraySort(self, Sort indexSort, Sort elemSort):
        cdef Sort sort = Sort()
        # TODO: Decide if there should be a null-check on inputs...
        sort.s = new c_Sort(self.s.mkArraySort(indexSort.s[0], elemSort.s[0]).getType())
        return sort

    def mkBitVectorSort(self, uint32_t size):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.mkBitVectorSort(size).getType())
        return sort

    def mkFloatingPointSort(self, uint32_t exp, uint32_t sig):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.mkFloatingPointSort(exp, sig).getType())
        return sort

    # FIXME: Complains about expecting at least one constructor?
    def mkDatatypeSort(self, DatatypeDecl dtypedecl):
        cdef Sort sort = Sort()
        sort.s = new c_Sort(self.s.mkDatatypeSort(dtypedecl.d[0]).getType())
        return sort

    def mkFunctionSort(self, sorts, Sort codomain):

        cdef Sort sort = Sort()
        # populate a vector with dereferenced c_Sorts
        cdef vector[c_Sort] v

        if isinstance(sorts, Sort):
            sort.s = new c_Sort(self.s.mkFunctionSort((<Sort?> sorts).s[0], codomain.s[0]).getType())
        elif isinstance(sorts, list):
            for s in sorts:
                v.push_back((<Sort?>s).s[0])

            sort.s = new c_Sort(self.s.mkFunctionSort(<const vector[c_Sort]&> v, \
                                                      codomain.s[0]).getType())

        return sort

    def mkParamSort(self, symbolname):
        cdef Sort sort = Sort()
        cdef c_Sort csort

        # encode string as bytes
        symbolname = symbolname.encode()

        csort = self.s.mkParamSort(symbolname)
        sort.s = &csort
        return sort

    def mkPredicateSort(self, sorts):

        cdef Sort sort = Sort()
        cdef c_Sort csort

        cdef vector[c_Sort] v

        for s in sorts:
            v.push_back((<Sort?> s).s[0])

        csort = self.s.mkPredicateSort(<const vector[c_Sort]&> v)
        sort.s = &csort

        return sort

    def mkRecordSort(self, fields):
        cdef Sort sort = Sort()
        cdef c_Sort csort

        cdef vector[pair[string, c_Sort]] v
        cdef pair[string, c_Sort] p

        for f in fields:
            name, sortarg = f
            name = name.encode()
            p = pair[string, c_Sort](<string?> name, (<Sort?> sortarg).s[0])
            v.push_back(p)

        csort = self.s.mkRecordSort(<const vector[pair[string, c_Sort]] &> v)
        sort.s = &csort
        return sort

    def mkSetSort(self, Sort elemSort):
        cdef Sort sort = Sort()
        cdef c_Sort csort

        csort = self.s.mkSetSort(elemSort.s[0])
        sort.s = &csort

        return sort

    def mkUninterpretedSort(self, str name):
        cdef Sort sort = Sort()
        cdef c_Sort csort

        csort = self.s.mkUninterpretedSort(name.encode())
        sort.s = &csort

        return sort

    def mkSortConstructorSort(self, str symbol, size_t arity):
        cdef Sort sort = Sort()
        cdef c_Sort csort

        csort = self.s.mkSortConstructorSort(symbol.encode(), arity)
        sort.s = &csort

        return sort

    def mkTupleSort(self, sorts):
        cdef Sort sort = Sort()
        cdef c_Sort csort

        cdef vector[c_Sort] v
        for s in sorts:
            v.push_back((<Sort?> s).s[0])

        csort = self.s.mkTupleSort(v)
        sort.s = &csort

        return sort

    def mkTerm(self, Kind kind, *args):
        cdef c_Term t
        cdef Term term = Term()

        if len(args) == 1 and isinstance(args[0], Sort):
            t = self.s.mkTerm(kind.k, (<Sort?> args[0]).s[0])
            term.t = &t
            return term

        if isinstance(args[0], Sequence):
            if not len(args) == 1:
                raise RuntimeError("Expecting one sequence or multiple arguments, got {}".format(args))

            args = args[0]

        cdef vector[c_Term] v
        for a in args:
            v.push_back((<Term> a).t[0])

        t = self.s.mkTerm(kind.k, <const vector[c_Term]&> v)
        term.t = &t

        return term

    def mkTrue(self):
        cdef c_Term t
        cdef Term term = Term()

        t = self.s.mkTrue()
        term.t = &t
        return term

    def mkFalse(self):
        cdef c_Term t
        cdef Term term = Term()

        t = self.s.mkFalse()
        term.t = &t
        return term

    def mkBoolean(self, bint val):
        cdef c_Term t
        cdef Term term = Term()

        t = self.s.mkBoolean(val)
        term.t = &t
        return term

    # TODO: Fill in missing functions

    def mkBitVector(self, int size, val = None):
        cdef c_Term t
        cdef Term term = Term()

        if val is None:
            t = self.s.mkBitVector(size)
        elif isinstance(val, int):
            # TODO: Figure out which one to use?
            # uint32_t or uint64_t
            t = self.s.mkBitVector(size, <uint64_t> val)
        else:
            raise RuntimeError("Unsupported value type: {}".format(type(val)))

        term.t = &t
        return term

    # TODO: Fill in a bunch of missing functions

    def declareConst(self, str symbol, Sort sort):
        cdef Term term = Term()

        term.t = new c_Term(self.s.declareConst(symbol.encode(), sort.s[0]).getExpr())

        return term

    def declareFun(self, str symbol, sorts, Sort sort):
        cdef Term term = Term()

        cdef vector[c_Sort] v
        for s in sorts:
            v.push_back((<Sort?> s).s[0])

        term.t = new c_Term(self.s.declareFun(symbol.encode(), <const vector[c_Sort]&> v, sort.s[0]).getExpr())

        return term

    def mkVar(self, str symbol, Sort sort):
        cdef Term term = Term()
        term.t = new c_Term(self.s.mkVar(symbol.encode(), sort.s[0]).getExpr())

        return term


cdef class Term:
    cdef c_Term* t
    # TODO: Decide if we should support the Term(Expr) constructor
    #       might not, because should never construct it directly
    def __cinit__(self):
        self.t = NULL

    def __eq__(self, Term other):
        if self.__isnull__() and other.__isnull__():
            return True
        elif self.__isnull__() or other.__isnull__():
            return False
        else:
            return self.t[0] == other.t[0]

    def __ne__(self, Term other):
        # TODO: Think about desired semantics -- might be better to return True
        #       when there's a NULL (or just error out)
        return not self.__eq__(other)

    def __isnull__(self):
        return self.t == NULL

    def __str__(self):
        return self.t.toString().decode()

    def __repr__(self):
        return self.t.toString().decode()
