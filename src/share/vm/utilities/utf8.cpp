/*
 * utf8.cpp
 *
 *  Created on: 2018年1月5日
 *      Author: limaozhi
 */

// OK

#include "precompiled.hpp"
#include "utilities/utf8.hpp"

/*
 * 从str读取一个UTF8字符，转化为unicode码存于value，返回str下一个UTF8字符地址
 * str: UTF8字符串
 * 只处理下列范围
 * 000000~00007F: 0xxxxxxx
 * 000080~0007FF: 110xxxxx 10xxxxxx
 * 000800~00FFFF: 1110xxxx 10xxxxxx 10xxxxxx
 */
char* UTF8::next(const char* str, jchar* value) {
	unsigned const char *ptr = (const unsigned char *) str;
	unsigned char ch, ch2, ch3;
	int length = -1; /* bad length */
	jchar result;

	switch ((ch = ptr[0]) >> 4) { // 根据第一字节判断unicode码范围
	default: // 000000~00007F: 0xxxxxxx
		result = ch;
		length = 1;
		break;

	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xF:
		/* Shouldn't happen. */
		break;

	case 0xC:
	case 0xD: // 000080~0007FF: 110xxxxx 10xxxxxx
		if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
			unsigned char high_five = ch & 0x1F;
			unsigned char low_six = ch2 & 0x3F;
			result = (high_five << 6) + low_six;
			length = 2;
			break;
		}
		break;

	case 0xE: // 000800~00FFFF: 1110xxxx 10xxxxxx 10xxxxxx
		if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
			if (((ch3 = ptr[2]) & 0xC0) == 0x80) {
				unsigned char high_four = ch & 0x0f;
				unsigned char mid_six = ch2 & 0x3f;
				unsigned char low_six = ch3 & 0x3f;
				result = (((high_four << 6) + mid_six) << 6) + low_six;
				length = 3;
			}
		}
		break;
	} /* end of switch */

	if (length <= 0) {
		*value = ptr[0]; /* default bad result; */
		return (char*) (ptr + 1); // make progress somehow
	}

	*value = result;

	// The assert is correct but the .class file is wrong
	// assert(UNICODE::utf8_size(result) == length, "checking reverse computation");
	return (char *) (ptr + length);
}

/**
 * 从str读取一个UTF8字符，转化为unicode码存于value，返回str下一个UTF8字符地址
 * str: 修改版UTF8字符串
 */
char* UTF8::next_character(const char* str, jint* value) {
	unsigned const char* ptr = (const unsigned char*) str;
	// 010000~10FFFF: 010000 + (00000 ~ FFFFF)
	if (is_supplementary_character(ptr)) {
		*value = get_supplementary_character(ptr);
		return (char*)(ptr + 6);
	}
	// 000000~00FFFF
	jchar result;
	char* next_ch = next(str, &result);
	*value = result;
	return next_ch;
}

/**
 * 返回unicode码个数
 * str：标准UTF8字符串
 */
int UTF8::unicode_length(const char* str, int len) {
	int num_chars = len;
	for (int i = 0; i < len; i++) {
		// 刨去UTF8字符的第2/3/4字节计数
		if ((str[i] & 0xC0) == 0x80) {
			--num_chars;
		}
	}
	return num_chars;
}

/**
 * 返回unicode码个数
 * str：标准UTF8字符串
 */
int UTF8::unicode_length(const char* str) {
	int num_chars = 0;
	for (const char* p = str; *p; p++) {
		// UTF8字符的第2/3/4字节为10xxxxxx，不能计为字符数
		if (((*p) & 0xC0) != 0x80) {
			num_chars++;
		}
	}
	return num_chars;
}

/**
 * 将unicode码ch转为utf8字符写入到base，并返回base下个写入地址
 */
static u_char* utf8_write(u_char* base, jchar ch) {
	if ( (ch != 0) && (ch <= 0x7f) ) { // 000000 - 00007F     0xxxxxxx
		base[0] = (u_char) ch;
		return base + 1;
	}
	if (ch <= 0x7FF) { // 000080 - 0007FF     110xxxxx 10xxxxxx
		base[0] = 0xC0 | (ch >> 6);
		base[1] = 0x80 | (ch & 0x3F);
		return base + 2;
	}
	// 000800 - 00FFFF     1110xxxx 10xxxxxx 10xxxxxx
	base[0] = 0xE0 | (ch >> 12);
	base[1] = 0x80 | ((ch >> 6) & 0x3F);
	base[2] = 0x80 | (ch & 0x3F);
	return base + 3;
}

