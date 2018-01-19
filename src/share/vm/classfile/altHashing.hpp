/*
 * altHashing.hpp
 *
 *  Created on: 2018年1月19日
 *      Author: limaozhi
 */

// OK

#ifndef SHARE_VM_CLASSFILE_ALTHASHING_HPP_
#define SHARE_VM_CLASSFILE_ALTHASHING_HPP_

#include "prims/jni.h"
#include "classfile/symbolTable.hpp"

/**
 * Hashing utilities.
 *
 * Implementation of Murmur3 hashing.
 * This code was translated from src/share/classes/sun/misc/Hashing.java
 * code in the JDK.
 */

class AltHashing : AllStatic {

  // utility function copied from java/lang/Integer
  static jint Integer_rotateLeft(jint i, int distance) {
    return (i << distance) | (((juint)i) >> (32-distance));
  }
  static jint murmur3_32(const int* data, int len);
  static jint murmur3_32(jint seed, const int* data, int len);

#ifndef PRODUCT
  // Hashing functions used for internal testing
  static jint murmur3_32(const jbyte* data, int len);
  static jint murmur3_32(const jchar* data, int len);
  static void testMurmur3_32_ByteArray();
  static void testEquivalentHashes();
#endif // PRODUCT

 public:
  static jint compute_seed();
  static jint murmur3_32(jint seed, const jbyte* data, int len);
  static jint murmur3_32(jint seed, const jchar* data, int len);
  NOT_PRODUCT(static void test_alt_hash();)
};

#endif /* SHARE_VM_CLASSFILE_ALTHASHING_HPP_ */
