/*
 * allocation.inline.hpp
 *
 *  Created on: 2018年1月7日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_MEMORY_ALLOCATION_INLINE_HPP_
#define SHARE_VM_MEMORY_ALLOCATION_INLINE_HPP_

#include "allocation.hpp"
#include "runtime/atomic.inline.hpp"
#include "runtime/os.hpp"

// allocate using malloc; will fail if no memory available
inline char* AllocateHeap(size_t size, MEMFLAGS flags, address pc = 0,
	AllocFailType alloc_failmode = AllocFailStrategy::EXIT_OOM) {

	if (pc == 0) {
		pc = CURRENT_PC;
	}

	char* p = (char*) os::malloc(size, flags, pc);

	if (p == NULL && alloc_failmode == AllocFailStrategy::EXIT_OOM) {
		vm_exit_out_of_memory(size, OOM_MALLOC_ERROR, "AllocateHeap");
	}
	return p;
}



#endif /* SHARE_VM_MEMORY_ALLOCATION_INLINE_HPP_ */