/**
 * 将utf8字符串转化为unicode字符串
 */
void UTF8::convert_to_unicode(const char* utf8_str, jchar* unicode_str, int unicode_length) {
	unsigned char ch;
	const char *ptr = utf8_str;
	int index = 0;

	for (; index < unicode_length; index++) { // 000000 - 00007F     0xxxxxxx
		if ((ch = ptr[0]) > 0x7F) {
			break;
		}
		unicode_str[index] = ch;
		ptr = (const char *) (ptr + 1);
	}

	// 000080 - 0007FF     110xxxxx 10xxxxxx
	// 000800 - 00FFFF     1110xxxx 10xxxxxx 10xxxxxx
	for (; index < unicode_length; index++) {
		ptr = UTF8::next(ptr, &unicode_str[index]);
	}
}

// 将UTF8字符串转化为ASCII可打印字符串的字节长度
int UTF8::quoted_ascii_length(const char* utf8_str, int utf8_length) {
	const char* ptr = utf8_str;
	const char* end = ptr + utf8_length;
	int result = 0;
	while (ptr < end) {
		jchar c;
		ptr = UTF8::next(ptr, &c);
		if (c >= 32 && c < 127) { // 可打印字符
			result++;
		} else {
			result += 6;  // \uxxxx
		}
	}
	return result;
}

// 将UTF8字符串转化为ASCII可打印字符串，以\0结束
void UTF8::as_quoted_ascii(const char* utf8_str, int utf8_length, char* buf, int buflen) {
	const char *ptr = utf8_str;
	const char *utf8_end = ptr + utf8_length;
	char* p = buf;
	char* end = buf + buflen;
	while (ptr < utf8_end) {
		jchar c;
		ptr = UTF8::next(ptr, &c);
		if (c >= 32 && c < 127) {
			if (p + 1 >= end)
				break;      // string is truncated
			*p++ = (char) c;
		} else {
			if (p + 6 >= end)
				break;      // string is truncated
			sprintf(p, "\\u%04x", c); // \uxxxx
			p += 6;
		}
	}
	assert(p < end, "sanity");
	*p = '\0';
}

// 将ASCII可打印字符串转化为UTF8字符串
const char* UTF8::from_quoted_ascii(const char* quoted_ascii_str) {
	const char* ptr = quoted_ascii_str;
	char* result = NULL;
	while (*ptr != '\0') { // 判断是否有\uxxxx
		char c = *ptr;
		if (c < 32 || c >= 127) break;
	}
	if (*ptr == '\0') {
		return quoted_ascii_str; // 不需要转化\uxxxx，直接返回原始字符串
	}

	int length = ptr - quoted_ascii_str;
	char* buffer = NULL;
	for (int round = 0; round < 2; round++) {  // 第一遍先计算需要多少字节，申请空间；第一遍转化字符串
		while(*ptr != '\0') {
			if (*ptr != '\\') { // [32,127)
				if (buffer != NULL) {
					buffer[length] = *ptr;
				}
				length++;
			} else { // \uxxxx
				switch (ptr[1]) {
					case 'u': {
						ptr += 2;
						jchar value = 0;
						for (int i = 0; i < 4; i++) {
							char c = *ptr++;
							switch (c) {
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
								value = (value << 4) + c - '0';
								break;
							case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
								value = (value << 4) + 10 + c - 'a';
								break;
							case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
								value = (value << 4) + 10 + c - 'A';
								break;
							default:
								ShouldNotReachHere();
							}
						}
						if (buffer == NULL) {
							char utf8_buffer[4];
							char* next = (char*)utf8_write((u_char*)utf8_buffer, value);
							length += next - utf8_buffer;
						} else {
							char* next = (char*)utf8_write((u_char*)&buffer[length], value);
							length += next - &buffer[length];
						}
						break;
					};
					case 't': if (buffer != NULL) buffer[length] = '\t'; ptr += 2; length++; break;
					case 'n': if (buffer != NULL) buffer[length] = '\n'; ptr += 2; length++; break;
					case 'r': if (buffer != NULL) buffer[length] = '\r'; ptr += 2; length++; break;
					case 'f': if (buffer != NULL) buffer[length] = '\f'; ptr += 2; length++; break;
					default:
						ShouldNotReachHere();
				}
			}
		}
		if (round == 0) {
			buffer = NEW_RESOURCE_ARRAY(char, length + 1);
			ptr = quoted_ascii_str;
		} else {
			buffer[length] = '\0';
		}
	}
	return buffer;
}

