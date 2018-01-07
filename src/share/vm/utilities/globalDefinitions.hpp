/*
 * globalDefinitions.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */



#ifndef SHARE_VM_UTILITIES_GLOBALDEFINITIONS_HPP_
#define SHARE_VM_UTILITIES_GLOBALDEFINITIONS_HPP_

#include "globalDefinitions_gcc.hpp" // FIXME
#include <stddef.h>

const int LogBytesPerShort = 1;
const int LogBytesPerInt   = 2;
const int LogBytesPerWord  = 2; // ILP32
const int LogBytesPerLong  = 3;

const int BytesPerShort    = 1 << LogBytesPerShort; // 1B
const int BytesPerInt      = 1 << LogBytesPerInt;   // 4B
const int BytesPerWord     = 1 << LogBytesPerWord;  // 4B
const int BytesPerLong     = 1 << LogBytesPerLong;  // 8B

const int LogBitsPerByte     = 3;
const int LogBitsPerShort    = LogBitsPerByte + LogBytesPerShort;
const int LogBitsPerInt      = LogBitsPerByte + LogBytesPerInt;
const int LogBitsPerWord     = LogBitsPerByte + LogBytesPerWord;
const int LogBitsPerLong     = LogBitsPerByte + LogBytesPerLong;

const int BitsPerByte        = 1 << LogBitsPerByte;  //  8b
const int BitsPerShort       = 1 << LogBitsPerShort; // 32b
const int BitsPerInt         = 1 << LogBitsPerInt;   // 32b
const int BitsPerWord        = 1 << LogBitsPerWord;  // 32b
const int BitsPerLong        = 1 << LogBitsPerLong;  // 64b

const int WordAlignmentMask  = (1 << LogBytesPerWord) - 1;
const int LongAlignmentMask  = (1 << LogBytesPerLong) - 1;

const int WordsPerLong       = 2;

const int oopSize            = sizeof(char*); // ILP32 => 4B
extern int heapOopSize;
const int wordSize           = sizeof(char*); // ILP32 => 4B
const int longSize           = sizeof(jlong); // 8B
const int jintSize           = sizeof(jint);  // 8B
const int size_tSize         = sizeof(size_t);// ILP32 => 4B

const int BytesPerOop        = BytesPerWord; // 4

extern int LogBytesPerHeapOop;                // Oop within a java object
extern int LogBitsPerHeapOop;
extern int BytesPerHeapOop;
extern int BitsPerHeapOop;

// Oop encoding heap max
extern uint64_t OopEncodingHeapMax;

const int BitsPerJavaInteger = 32;
const int BItsPerJavaLong    = 64;
const int BitsPerSize_t      = size_tSize * BitsPerByte;

const int jintAsStringSize   = 12; // -2147483648~2147483647

// An opaque struct of heap-word width, so that HeapWord* can be a generic
// pointer into the heap.  We require that object sizes be measured in
// units of heap words, so that that
//   HeapWord* hw;
//   hw += oop(hw)->foo();
// works, where foo is a method (like size or scavenge) that returns the
// object size.
class HeapWord {
private:
	char* i;
public:
	char* value() { return i; }
};

// Analogous opaque struct for metadata allocated from
// metaspaces.
class MetaWord {
private:
	char* i;
};

// HeapWordSize must be 2^LogHeapWordSize.
const int HeapWordSize        = sizeof(HeapWord);
const int LogHeapWordSize     = 2;

const int HeapWordsPerLong    = BytesPerLong / HeapWordSize;
const int LogHeapWordsPerLong = LogBytesPerLong - LogHeapWordSize;

// The minimum number of native machine words necessary to contain "byte_size"
// bytes.
inline size_t heap_word_size(size_t byte_size) {
  return (byte_size + (HeapWordSize-1)) >> LogHeapWordSize;
}

// VM type definitions

typedef intptr_t  intx;
typedef uintptr_t uintx;

typedef unsigned int uint;

// Java type definitions

typedef signed char   s_char;
typedef unsigned char u_char;
typedef u_char*       address;
typedef uintptr_t     address_word;

inline address set_address_bits(address x, int m)   { return address(intptr_t(x) | m); }
inline address clear_address_bits(address x, int m) { return address(intptr_t(x) & ~m); }

inline size_t pointer_delta(const void* left, const void* right, size_t element_size) {
	return ( ((uintptr_t) left) - ((uintptr_t) right) ) / element_size;
}

inline size_t pointer_delta(const HeapWord* left, const HeapWord* right) {
	return pointer_delta(left, right, sizeof(HeapWord));
}

inline size_t pointer_delta(const MetaWord* left, const MetaWord* right) {
	return pointer_delta(left, right, sizeof(HeapWord));
}


typedef jubyte   u1;
typedef jushort  u2;
typedef juint	 u4;
typedef julong	 u8;

typedef jbyte    s1;
typedef jshort   s2;
typedef jint     s4;
typedef jlong    s8;

#endif /* SHARE_VM_UTILITIES_GLOBALDEFINITIONS_HPP_ */
