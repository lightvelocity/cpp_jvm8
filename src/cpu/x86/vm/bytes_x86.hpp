/*
 * bytes_x86.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef CPU_X86_VM_BYTES_X86_HPP_
#define CPU_X86_VM_BYTES_X86_HPP_

#include "../../../share/vm/utilities/globalDefinitions.hpp"

class Bytes {
private:

public:
	 static inline u2 get_native_u2(address p) { return *(u2*)p; }
	static inline u4 get_native_u4(address p) { return *(u4*)p; }
	static inline u8 get_native_u8(address p) { return *(u8*)p; }

	// x86 little-endian to Java big-endian
	static inline u2 get_Java_u2(address p) { return swap_u2(get_native_u2(p)); }
	static inline u2 get_Java_u4(address p) { return swap_u4(get_native_u4(p)); }
	static inline u2 get_Java_u8(address p) { return swap_u8(get_native_u8(p)); }

	static inline u2 swap_u2(u2 x) {
		address p = (address) &x;
		return ( (u2(p[0]) << 8) | (u2(p[1])) );
	}

	static inline u4 swap_u4(u4 x) {
		address p = (address) &x;
		return ( (u4(p[0]) << 24) | (u4(p[1]) << 16) | (u4(p[2]) << 8) | (u4(p[3])) );
	}

	static inline u8 swap_u8(u8 x) {
		address p = (address) &x;
		return ( (u8(p[0]) << 56) | (u8(p[1]) << 48) | (u8(p[2]) << 40) | (u8(p[3]) << 32) |
			(u8(p[4]) << 24) | (u8(p[5]) << 16) | (u8(p[6]) << 8) | (u8(p[7])) );
	}
};



#endif /* CPU_X86_VM_BYTES_X86_HPP_ */
