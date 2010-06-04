/*********************                                                        */
/*! \file rational.h
 ** \verbatim
 ** Original author: taking
 ** Major contributors: cconway
 ** Minor contributors (to current version): dejan, mdeters
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Multi-precision rational constants.
 **
 ** Multi-precision rational constants.
 **/

#include "cvc4_public.h"

#ifndef __CVC4__RATIONAL_H
#define __CVC4__RATIONAL_H

#include <gmp.h>
#include <string>
#include "util/integer.h"

namespace CVC4 {

/**
 ** A multi-precision rational constant.
 ** This stores the rational as a pair of multi-precision integers,
 ** one for the numerator and one for the denominator.
 ** The number is always stored so that the gcd of the numerator and denominator
 ** is 1.  (This is referred to as referred to as canonical form in GMP's
 ** literature.) A consequence is that that the numerator and denominator may be
 ** different than the values used to construct the Rational.
 **
 ** NOTE: The correct way to create a Rational from an int is to use one of the
 ** int numerator/int denominator constructors with the denominator 1.  Trying
 ** to construct a Rational with a single int, e.g., Rational(0), will put you
 ** in danger of invoking the char* constructor, from whence you will segfault.
 **/

class CVC4_PUBLIC Rational {
private:
  /**
   * Stores the value of the rational is stored in a C++ GMP rational class.
   * Using this instead of mpq_t allows for easier destruction.
   */
  mpq_class d_value;

  /**
   * Constructs a Rational from a mpq_class object.
   * Does a deep copy.
   * Assumes that the value is in canonical form, and thus does not
   * have to call canonicalize() on the value.
   */
  Rational(const mpq_class& val) : d_value(val) {  }

public:

  /** Creates a rational from a decimal string (e.g., <code>"1.5"</code>).
   *
   * @param dec a string encoding a decimal number in the format
   * <code>[0-9]*\.[0-9]*</code>
   */
  static Rational fromDecimal(const std::string& dec);

  /** Constructs a rational with the value 0/1. */
  Rational() : d_value(0){
    d_value.canonicalize();
  }

  /**
   * Constructs a Rational from a C string in a given base (defaults to 10).
   * Throws std::invalid_argument if the string is not a valid rational.
   * For more information about what is a valid rational string,
   * see GMP's documentation for mpq_set_str().
   */
  explicit Rational(const char * s, int base = 10): d_value(s,base) {
    d_value.canonicalize();
  }
  Rational(const std::string& s, unsigned base = 10) : d_value(s, base) {
    d_value.canonicalize();
  }

  /**
   * Creates a Rational from another Rational, q, by performing a deep copy.
   */
  Rational(const Rational& q) : d_value(q.d_value) {
    d_value.canonicalize();
  }

  /**
   * Constructs a canonical Rational from a numerator.
   */
  Rational(signed int n) : d_value(n,1) {
    d_value.canonicalize();
  }
  Rational(unsigned int n) : d_value(n,1) {
    d_value.canonicalize();
  }
  Rational(signed long int n) : d_value(n,1) {
    d_value.canonicalize();
  }
  Rational(unsigned long int n) : d_value(n,1) {
    d_value.canonicalize();
  }

  /**
   * Constructs a canonical Rational from a numerator and denominator.
   */
  Rational(signed int n, signed int d) : d_value(n,d) {
    d_value.canonicalize();
  }
  Rational(unsigned int n, unsigned int d) : d_value(n,d) {
    d_value.canonicalize();
  }
  Rational(signed long int n, signed long int d) : d_value(n,d) {
    d_value.canonicalize();
  }
  Rational(unsigned long int n, unsigned long int d) : d_value(n,d) {
    d_value.canonicalize();
  }

  Rational(const Integer& n, const Integer& d) :
    d_value(n.get_mpz(), d.get_mpz())
  {
    d_value.canonicalize();
  }
  Rational(const Integer& n) :
    d_value(n.get_mpz())
  {
    d_value.canonicalize();
  }
  ~Rational() {}


  /**
   * Returns the value of numerator of the Rational.
   * Note that this makes a deep copy of the numerator.
   */
  Integer getNumerator() const {
    return Integer(d_value.get_num());
  }

  /**
   * Returns the value of denominator of the Rational.
   * Note that this makes a deep copy of the denominator.
   */
  Integer getDenominator() const{
    return Integer(d_value.get_den());
  }

  Rational inverse() const {
    return Rational(getDenominator(), getNumerator());
  }

  int cmp(const Rational& x) const {
    //Don't use mpq_class's cmp() function.
    //The name ends up conflicting with this function.
    return mpq_cmp(d_value.get_mpq_t(), x.d_value.get_mpq_t());
  }


  int sgn() {
    return mpq_sgn(d_value.get_mpq_t());
  }

  Rational& operator=(const Rational& x){
    if(this == &x) return *this;
    d_value = x.d_value;
    return *this;
  }

  Rational operator-() const{
    return Rational(-(d_value));
  }

  bool operator==(const Rational& y) const {
    return d_value == y.d_value;
  }

  bool operator!=(const Rational& y) const {
    return d_value != y.d_value;
  }

  bool operator< (const Rational& y) const {
    return d_value < y.d_value;
  }

  bool operator<=(const Rational& y) const {
    return d_value <= y.d_value;
  }

  bool operator> (const Rational& y) const {
    return d_value > y.d_value;
  }

  bool operator>=(const Rational& y) const {
    return d_value >= y.d_value;
  }


  Rational operator+(const Rational& y) const{
    return Rational( d_value + y.d_value );
  }

  Rational operator-(const Rational& y) const {
    return Rational( d_value - y.d_value );
  }

  Rational operator*(const Rational& y) const {
    return Rational( d_value * y.d_value );
  }
  Rational operator/(const Rational& y) const {
    return Rational( d_value / y.d_value );
  }

  /** Returns a string representing the rational in the given base. */
  std::string toString(int base = 10) const {
    return d_value.get_str(base);
  }

  /**
   * Computes the hash of the rational from hashes of the numerator and the
   * denominator.
   */
  size_t hash() const {
    size_t numeratorHash = gmpz_hash(d_value.get_num_mpz_t());
    size_t denominatorHash = gmpz_hash(d_value.get_den_mpz_t());

    return numeratorHash xor denominatorHash;
  }

};/* class Rational */

struct RationalHashStrategy {
  static inline size_t hash(const CVC4::Rational& r) {
    return r.hash();
  }
};/* struct RationalHashStrategy */

std::ostream& operator<<(std::ostream& os, const Rational& n);

}/* CVC4 namespace */

#endif /* __CVC4__RATIONAL_H */
