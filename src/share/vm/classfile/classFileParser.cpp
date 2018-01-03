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
		}
	}



}
