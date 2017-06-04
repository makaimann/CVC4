#include "cvc4_public.h"

#ifndef __CVC4__MATRIX_H
#define __CVC4__MATRIX_H

#include <iosfwd>

#include "base/exception.h"
#include "util/rational.h"

namespace CVC4 {

class CVC4_PUBLIC Matrix {
public:

 Matrix(const std::vector< std::vector <Rational> > vals)
   {
     d_rows = vals.size();
     d_cols = vals[0].size();
     d_values = vals;
     for(unsigned i = 0; i < vals.size(); ++i) {
       std::vector<double> dv;
       for(unsigned j = 0; j < vals[0].size(); ++j) {
         dv.push_back(((double) vals[i][j].getNumerator().getSignedInt())/vals[i][j].getDenominator().getSignedInt());
       }
       double_values.push_back(dv);
     }
   }


 Matrix(const std::vector< std::vector<double> > vals) {
   d_rows = vals.size();
   d_cols = vals[0].size();
   double_values = vals;
   for(unsigned i = 0; i < vals.size(); ++i) {
     std::vector<Rational> rv;
     for(unsigned j = 0; j < vals[0].size(); ++j) {
       rv.push_back(Rational::fromDouble(vals[i][j]));
     }
     d_values.push_back(rv);
   }
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

  // If it's a single element, set dindex
  // to the position of {{x}}
  //                       ^
  // This will then grab the one element

  if (dindex == std::string::npos) {
    dindex = s.length() - 2;
  }
  
  size_t cindex = s.find(close);
  std::string valstring;
  std::vector< std::vector <Rational> > ratmat;
  std::vector< std::vector <double> > dmat;
  while(dindex != std::string::npos) {
    std::vector<Rational> v;
    std::vector<double> dv;
    while(dindex < cindex) {
      valstring = s.substr(index, dindex-index);
      v.push_back(Rational(valstring.c_str()));
      dv.push_back(std::atof(valstring.c_str()));
      s = s.substr(dindex + 1, s.length() - dindex);
      index = 0;
      dindex = s.find(delimiter);
      cindex = s.find(close);
    }
      //need to grab last value
    valstring = s.substr(index, cindex-index);
    v.push_back(Rational(valstring.c_str()));
    dv.push_back(std::atof(valstring.c_str()));
    ratmat.push_back(v);
    dmat.push_back(dv);
    s = s.substr(dindex + 2, s.length() - dindex - 1);
    index = 0;
    dindex = s.find(delimiter);
    cindex = s.find(close);
  }
  
  d_values = ratmat;
  double_values = dmat;
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

 std::vector<unsigned> getDim() const {
   std::vector<unsigned> dimensions;
   dimensions.push_back(d_rows);
   dimensions.push_back(d_cols);
   return dimensions;
 }

 bool operator ==(const Matrix& y) const {
   if (d_rows != y.d_rows || d_cols != y.d_cols) return false;
   return d_values == y.d_values;
 }

 std::vector< std::vector<Rational> > getValues() const {
   return d_values;
 }

 std::vector< std::vector <double> > getDoubleValues() const {
   return double_values;
 }
 
// Not getting syntax highlighting?
private:
  unsigned d_rows;
  unsigned d_cols;
  std::vector< std::vector <Rational> > d_values;
  std::vector< std::vector <double> > double_values;
  
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
