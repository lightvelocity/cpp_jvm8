/*
 * classFileStream.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_CLASSFILE_CLASSFILESTREAM_HPP_
#define SHARE_VM_CLASSFILE_CLASSFILESTREAM_HPP_

#include "../utilities/globalDefinitions.hpp"

class ClassFileStream {
private:
	u1*   _buffer_start;
	u1*   _buffer_end;
	u1*   _current;
	char* _source;

public:
	ClassFileStream(u1* buffer, int length, char* source);

	u1* buffer() const  { return _buffer_start; }
	int length() const  { return _buffer_end - _buffer_start; }
	u1* current() const { return _current; }
	void set_current(u1* pos) { _current = pos; }

	u1 get_u1_fast() {
		return *_current++;
	}

	u2 get_u2_fast() {
		u2 res = Bytes::get_Java_u2(_current);
		_current += 2;
		return res;
	}

	u4 get_u4_fast() {
		u4 res = Bytes::get_Java_u4(_current);
		_current += 4;
		return res;
	}

	u8 get_u8_fast() {
		u8 res = Bytes::get_Java_u8(_current);
		_current += 8;
		return res;
	}
};



#endif /* SHARE_VM_CLASSFILE_CLASSFILESTREAM_HPP_ */
