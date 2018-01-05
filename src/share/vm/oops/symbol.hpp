/*
 * symbol.hpp
 *
 *  Created on: 2018年1月5日
 *      Author: Administrator
 */

#ifndef SHARE_VM_OOPS_SYMBOL_HPP_
#define SHARE_VM_OOPS_SYMBOL_HPP_

#include "memory/allocation.hpp"
#include "runtime/atomic.hpp"

class SymbolBase: public MetaspaceObj {
public:
	ATOMIC_SHORT_PAIR(
	    volatile short _refcount,  // need atomic operation
		unsigned short _length     // number of UTF8 characters in the symbol (does not need atomic op)
	);
	int 			   _identity_hash;
};



#endif /* SHARE_VM_OOPS_SYMBOL_HPP_ */
