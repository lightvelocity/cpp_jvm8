/*
 * atomic.hpp
 *
 *  Created on: 2018年1月5日
 *      Author: Administrator
 */

#ifndef SHARE_VM_RUNTIME_ATOMIC_HPP_
#define SHARE_VM_RUNTIME_ATOMIC_HPP_

class Atomic {


};

// VM_LITTLE_ENDIAN
#ifdef VM_LITTLE_ENDIAN
#define ATOMIC_SHORT_PAIR(atomic_decl, non_atomic_decl) \
	non_atomic_decl; \
	atomic_decl
#else
#define ATOMIC_SHORT_PAIR(atomic_decl, non_atomic_decl) \
	atomic_decl; \
	non_atomic_decl
#endif


#endif /* SHARE_VM_RUNTIME_ATOMIC_HPP_ */
