/*
 * hashtable.cpp
 *
 *  Created on: 2018年1月9日
 *      Author: Administrator
 */

#include "precompiled.hpp"
#include "classfile/altHashing.hpp"
#include "classfile/javaClasses.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/filemap.hpp"
#include "memory/resourceArea.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/safepoint.hpp"
#include "utilities/dtrace.hpp"
#include "utilities/hashtable.hpp"
#include "utilities/hashtable.inline.hpp"
#include "utilities/numberSeq.hpp"

// This is a generic hashtable, designed to be used for the symbol
// and string tables.
//
// It is implemented as an open hash table with a fixed number of buckets.
//
// %note:
//  - HashtableEntrys are allocated in blocks to reduce the space overhead.

template <MEMFLAGS F> BasicHashtableEntry<F>* BasicHashtable<F>::new_entry(unsigned int hashValue) {
	BasicHashtableEntry<F>* entry;

	if (_free_list) {
		entry = _free_list;
		_free_list = _free_list->next();
	} else {
		if (_first_free_entry + _entry_size >= _end_block) {
			int block_size = MIN2(512, MAX2((int)_table_size / 2, (int)_number_of_entries));
			int len = _entry_size * block_size;
			len = 1 << log2_intptr(len); // round down to power of 2
			assert(len >= _entry_size, "");
			_first_free_entry = NEW_C_HEAP_ARRAY2(char, len, F, CURRENT_PC);
			_end_block = _first_free_entry + len;
		}
		entry = (BasicHashtableEntry<F>*)_first_free_entry;
		_first_free_entry += _entry_size;
	}

	assert(_entry_size % HeapWordSize == 0, "");
	entry->set_hash(hashValue);
	return entry;
}

template <class T, MEMFLAGS F> HashtableEntry<T, F>* Hashtable<T, F>::new_entry(unsigned int hashValue, T obj) {
	HashtableEntry<T, F>* entry;

	entry = (HashtableEntry<T, F>*)BasicHashtable<F>::new_entry(hashValue);
	entry -> set_literal(obj);
	return entry;
}

// Check to see if the hashtable is unbalanced.  The caller set a flag to
// rehash at the next safepoint.  If this bucket is 60 times greater than the
// expected average bucket length, it's an unbalanced hashtable.
// This is somewhat an arbitrary heuristic but if one bucket gets to
// rehash_count which is currently 100, there's probably something wrong.

template <MEMFLAGS F> bool BasicHashtable<F>::check_rehash_table(int count) {
	assert(table_size() != 0, "underflow");
	if (count > ( ( (double)number_of_entries() / (double)table_size() ) * rehash_multiple ) ) {
		return true;
	}
	return false;
}



template <class T, MEMFLAGS F> jint Hashtable<T, F>::_seed = 0;

// Create a new table and using alternate hash code, populate the new table
// with the existing elements.   This can be used to change the hash code
// and could in the future change the size of the table.

