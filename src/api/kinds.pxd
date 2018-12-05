from Kind cimport Kind

# TODO name the class Kind
# probably would be convenient to separate Kind class
# declaration from all of the enums (i.e. use different pxd files)
cdef class kind:
    cdef Kind k
    cdef str name
