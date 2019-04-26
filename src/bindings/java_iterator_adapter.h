/*********************                                                        */
/*! \file java_iterator_adapter.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Morgan Deters
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief An iterator adapter for the Java bindings, giving Java iterators
 ** the ability to access elements from STL iterators.
 **
 ** An iterator adapter for the Java bindings, giving Java iterators the
 ** ability to access elements from STL iterators.  This class is mapped
 ** into Java by SWIG, where it implements Iterator (some additional
 ** Java-side functions are added by the SWIG layer to implement the full
 ** interface).
 **
 ** The functionality requires significant assistance from the ".i" SWIG
 ** interface files, applying a variety of typemaps.
 **/

// private to the bindings layer
#ifndef SWIGJAVA
#  error This should only be included from the Java bindings layer.
#endif /* SWIGJAVA */

#ifndef CVC4__BINDINGS__JAVA_ITERATOR_ADAPTER_H
#define CVC4__BINDINGS__JAVA_ITERATOR_ADAPTER_H

namespace CVC4 {

template <class T>
class JavaIteratorAdapter {
  const T& d_t;
  typename T::const_iterator d_it;

public:
  JavaIteratorAdapter(const T& t) :
    d_t(t),
    d_it(d_t.begin()) {
  }

  bool hasNext() {
    return d_it != d_t.end();
  }

  typename T::const_iterator::value_type getNext() {
    typename T::const_iterator::value_type ret = *d_it;
    ++d_it;
    return ret;
  }
};/* class JavaIteratorAdapter<T> */

}/* CVC4 namespace */

#endif /* CVC4__BINDINGS__JAVA_ITERATOR_ADAPTER_H */
