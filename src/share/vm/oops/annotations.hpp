/*
 * annotations.hpp
 *
 *  Created on: 2018年1月7日
 *      Author: limaozhi
 */

// OK

#ifndef SHARE_VM_OOPS_ANNOTATIONS_HPP_
#define SHARE_VM_OOPS_ANNOTATIONS_HPP_

#include "oops/metadata.hpp"
#include "runtime/handles.hpp"
#include "utilities/array.hpp"
#include "utilities/exceptions.hpp"
#include "utilities/globalDefinitions.hpp"

class ClassLoaderData;
class outputStream;
class KlassSizeStats;

typedef Array<u1> AnnotationArray;

// Class to hold the various types of annotations. The only metadata that points
// to this is InstanceKlass, or another Annotations instance if this is a
// a type_annotation instance.

class Annotations: public MetaspaceObj {

	// Annotations for this class, or null if none.
	AnnotationArray* _class_annotations;

	// Annotation objects (byte arrays) for fields, or null if no annotations.
	// Indices correspond to entries (not indices) in fields array.
	Array<AnnotationArray*>* _fields_annotations;

	// Type annotations for this class, or null if none.
	AnnotationArray* _class_type_annotations;
	Array<AnnotationArray*>* _fields_type_annotations;

public:
	// Allocate instance of this class
	static Annotations* allocate(ClassLoaderData* loader_data, TRAPS);

	static void free_contents(ClassLoaderData* loader_data, Array<AnnotationArray*>* p);
	void deallocate_contents(ClassLoaderData* loader_data);
	DEBUG_ONLY(bool on_stack() {return false;}) // for template

	// Sizing (in words)
	static int size() { return sizeof(Annotations) / wordSize;}
#if INCLUDE_SERVICES
	void collect_statistics(KlassSizeStats *sz) const;
#endif

	// Constructor to initialize to null
	Annotations() :	_class_annotations(NULL),
			_fields_annotations(NULL),
			_class_type_annotations(NULL),
			_fields_type_annotations(NULL) {}

	AnnotationArray* class_annotations() const { return _class_annotations; }
	Array<AnnotationArray*>* fields_annotations() const { return _fields_annotations; }
	AnnotationArray* class_type_annotations() const { return _class_type_annotations;}
	Array<AnnotationArray*>* fields_type_annotations() const { return _fields_type_annotations; }

	void set_class_annotations(AnnotationArray* md) { _class_annotations = md; }
	void set_fields_annotations(Array<AnnotationArray*>* md) { _fields_annotations = md; }
	void set_class_type_annotations(AnnotationArray* cta) { _class_type_annotations = cta; }
	void set_fields_type_annotations(Array<AnnotationArray*>* fta) { _fields_type_annotations = fta; }

	// Turn metadata annotations into a Java heap object (oop)
	static typeArrayOop make_java_array(AnnotationArray* annotations, TRAPS);

	bool is_klass() const { return false; }
private:
	static julong count_bytes(Array<AnnotationArray*>* p);
public:
	const char* internal_name() const { return "{constant pool}"; }
#ifndef PRODUCT
	void print_on(outputStream* st) const;
#endif
	void print_value_on(outputStream* st) const;
};

#endif /* SHARE_VM_OOPS_ANNOTATIONS_HPP_ */