// 在base[0,length)中查找c，若找到则返回其地址，若找不到返回NULL
const jbyte* UTF8::strrchr(const jbyte* base, int length, jbyte c) {
	assert(length >= 0, "sanity check");
	assert(c >= 0, "does not work for non-ASCII characters");
	// 后向遍历
	while (--length >= 0 && base[length] != c);
	return (length < 0) ? NULL : &base[length];
}

// 比较两个字符串是否相同
bool UTF8::equal(const jbyte* base1, int length1, const jbyte* base2, int length2) {
	if (length1 != length2) return false;
	for (int i = 0; i < length1; i++) {
		if (base1[i] != base2[i]) return false;
	}
	return true;
}

// 修改版UTF8
// 010000~10FFFF: 010000 + (00000 ~ FFFFF): 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
bool UTF8::is_supplementary_character(const unsigned char* str) {
	return ((str[0] & 0xFF) == 0xED) && ((str[1] & 0xF0) == 0xA0) && ((str[2] & 0xC0) == 0x80)
		&& ((str[3] & 0xFF) == 0xED) && ((str[4] & 0xF0) == 0xB0) && ((str[5] & 0xC0) == 0x80);
}

// 修改版UTF8
// 010000~10FFFF: 010000 + (00000 ~ FFFFF): 11101101 1010xxxx 10xxxxxx 11101101 1011xxxx 10xxxxxx
jint UTF8::get_supplementary_character(const unsigned char* str) {
	return 0x10000 + ((str[1] & 0x0f) << 16) + ((str[2] & 0x3f) << 10)
			       + ((str[4] & 0x0f) << 6)  + ( str[5] & 0x3f);
}


//------------------------------

/**
 * unicode码的UTF8字符的字节数
 */
int UNICODE::utf8_size(jchar c) {
	if((0x1 <= c) && (c <= 0x7F)) return 1;
	if(c <= 0x7FF) return 2;
	return 3;
}

/**
 * unicode码字符串的UTF8字符串的字节数
 */
int UNICODE::utf8_length(jchar* base, int length) {
	int result = 0;
	for (int index = 0; index < length; index++) {
		jchar c = base[index];
		if ((0x0001 <= c) && (c <= 0x007F)) result += 1;
		else if (c <= 0x07FF) result += 2;
		else result += 3;
	}
	return result;
}

/**
 * unicode码字符串转化为UTF8字符串
 */
char* UNICODE::as_utf8(jchar* base, int length) {
	int utf8_len = utf8_len(base, length);
	u_char* result = NEW_RESOURCE_ARRAY(u_char, utf8_len + 1);
	u_char* p = result;
	for (int index = 0; index < length; index++) {
		p = utf8_write(p, base[index]);
	}
	*p = '\0';
	assert(p == &&result[utf8_len], "length prediction must be correct");
	return (char*) result;
}

/**
 * unicode码字符串转化为UTF8字符串，存于buf
 */
char* UNICODE::as_utf8(jchar* base, int length, char* buf, int buflen) {
	u_char* p = (u_char*) buf;
	u_char* end = (u_char*) buf + buflen;
	for (int index = 0; index < length; index++) {
		jchar c = base[index];
		if (p + utf8_size(c) >= end)
			break;      // string is truncated
		p = utf8_write(p, base[index]);
	}
	*p = '\0';
	return buf;
}

/**
 * unicode码字符串转化为UTF8字符串，存于utf8_buffer
 */
void UNICODE::convert_to_utf8(const jchar* base, int length, char* utf8_buffer) {
	for (int index = 0; index < length; index++) {
		utf8_buffer = (char*)utf8_write((u_char*)utf8_buffer, base[index]);
	}
	*utf8_buffer = '\0';
}

/**
 * unicode字符串转化为可打印ascii字符串的字节长度
 */
int UNICODE::quoted_ascii_length(jchar* base, int length) {
	int result = 0;
	for (int i = 0; i < length; i++) {
		jchar c = base[i];
		if (c >= 32 && c < 127) {
			result++;
		} else {
			result += 6;
		}
	}
	return result;
}

/**
 * unicode字符串转化为可打印ascii字符串
 */
void UNICODE::as_quoted_ascii(const jchar* base, int length, char* buf, int buflen) {
	char* p = buf;
	char* end = buf + buflen;
	for (int index = 0; index < length; index++) {
		jchar c = base[index];
		if (c >= 32 && c < 127) {
			if (p + 1 >= end)
				break;      // string is truncated
			*p++ = (char) c;
		} else {
			if (p + 6 >= end)
				break;      // string is truncated
			sprintf(p, "\\u%04x", c);
			p += 6;
		}
	}
	*p = '\0';
}
