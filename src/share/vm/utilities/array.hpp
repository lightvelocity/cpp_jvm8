/*
 * array.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_UTILITIES_ARRAY_HPP_
#define SHARE_VM_UTILITIES_ARRAY_HPP_

#include "../memory/allocation.hpp"

// Array for metadata allocation
template <typename T>
class Array: public MetaspaceObj {

protected:
	int _length;  // the number of array elements
	T   _data[1]; // the array memory

	void initialize(int length) {
		_length = length;
	}

private:

	Array(const Array<T>&);
	void operator=(const Array<T>&);

public:

	int length() const { return _length; }
	T* data() { return _data; }
	bool is_empty() const { return length() == 0; }

	int index_of(const T& x) const {
		int i = length();
		while (i-- > 0 && _data[i] != x) ;
		return i;
	}

	T at(int i) const { return _data[i]; }
	void at_put(const int i, const T& x) { _data[i] = x; }
	T* adr_at(const int i) { return &_data[i]; }
	int find(const T& x) { return index_of(x); }

};



#endif /* SHARE_VM_UTILITIES_ARRAY_HPP_ */
