/*
 * classFileParser.hpp
 *
 *  Created on: 2018年1月3日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_CLASSFILE_CLASSFILEPARSER_HPP_
#define SHARE_VM_CLASSFILE_CLASSFILEPARSER_HPP_

#include "classFileStream.hpp"
#include "../oops/constantPool.hpp"

class ClassFileParser {
private:
	u2 _major_version;
	u2 _minor_version;
	Symbol* _class_name;
	ClassLoaderData* _loader_data;
	KlassHandle _host_klass;

	instanceKlassHandle _super_klass;
	ConstantPool* _cp;
	Array<u2>* _fields;

    ClassFileStream* _stream; // Actual input stream
    ClassFileStream* stream() { return _stream; }
    void set_stream(ClassFileStream* st) { _stream = st; }

public:
    ClassFileParser(ClassFileStream* st) { set_stream(st); }
    ~ClassFileParser();

    void parseClassFile(); // FIXME
    void parse_constant_pool();
    void parse_constant_pool_entries(int length);

};



#endif /* SHARE_VM_CLASSFILE_CLASSFILEPARSER_HPP_ */
