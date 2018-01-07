/*
 * os.hpp
 *
 *  Created on: 2018年1月7日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_RUNTIME_OS_HPP_
#define SHARE_VM_RUNTIME_OS_HPP_

#include "runtime/atomic.hpp"

class os: AllStatic {
public:
	enum { page_sizes_max = 9 }; // Size of _page_sizes array (8 plus a sentinel)
public:
	static size_t _page_sizes[page_sizes_max];
public:
	// threads

	enum ThreadType {
		vm_thread, cgc_thread,        // Concurrent GC thread
		pgc_thread,        // Parallel GC thread
		java_thread,
		compiler_thread,
		watcher_thread,
		os_thread
	};

	// General allocation (must be MT-safe)
	static void* malloc (size_t size, MEMFLAGS falgs, address caller_pc = 0);
	static void* realloc(void *memblock, size_t size, MEMFLAGS flags, address caller_pc = 0);
	static void  free   (void *memblock, MEMFLAGS flags = mtNone);
};



#endif /* SHARE_VM_RUNTIME_OS_HPP_ */
