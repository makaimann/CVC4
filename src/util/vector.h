#include "cvc4_public.h"

#include <iosfwd>

#include "base/exception.h"

#include "util/rational.h"

namespace CVC4 {

  class CVC4_PUBLIC Vector {
  public:

  Vector(const std::vector<Rational> vals):
    d_length(vals.size()),
    d_values(vals)
      {}


    Vector(const std::vector<double> vals) {
      unsigned d_length = vals.size();
      // should this be a reference?
      vector<Rational> d_values;
      for(int i = 0; i < vals.size(); ++i) {
        d_values[i] = Rational(vals[i])
      }
    }

  private:
    unsigned d_length;
    vector<Rational> d_values;
    
  }; /* class Vector */

  
  struct CVC4_PUBLIC VectorLength {
    unsigned length;
    VectorLength(unsigned length)
      : length(length) {}
    operator unsigned () const {return length; }
  }; /* struct VectorLength */

  template <typename T>
  struct CVC4_PUBLIC UnsignedHashFunction {
    inline size_t operator()(const T& x) const {
      return (size_t) x;
    }
  };

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
  
} /* end namespace CVC4 */
