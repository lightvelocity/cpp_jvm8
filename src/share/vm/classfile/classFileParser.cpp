/*
 * classFileParser.cpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#include "classFileParser.hpp"
#include "../oops/constantPool.hpp"
#include "../utilities/globalDefinitions.hpp"
#include "../prims/jvm.h"

#define JAVA_CLASSFILE_MAGIC              0xCAFEBABE
#define JAVA_MIN_SUPPORTED_VERSION        45
#define JAVA_MAX_SUPPORTED_VERSION        52
#define JAVA_MAX_SUPPORTED_MINOR_VERSION  0
#define JAVA_1_5_VERSION                  49
#define JAVA_6_VERSION                    50
#define JAVA_7_VERSION                    51
#define JAVA_8_VERSION                    52

void ClassFileParser::parseClassFile() {

    ClassFileStream* cfs = stream();

    // Magic value
    u4 magic = cfs->get_u4_fast();

    // Version numbers
    u2 minor_version = cfs->get_u2_fast();
    u2 major_version = cfs->get_u2_fast();


}

void ClassFileParser::parse_constant_pool() {

	ClassFileStream* cfs = stream();

	// Constant pool count
	u2 length = cfs->get_u2_fast();

	ConstantPool* constant_pool; // FIXME

	int index = 1;
	for (index = 1; index < length; index++) {
		jbyte tag = cp->tag_at(index).value();

	}

}

void ClassFileParser::parse_constant_pool_entries(int length) {
	ClassFileStream* cfs0 = stream();
	ClassFileStream cfs1 = *cfs0;
	ClassFileStream* cfs = &cfs1;

	u1* old_current = cfs0->current();

	const char* names[SymbolTable::symbol_alloc_batch_size];
	int lengths[SymbolTable::symbol_alloc_batch_size];
	int indices[SymbolTable::symbol_alloc_batch_size];
	unsigned int hashValues[SymbolTable::symbol_alloc_batch_size];
	int names_count = 0;

	for (int index = 1; index < length; index++) {
		u1 tag = cfs->get_u1_fast();
		switch (tag) {
			case JVM_CONSTANT_Class:
			{
				u2 name_index = cfs->get_u2_fast();
				_cp->klass_index_at_put(index, name_index);
			}
			break;
			case JVM_CONSTANT_Fieldref:
			{
				u2 class_index = cfs->get_u2_fast();
				u2 name_and_type_index = cfs->get_u2_fast();
				_cp->field_at_put(index, class_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_Methodref:
			{
				u2 class_index = cfs->get_u2_fast();
				u2 name_and_type_index = cfs->get_u2_fast();
				_cp->method_at_put(index, class_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_InterfaceMethodref:
			{
				u2 class_index = cfs->get_u2_fast();
				u2 name_and_type_index = cfs->get_u2_fast();
				_cp->interface_method_at_put(index, class_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_String:
			{
				u2 string_index = cfs->get_u2_fast();
				_cp->string_index_at_put(index, string_index);
			}
			break;
			case JVM_CONSTANT_MethodHandle:
			case JVM_CONSTANT_MethodType:
				if (tag == JVM_CONSTANT_MethodHandle) {
					u1 ref_kind = cfs->get_u1_fast();
					u2 method_index = cfs->get_u2_fast();
					_cp->method_handle_index_at_put(index, ref_kind, method_index);
				} else if (tag == JVM_CONSTANT_MethodType) {
					u2 signature_index = cfs->get_u2_fast();
					_cp->method_type_index_at_put(index, signature_index);
				}
			break;
			case JVM_CONSTANT_InvokeDynamic:
			{
				u2 bootstrap_specifier_index = cfs->get_u2_fast();
				u2 name_and_type_index = cfs->get_u2_fast();
				if (_max_bootstrap_specifier_index < (int) bootstrap_specifier_index)
					_max_bootstrap_specifier_index = (int) bootstrap_specifier_index;
				_cp->invoke_dynamic_at_put(index, bootstrap_specifier_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_Integer:
			{
				u4 bytes = cfs->get_u4_fast();
				_cp->int_at_put(index, (jint) bytes);
			}
			break;
			case JVM_CONSTANT_Float:
			{
				u4 bytes = cfs->get_u4_fast();
				_cp->float_at_put(index, *(jfloat*)&bytes);
			}
			break;
			case JVM_CONSTANT_Long:
			{
				u8 bytes = cfs->get_u8_fast();
				_cp->long_at_put(index, bytes);
			}
			index++; // skip entry following eight-byte constant, see JVM book p.98
			break;
			case JVM_CONSTANT_Double:
			{
				u8 bytes = cfs->get_u8_fast();
				_cp->double_at_put(index, *(jdouble*)&bytes);
			}
			index++; // skip entry following eight-byte constant, see JVM book p.98
			break;
			case JVM_CONSTANT_NameAndType:
			{
				u2 name_index = cfs->get_u2_fast();
				u2 signature_index = cfs->get_u2_fast();
				_cp->name_and_type_at_put(index, name_index, signature_index);
			}
			break;
			case JVM_CONSTANT_Utf8:
			{
				u2 utf8_length = cfs->get_u2_fast();
				u1* utf8_buffer = cfs->get_u1_fast();
				cfs->skip_u1_fast(utf8_length);

			}
		}
	}



}
