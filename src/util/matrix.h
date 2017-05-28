#include "cvc4_public.h"

#ifndef __CVC4__MATRIX_H
#define __CVC4__MATRIX_H

#include <iosfwd>

#include "base/exception.h"
#include "util/rational.h"

namespace CVC4 {

class CVC4_PUBLIC Matrix {
public:
  
  Matrix(unsigned rows, unsigned cols, std::vector< std::vector <Rational> > vals)
  {
    d_rows = rows;
    d_cols = cols;
    d_values = vals;
  }

 Matrix(const std::vector< std::vector <Rational> > vals)
   {
     d_rows = vals.size();
     d_cols = vals[0].size();
     d_values = vals;
   }

 //I know it's bad form to put this in the header, but don't
 // want to create too many files yet
Matrix(const std::string& mat) {
  std::string s = mat;
  std::string open = "{";
  std::string close = "}";
  std::string delimiter = ",";
  size_t index = 2;
  size_t dindex = s.find(delimiter);
  size_t cindex = s.find(close);
  std::string valstring;
  std::vector< std::vector <Rational> > ratmat;
  while(dindex != std::string::npos) {
    std::vector<Rational> v;
    while(dindex < cindex) {
      valstring = s.substr(index, dindex-index);
      v.push_back(Rational(std::atoi(valstring.c_str())));
      s = s.substr(dindex + 1, s.length() - dindex);
      index = 0;
      dindex = s.find(delimiter);
      cindex = s.find(close);
    }
      //need to grab last value
    valstring = s.substr(index, cindex-index);
    v.push_back(Rational(std::atoi(valstring.c_str())));
    ratmat.push_back(v);
    s = s.substr(dindex + 2, s.length() - dindex - 1);
    index = 0;
    dindex = s.find(delimiter);
    cindex = s.find(close);
  }
  
  d_values = ratmat;
  d_rows = ratmat.size();
  d_cols = ratmat[0].size();
 }


/* convenient methods */
 unsigned hash() const {
  // maybe not the best hash function but it works for now
   unsigned hashval = 0;
   for(unsigned r = 0; r < d_rows; ++r){
     for(unsigned c = 0; c < d_cols; ++c) {
       hashval += d_values[r][c].getNumerator().hash()
               +  d_values[r][c].getDenominator().hash();
     }
   }
   return hashval;
 }

 bool operator ==(const Matrix& y) const {
   if (d_rows != y.d_rows || d_cols != y.d_cols) return false;
   return d_values == y.d_values;
 }

 std::vector< std::vector<Rational> > getValues() const {
   return d_values;
 }
 
// Not getting syntax highlighting?
private:
  unsigned d_rows;
  unsigned d_cols;
  std::vector< std::vector <Rational> > d_values;
  
};/*class Matrix*/

/* Hash function for Matrix constants */
struct CVC4_PUBLIC MatrixHashFunction {
  inline size_t operator()(const Matrix& M) const {
    return M.hash();
  }
};/* struct MatrixHashFunction*/


/* Structure representing the matrix indexing operation */
struct CVC4_PUBLIC MatrixIndex {
  unsigned row;
  unsigned col;

  MatrixIndex(unsigned row, unsigned col)
  : row(row), col(col) {}

  bool operator == (const MatrixIndex& mindex) const {
    return row == mindex.row && col == mindex.col;
  }
};/* struct MatrixIndex */

/* Hash function for MatrixIndex objects */
 struct CVC4_PUBLIC MatrixIndexHashFunction {
   //based on BitVectorExtractHashFunction
   size_t operator()(const MatrixIndex& mindex) const {
     size_t hash = mindex.row;
     hash ^= mindex.col + 0x9e3779b9 + (hash << 6) + (hash >> 2);
     return hash;
   }
 };/* struct MatrixIndexHashFunction */

 /* struct for holding matrix dimension */
 struct CVC4_PUBLIC MatrixDim {
   unsigned rows;
   unsigned cols;
   MatrixDim(unsigned rows, unsigned cols)
   : rows(rows), cols(cols) {}
   operator std::vector<unsigned> () const {
     std::vector<unsigned> v;
     v.push_back(rows);
     v.push_back(cols);
     return v;
   }

   // build is complaining that this doesn't exist
   // I hope it isn't theory equality? What is it for?
   bool operator ==(const MatrixDim& y) const {
     return rows == y.rows && cols == y.cols;
   }
 };/* struct MatrixDim */


 /* struct for hashing Matrices based on BitVectorExtractHashFunction*/
 struct CVC4_PUBLIC MatrixDimHashFunction {
   size_t operator()(const MatrixDim& md) const {
     size_t hash = md.rows;
     hash ^= md.cols + 0x9e3779b9 + (hash << 6) + (hash >> 2);
     return hash;
   }
 };/* struct MatrixDimHashFunction */

 inline std::ostream& operator <<(std::ostream& os, const Matrix& M) CVC4_PUBLIC;
 inline std::ostream& operator <<(std::ostream& os, const Matrix& M) {
   return os << M.getValues();
 }

 inline std::ostream& operator <<(std::ostream& os, const MatrixDim& md) CVC4_PUBLIC;
 inline std::ostream& operator <<(std::ostream& os, const MatrixDim& md) {
   return os << "MatrixDim [" << md.rows << "," << md.cols << "]";
 }

 inline std::ostream& operator <<(std::ostream& os, const MatrixIndex& mi) CVC4_PUBLIC;
 inline std::ostream& operator <<(std::ostream& os, const MatrixIndex& mi) {
   return os << "MatrixIndex [" << mi.row << "," << mi.col << "]";
 }

}/* namespace CVC4 */

#endif /* __CVC4__MATRIX_H */
