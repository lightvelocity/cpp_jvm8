/*
 * annotations.hpp
 *
 *  Created on: 2018年1月7日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_OOPS_ANNOTATIONS_HPP_
#define SHARE_VM_OOPS_ANNOTATIONS_HPP_

#include "oops/metadata.hpp"
#include "runtime/handles.hpp"
#include "utilities/array.hpp"
#include "utilities/exceptions.hpp"
#include "utilities/globalDefinitions.hpp"

//class ClassLoaderData;
//class outputStream;
//class KlassSizeStats;

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




};

#endif /* SHARE_VM_OOPS_ANNOTATIONS_HPP_ */
