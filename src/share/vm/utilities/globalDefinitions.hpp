/*
 * globalDefinitions.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_UTILITIES_GLOBALDEFINITIONS_HPP_
#define SHARE_VM_UTILITIES_GLOBALDEFINITIONS_HPP_

#include "globalDefinitions_gcc.hpp"

const int WordsPerLong = 2;

const int oopSize = sizeof(char*);
extern int heapOopSize;
const int wordSize = sizeof(char*);
const int longSize = sizeof(jlong);
const int jintSize = sizeof(jint);
const int size_tSize = sizeof(size_t);

const int BitsPerJavaInteger = 32;
const int BItsPerJavaLong    = 64;
const int jintAsStringSize   = 12;

class HeapWord {
private:
	char* i;
public:
	char* value() { return i; }
};

class MetaWord {
private:
	char* i;
};

// VM type definitions

typedef intptr_t  intx;
typedef uintptr_t uintx;

typedef unsigned int uint;

typedef signed char   s_char;
typedef unsigned char u_char;
typedef u_char*       address;
typedef uintptr_t     address_word;

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef jbyte    s1;
typedef jshort   s2;
typedef jint     s4;
typedef jlong    s8;

typedef signed char   jbyte;

#endif /* SHARE_VM_UTILITIES_GLOBALDEFINITIONS_HPP_ */
