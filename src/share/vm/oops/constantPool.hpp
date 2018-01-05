/*
 * constantPool.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_OOPS_CONSTANTPOOL_HPP_
#define SHARE_VM_OOPS_CONSTANTPOOL_HPP_

#include "metadata.hpp"
#include "../utilities/array.hpp"
#include "../utilities/constantTag.hpp"
#include "../../../cpu/x86/vm/jni_x86.h"
#include "../prims/jni.h"
#include "../../../cpu/x86/vm/bytes_x86.hpp"

class CPSlot {
	intptr_t _ptr;
public:
	CPSlot(intptr_t ptr): _ptr(ptr) {}
	CPSlot(Klass* ptr): _ptr((intptr_t)ptr) {}
	CPSlot(Symbol* ptr): _ptr((intptr_t)ptr | 1) {}

	intptr_t value() { return _ptr; }
	Klass* get_klass() { return (Klass*)_ptr; }
	Symbol* get_symbol() { return (Symbol*)(_ptr & ~1); }

};


class ConstantPool: public Metadata {

private:
	Array<u1>*  _tags;
	ConstantPoolCache* _cache;
	InstanceKlass* _pool_holder;
	Array<u2>* _operands;

	jobject _resolved_references;
	Array<u2>* _reference_map;

	int _flags;
	int _length;

	void set_tags(Array<u1>* tags) { _tags = tags; }
	void tag_at_put(int which, jbyte t) { tags()->at_put(which, t); }
	void release_tag_at_put(int which, jbyte t) { tags()->release_at_put(which, t); }

	void set_operands(Array<u2>* operands) { _operands = operands; }

	int flags() const { return _flags; }
	void set_flags(int f) { _flags = f; }

private:

	intptr_t* base() const {
		return (intptr_t*)( ((char*) this) + sizeof(ConstantPool) );
	}

	jint* int_at_addr(int which) const {
		return (jint*) &base()[which];
	}

	jlong* long_at_addr(int which) const {
		return (jlong*) &base()[which];
	}

	jfloat* float_at_addr(int which) const {
		return (jfloat*) &base()[which];
	}

	jdouble* double_at_addr(int which) const {
		return (jdouble*) &base()[which];
	}

	ConstantPool(Array<u1> * tags);
public:
	static ConstantPool* allocate(int length);

	bool is_constantPool() const volatile { return true; }

	Array<u1>* tags() const { return _tags; }
	Array<u2>* operands() const { return _operands; }

	void klass_at_put(int which, Klass* k) {
		release_tag_at_put(which, JVM_CONSTANT_Class);
	}

	void klass_index_at_put(int which, int name_index) {
		tag_at_put(which, JVM_CONSTANT_ClassIndex);
		*int_at_addr(which) = name_index;
	}

	void method_handle_index_at_put(int which, int ref_kind, int ref_index) {
		tag_at_put(which, JVM_CONSTANT_MethodHandle);
		*int_at_addr(which) = ( (jint) ref_index << 16) | ref_kind;
	}

	void method_type_index_at_put(int which, int ref_index) {
		tag_at_put(which, JVM_CONSTANT_MethodType);
		*int_at_addr(which) = ref_index;
	}

	void invoke_dynamic_at_put(int which, int bootstrap_specifier_index, int name_and_type_index) {
		tag_at_put(which, JVM_CONSTANT_InvokeDynamic);
		*int_at_addr(which) = ( (jint) name_and_type_index << 16) | bootstrap_specifier_index;
	}

	void int_at_put(int which, jint i) {
		tag_at_put(which, JVM_CONSTANT_Integer);
		*int_at_addr(which) = i;
	}

	void long_at_put(int which, jlong l) {
		tag_at_put(which, JVM_CONSTANT_Long);
		Bytes::put_native_u8((address) long_at_addr(which), *((u8*) &l));
	}

	void float_at_put(int which, jfloat f) {
		tag_at_put(which, JVM_CONSTANT_Float);
		*float_at_addr(which) = f;
	}

	void double_at_put(int which, jdouble d) {
		tag_at_put(which, JVM_CONSTANT_Double);
		// *double_at_addr(which) = d;
		// u8 temp = *(u8*) &d;
		Bytes::put_native_u8((address) double_at_addr(which), *((u8*) &d));
	}

	void string_index_at_put(int which, int string_index) {
		tag_at_put(which, JVM_CONSTANT_StringIndex);
		*int_at_addr(which) = string_index;
	}

	void field_at_put(int which, int class_index, int name_and_type_index) {
		tag_at_put(which, JVM_CONSTANT_Fieldref);
		*int_at_addr(which) = ( (jint) name_and_type_index << 16) | class_index;
	}

	void method_at_put(int which, int class_index, int name_and_type_index) {
		tag_at_put(which, JVM_CONSTANT_Methodref);
		*int_at_addr(which) = ( (jint) name_and_type_index << 16) | class_index;
	}

	void interface_method_at_put(int which, int class_index, int name_and_type_index) {
		tag_at_put(which, JVM_CONSTANT_InterfaceMethodref);
		*int_at_addr(which) = ( (jint) name_and_type_index << 16) | class_index;
	}

	void name_and_type_at_put(int which, int name_index, int signature_index) {
		tag_at_put(which, JVM_CONSTANT_NameAndType);
		*int_at_addr(which) = ( (jint) signature_index << 16) | name_index;
	}
};



#endif /* SHARE_VM_OOPS_CONSTANTPOOL_HPP_ */
