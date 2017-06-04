#include "cvc4_public.h"

#ifndef __CVC4__VECTOR_H
#define __CVC4__VECTOR_H

#include <iosfwd>

#include "base/exception.h"
#include "util/rational.h"

namespace CVC4 {

  class CVC4_PUBLIC Vector {
  public:

  Vector(const std::vector<Rational> vals):
    d_length(vals.size()),
    d_values(vals)
      {
        for(unsigned i = 0; i < vals.size(); ++i) {
          double_values.push_back(((double) vals[i].getNumerator().getSignedInt())/vals[i].getDenominator().getSignedInt());
        }
      }


  Vector(const std::vector<double> vals) {
    d_length = vals.size();
    double_values = vals;
    // should this be a reference?
    for(unsigned i = 0; i < vals.size(); ++i) {
      d_values.push_back(Rational::fromDouble(vals[i]));
    }
  }

  Vector(const std::string& vec) {
    std::string s = vec;
    std::vector<Rational> ratvec;
    std::vector<double> dvec;
    std::string delimiter = ",";
    size_t index = 1;
    size_t dindex = s.find(delimiter);
    
    std::string valstring;
    while(dindex != std::string::npos) {
      valstring = s.substr(index, dindex-index);
      ratvec.push_back(Rational(valstring.c_str()));
      dvec.push_back(std::atof(valstring.c_str()));
      s = s.substr(dindex + 1, s.length() - dindex);
      index = 0;
      dindex = s.find(delimiter);
    }
    valstring = s.substr(index, s.length()-1);
    ratvec.push_back(Rational(valstring.c_str()));

    d_length = ratvec.size();
    d_values = ratvec;
    double_values = dvec;
  }

  bool operator ==(const Vector& y) const {
    if (d_length != y.d_length) return false;
    // Not sure how equality of std::vector works
    return d_values == y.d_values;
  }

  /* convenient methods */
  unsigned hash() const {
    //maybe not the best hash function but it works for now
    unsigned hashval = 0;
    for(unsigned i = 0; i < d_length; ++i) {
      hashval += d_values[i].getNumerator().hash();
      hashval += d_values[i].getDenominator().hash();
    }
    return hashval;
  }

  unsigned getLength() const {
    return d_length;
  }

  std::vector<Rational> getValues() const {
    return d_values;
  }

  std::vector<double> getDoubleValues() const {
    return double_values;
  }

  private:
    unsigned d_length;
    std::vector<Rational> d_values;
    std::vector<double> double_values;
    
  }; /* class Vector */

  
  struct CVC4_PUBLIC VectorLength {
    unsigned length;
    VectorLength(unsigned length)
      : length(length) {}
    operator unsigned () const {return length; }
  }; /* struct VectorLength */

  /* Hash function for Vector constants */
  struct CVC4_PUBLIC VectorHashFunction {
    inline size_t operator()(const Vector& V) const {
      return V.hash();
    }
  }; /* struct VectorHashFunction*/

  struct CVC4_PUBLIC VectorIndex {
    unsigned index;

    VectorIndex(unsigned index)
    : index(index) {}

    bool operator == (const VectorIndex& vindex) const {
      return index == vindex.index;
    }
  };/* struct VectorIndex */

  /* Hash function for VectorIndex class */
  struct CVC4_PUBLIC VectorIndexHashFunction {
    size_t operator()(const VectorIndex& vindex) const {
      size_t hash = vindex.index;
      // for now juse use this as the hash
      return hash;
    }
  }; /* struct VectorIndexHashFunction */

  inline std::ostream& operator <<(std::ostream& os, const Vector& v) CVC4_PUBLIC;
  inline std::ostream& operator <<(std::ostream& os, const Vector& v) {
    return os << v.getValues();
  }

  //Not sure why I have to define this here, but BitVectorSize doesn't overload <<
  inline std::ostream& operator <<(std::ostream& os, const VectorLength& vl) CVC4_PUBLIC;
  inline std::ostream& operator <<(std::ostream& os, const VectorLength& vl) {
    return os << "VectorLength [" << vl.length << "]";
  }

  inline std::ostream& operator <<(std::ostream& os, const VectorIndex& vi) CVC4_PUBLIC;
  inline std::ostream& operator <<(std::ostream& os, const VectorIndex& vi) {
    return os << "VectorIndex [" << vi.index << "]";
  }
  
} /* end namespace CVC4 */

#endif /* __CVC4__VECTOR_H */
