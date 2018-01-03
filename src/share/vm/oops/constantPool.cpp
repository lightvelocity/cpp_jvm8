/*
 * constantPool.cpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#include "constantPool.hpp"

ConstantPool* ConstantPool::allocate(int length) {
	Array<u1>* tags;
	return new ConstantPool(tags);
}


