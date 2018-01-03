/*
 * constantPool.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_OOPS_CONSTANTPOOL_HPP_
#define SHARE_VM_OOPS_CONSTANTPOOL_HPP_

#include "metadata.hpp"
#include "../utilities/globalDefinitions.hpp"
#include "../utilities/array.hpp"

class ConstantPool : public Metadata {

private:
	Array<u1>*  _tags; // the tag array describing the constant pool's contents

	void set_tags(Array<u1>* tags) { _tags = tags; }
	void tag_at_put(int which, jbyte t) { tags()->at_put(which, t); }
	//void release_tag_at_put(int which, jbyte t) { tags()->release_at_put(which, t); }


	ConstantPool(Array<u1> * tags);
public:
	static ConstantPool* allocate(int length);

	bool is_constantPool() const volatile { return true; }

	Array<u1>* tags() const { return _tags; }

};



#endif /* SHARE_VM_OOPS_CONSTANTPOOL_HPP_ */
