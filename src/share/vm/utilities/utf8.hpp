/*
 * utf8.hpp
 *
 *  Created on: 2018年1月5日
 *      Author: limaozhi
 */

// OK

#ifndef SHARE_VM_UTILITIES_UTF8_HPP_
#define SHARE_VM_UTILITIES_UTF8_HPP_

#include "memory/allocation.hpp"
#include "utilities/top.hpp"

// Low level interface for UTF8 strings
/*
 * Java修改版UTF8
 * 码位数		  Unicode编码                            		UTF-8字节流                                                                            	总编码个数
 * 2^7		  000000 - 00007F     		0xxxxxxx								2^7
 * 2^11-2^7	  000080 - 0007FF     		110xxxxx 10xxxxxx						2^11
 * 2^16-2^7	  000800 - 00FFFF     		1110xxxx 10xxxxxx 10xxxxxx				2^16
 * 原UTF8:
 * 2^20		  010000 - 10FFFF     		11110xxx 10xxxxxx 10xxxxxx 10xxxxxx		2^21
 * 修改版UTF8:
 * 2^20       010000 + (00000 ~ FFFFF)	11101101 1010xxxx 10xxxxxx				2^20
 * 						          		11101101 1011xxxx 10xxxxxx
 *
 * Unicode Supplementary Characters: U+10000 ~ U+10FFFF
 */
class UTF8: AllStatic {
public:
	// returns the unicode length of a 0-terminated utf8 string
	// unicode码的个数
	static int unicode_length(const char* utf8_str);

	// returns the unicode length of a non-0-terminated utf8 string
	// unicode码的个数
	static int unicode_length(const char* utf8_str, int len);

	// converts a utf8 string to a unicode string
	// 将utf8字符串转为unicode码字符串
	static void convert_to_unicode(const char* utf8_str, jchar* unicode_buffer, int unicode_length);

	// returns the quoted ascii length of a utf8 string
	// utf8字符串中quoted ascii字符的个数
	static int quoted_ascii_length(const char* utf8_str, int utf8_length);

	// converts a utf8 string to quoted ascii
	// utf8字符串转为quoted ascii
	static void as_quoted_ascii(const char* utf8_str, int utf8_length, char* buf, int buflen);

	// converts a quoted ascii string to utf8 string.  returns the original
	// string unchanged if nothing needs to be done.
	// 将quoted ascii字符串转为utf8字符串，如果没有改变则返回原字符串
	static const char* from_quoted_ascii(const char* quoted_ascii_string);

	// decodes the current utf8 character, stores the result in value,
	// and returns the end of the current utf8 chararacter.
	// 解码当前的utf8字符，存储在value中，返回下一个utf8字符的指针
	static char* next(const char* str, jchar* value);

	// decodes the current utf8 character, gets the supplementary character instead of
	// the surrogate pair when seeing a supplementary character in string,
	// stores the result in value, and returns the end of the current utf8 chararacter.
	// 解码当前的utf8字符，
	static char* next_character(const char* str, jint* value);

	// Utility methods
	static const jbyte* strrchr(const jbyte* base, int length, jbyte c);
	static bool equal(const jbyte* base1, int length1, const jbyte* base2, int length2);
	static bool is_supplementary_character(const unsigned char* str);
	static jint get_supplementary_character(const unsigned char* str);
};

// Low-level interface for UNICODE strings

// A unicode string represents a string in the UTF-16 format in which supplementary
// characters are represented by surrogate pairs. Index values refer to char code
// units, so a supplementar character uses two positions in a unicode string.

/**
 * UTF16
 * 码位数		Unicode编码					UTF16字节流							   	总编码个数
 * 2^16		000000 - 00FFFF				yyyyyyyy xxxxxxxx						2^16
 * 2^20     010000 + (00000 ~ FFFFF)	110110yy yyyyyyyy 110111xx xxxxxxxx     2^20
 */
class UNICODE: AllStatic {
public:
	// returns the utf8 size of a unicode character
	static int utf8_size(jchar c);

	// returns the utf8 length of a unicode string
	static int utf8_length(jchar* base, int length);

	// converts a unicode string to utf8 string
	static void convert_to_utf8(const jchar* base, int length, char* utf8_buffer);

	// converts a unicode string to utf8 string; result is allocated
	// in resource area unless a buffer is provided
	static char* as_utf8(jchar* base, int length);
	static char* as_utf8(jchar* base, int length, char* buf, int buflen);

	// returns the quoted ascii length of a unicode string
	static int quoted_ascii_length(jchar* base, int length);

	// converts a utf8 string to quoted ascii
	static void as_quoted_ascii(const jchar* base, int length, char* buf, int buflen);

	// converts a utf8 string to quoted ascii
	static void as_quoted_ascii(const jchar* base, int length, char* buf, int buflen);
};

#endif /* SHARE_VM_UTILITIES_UTF8_HPP_ */
