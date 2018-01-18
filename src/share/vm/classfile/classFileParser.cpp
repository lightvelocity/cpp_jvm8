/*
 * Copyright (c) 1997, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "precompiled.hpp"
#include "classfile/classFileParser.hpp"
#include "classfile/classLoader.hpp"
#include "classfile/classLoaderData.hpp"
#include "classfile/classLoaderData.inline.hpp"
#include "classfile/defaultMethods.hpp"
#include "classfile/javaClasses.hpp"
#include "classfile/symbolTable.hpp"
#include "classfile/systemDictionary.hpp"
#include "classfile/verificationType.hpp"
#include "classfile/verifier.hpp"
#include "classfile/vmSymbols.hpp"
#include "memory/allocation.hpp"
#include "memory/gcLocker.hpp"
#include "memory/metadataFactory.hpp"
#include "memory/oopFactory.hpp"
#include "memory/referenceType.hpp"
#include "memory/universe.inline.hpp"
#include "oops/constantPool.hpp"
#include "oops/fieldStreams.hpp"
#include "oops/instanceKlass.hpp"
#include "oops/instanceMirrorKlass.hpp"
#include "oops/klass.inline.hpp"
#include "oops/klassVtable.hpp"
#include "oops/method.hpp"
#include "oops/symbol.hpp"
#include "prims/jvm.h"
#include "prims/jvmtiExport.hpp"
#include "prims/jvmtiThreadState.hpp"
#include "runtime/javaCalls.hpp"
#include "runtime/perfData.hpp"
#include "runtime/reflection.hpp"
#include "runtime/signature.hpp"
#include "runtime/timer.hpp"
#include "services/classLoadingService.hpp"
#include "services/threadService.hpp"
#include "utilities/array.hpp"
#include "utilities/globalDefinitions.hpp"

// We generally try to create the oops directly when parsing, rather than
// allocating temporary data strucures and copying the bytes twice. A
// temporary area is only needed when parsing utf8 entries in the constant
// pool and when parsing line number tables.

// We add assert in debug mode when class format is not checked

#define JAVA_CLASSFILE_MAGIC              0xCAFEBABE
#define JAVA_MIN_SUPPORTED_VERSION        45
#define JAVA_MAX_SUPPORTED_VERSION        52
#define JAVA_MAX_SUPPORTED_MINOR_VERSION  0

// Used for two backward compatibility reasons:
// - to check for new additions to the class file format in JDK1.5
// - to check for bug fixes in the format checker in JDK1.5
#define JAVA_1_5_VERSION                  49

// Used for backward compatibility reasons:
// - to check for javac bug fixes that happened after 1.5
// - also used as the max version when running in jdk6
#define JAVA_6_VERSION                    50

// Used for backward compatibility reasons:
// - to check NameAndType_info signatures more aggressively
#define JAVA_7_VERSION                    51

// Extension method support.
#define JAVA_8_VERSION                    52

void ClassFileParser::parse_constant_pool_entries(int length, TRAPS) {
	// use a local copy of ClassFileStream. It helps the C++ compiler to optimize
	// this function (_current can be allocated in a register, with scalar
	// replacement of aggregates). The _current pointer is copied back to
	// stream() when this funciton returns. DON'T call another method within
	// this method that use stream().
	ClassFileStream*	cfs0 = stream();
	ClassFileStream		cfs1 = *cfs0;
	ClassFileStream*	cfs	 = &cfs1;
#ifdef ASSERT
	assert(cfs->allocated_on_stack(), "should be local");
	u1* old_current = cfs0->current();
#endif
	Handle class_loader(THREAD, _loader_data->class_loader());

	// Used for batching symbol allocations
	const char*	names[SymbolTable::symbol_alloc_batch_size];
	int lengths[SymbolTable::symbol_alloc_batch_size];
	int indices[SymbolTable::symbol_alloc_batch_size];
	unsigned int hashValues[SymbolTable::symbol_alloc_batch_size];
	int names_const = 0;

	// parsing Index 0 is ununsed
	for (int index = 1; index < length; index++) {
		// Each of the following case guarantees one more byte in the stream
		// for the following tag or the access_flags following constant pool,
		// so we don't need bounds-check for reading tag.
		u1 tag = cfs->get_u1_fast();
		switch (tag) {
			case JVM_CONSTANT_Class :
			{
				cfs->guarantee_more(3, CHECK); // name_index, tag/access_flags
				u2 name_index = cfs->get_u2_fast();
				_cp->klass_index_at_put(index, name_index);
			}
			break;
			case JVM_CONSTANT_Filedref:
			{
				cfs->guarantee_more(5, CHECK); // class_index, name_and_type_index, tag/access_flags
				u2 class_index = cfs->get_u2_fast();
				u2 name_and_type_index = cfs->get_u2_fast();
				_cp->field_at_put(index, class_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_Methodref :
			{
			  cfs->guarantee_more(5, CHECK);  // class_index, name_and_type_index, tag/access_flags
			  u2 class_index = cfs->get_u2_fast();
			  u2 name_and_type_index = cfs->get_u2_fast();
			  _cp->method_at_put(index, class_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_InterfaceMethodref :
			{
			  cfs->guarantee_more(5, CHECK);  // class_index, name_and_type_index, tag/access_flags
			  u2 class_index = cfs->get_u2_fast();
			  u2 name_and_type_index = cfs->get_u2_fast();
			  _cp->interface_method_at_put(index, class_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_String :
			{
			  cfs->guarantee_more(3, CHECK);  // string_index, tag/access_flags
			  u2 string_index = cfs->get_u2_fast();
			  _cp->string_index_at_put(index, string_index);
			}
			break;
			case JVM_CONSTANT_MethodHandle :
			case JVM_CONSTANT_MethodType :
			if (_major_version < Verifier::INVOKEDYNAMIC_MAJOR_VERSION) {
			  classfile_parse_error(
				"Class file version does not support constant tag %u in class file %s",
				tag, CHECK);
			}
			if (!EnableInvokeDynamic) {
			  classfile_parse_error(
				"This JVM does not support constant tag %u in class file %s",
				tag, CHECK);
			}
			if (tag == JVM_CONSTANT_MethodHandle) {
			  cfs->guarantee_more(4, CHECK);  // ref_kind, method_index, tag/access_flags
			  u1 ref_kind = cfs->get_u1_fast();
			  u2 method_index = cfs->get_u2_fast();
			  _cp->method_handle_index_at_put(index, ref_kind, method_index);
			} else if (tag == JVM_CONSTANT_MethodType) {
			  cfs->guarantee_more(3, CHECK);  // signature_index, tag/access_flags
			  u2 signature_index = cfs->get_u2_fast();
			  _cp->method_type_index_at_put(index, signature_index);
			} else {
			  ShouldNotReachHere();
			}
			break;
			case JVM_CONSTANT_InvokeDynamic :
			{
			  if (_major_version < Verifier::INVOKEDYNAMIC_MAJOR_VERSION) {
				classfile_parse_error(
				  "Class file version does not support constant tag %u in class file %s",
				  tag, CHECK);
			  }
			  if (!EnableInvokeDynamic) {
				classfile_parse_error(
				  "This JVM does not support constant tag %u in class file %s",
				  tag, CHECK);
			  }
			  cfs->guarantee_more(5, CHECK);  // bsm_index, nt, tag/access_flags
			  u2 bootstrap_specifier_index = cfs->get_u2_fast();
			  u2 name_and_type_index = cfs->get_u2_fast();
			  if (_max_bootstrap_specifier_index < (int) bootstrap_specifier_index)
				_max_bootstrap_specifier_index = (int) bootstrap_specifier_index;  // collect for later
			  _cp->invoke_dynamic_at_put(index, bootstrap_specifier_index, name_and_type_index);
			}
			break;
			case JVM_CONSTANT_Integer :
			{
			  cfs->guarantee_more(5, CHECK);  // bytes, tag/access_flags
			  u4 bytes = cfs->get_u4_fast();
			  _cp->int_at_put(index, (jint) bytes);
			}
			break;
			case JVM_CONSTANT_Float :
			{
			  cfs->guarantee_more(5, CHECK);  // bytes, tag/access_flags
			  u4 bytes = cfs->get_u4_fast();
			  _cp->float_at_put(index, *(jfloat*)&bytes);
			}
			break;
			case JVM_CONSTANT_Long :
			// A mangled type might cause you to overrun allocated memory
			guarantee_property(index+1 < length,
							   "Invalid constant pool entry %u in class file %s",
							   index, CHECK);
			{
			  cfs->guarantee_more(9, CHECK);  // bytes, tag/access_flags
			  u8 bytes = cfs->get_u8_fast();
			  _cp->long_at_put(index, bytes);
			}
			index++;   // Skip entry following eigth-byte constant, see JVM book p. 98
			break;
			case JVM_CONSTANT_Double :
			// A mangled type might cause you to overrun allocated memory
			guarantee_property(index+1 < length,
							   "Invalid constant pool entry %u in class file %s",
							   index, CHECK);
			{
			  cfs->guarantee_more(9, CHECK);  // bytes, tag/access_flags
			  u8 bytes = cfs->get_u8_fast();
			  _cp->double_at_put(index, *(jdouble*)&bytes);
			}
			index++;   // Skip entry following eigth-byte constant, see JVM book p. 98
			break;
			case JVM_CONSTANT_NameAndType :
			{
			  cfs->guarantee_more(5, CHECK);  // name_index, signature_index, tag/access_flags
			  u2 name_index = cfs->get_u2_fast();
			  u2 signature_index = cfs->get_u2_fast();
			  _cp->name_and_type_at_put(index, name_index, signature_index);
			}
			break;
			case JVM_CONSTANT_Utf8 :
			{
			  cfs->guarantee_more(2, CHECK);  // utf8_length
			  u2  utf8_length = cfs->get_u2_fast();
			  u1* utf8_buffer = cfs->get_u1_buffer();
			  assert(utf8_buffer != NULL, "null utf8 buffer");
			  // Got utf8 string, guarantee utf8_length+1 bytes, set stream position forward.
			  cfs->guarantee_more(utf8_length+1, CHECK);  // utf8 string, tag/access_flags
			  cfs->skip_u1_fast(utf8_length);

			  // Before storing the symbol, make sure it's legal
			  if (_need_verify) {
				verify_legal_utf8((unsigned char*)utf8_buffer, utf8_length, CHECK);
			  }

			  if (EnableInvokeDynamic && has_cp_patch_at(index)) {
				Handle patch = clear_cp_patch_at(index);
				guarantee_property(java_lang_String::is_instance(patch()),
								   "Illegal utf8 patch at %d in class file %s",
								   index, CHECK);
				char* str = java_lang_String::as_utf8_string(patch());
				// (could use java_lang_String::as_symbol instead, but might as well batch them)
				utf8_buffer = (u1*) str;
				utf8_length = (int) strlen(str);
			  }

			  unsigned int hash;
			  Symbol* result = SymbolTable::lookup_only((char*)utf8_buffer, utf8_length, hash);
			  if (result == NULL) {
				names[names_count] = (char*)utf8_buffer;
				lengths[names_count] = utf8_length;
				indices[names_count] = index;
				hashValues[names_count++] = hash;
				if (names_count == SymbolTable::symbol_alloc_batch_size) {
				  SymbolTable::new_symbols(_loader_data, _cp, names_count, names, lengths, indices, hashValues, CHECK);
				  names_count = 0;
				}
			  } else {
				_cp->symbol_at_put(index, result);
			  }
			}
			break;
			default:
			classfile_parse_error(
			  "Unknown constant tag %u in class file %s", tag, CHECK);
			break;
		}
	}

	// Allocate the remaining symbols
	if (names_count > 0) {
	    SymbolTable::new_symbols(_loader_data, _cp, names_count, names, lengths, indices, hashValues, CHECK);
	  }

	  // Copy _current pointer of local copy back to stream().
	#ifdef ASSERT
	  assert(cfs0->current() == old_current, "non-exclusive use of stream()");
	#endif
	  cfs0->set_current(cfs1.current());
}

bool inline valid_cp_range(int index, int length) { return (index > 0 && index < length); }

inline Symbol* check_symbol_at(constantPoolHandle cp, int index) {
  if (valid_cp_range(index, cp->length()) && cp->tag_at(index).is_utf8())
    return cp->symbol_at(index);
  else
    return NULL;
}

constantPoolHandle ClassFileParser::parse_constant_pool(TRAPS) {
	ClassFileStream* cfs = stream();
	constantPoolHandle nullHandle;

	cfs->guarantee_more(3, CHECK_(nullHandle)); // length, first cp tag
	u2 length = cfs->get_u2_fast();
	guarantee_property(
	length >= 1, "Illegal constant pool size %u in class file %s",
	length, CHECK_(nullHandle));
	ConstantPool* constant_pool = ConstantPool::allocate(_loader_data, length,
														CHECK_(nullHandle));
	_cp = constant_pool; // save in case of errors
	constantPoolHandle cp (THREAD, constant_pool);

	// parsing constant pool entries
	parse_constant_pool_entries(length, CHECK_(nullHandle));

	int index = 1;  // declared outside of loops for portability

	// first verification pass - validate cross references and fixup class and string constants
	  for (index = 1; index < length; index++) {          // Index 0 is unused
	    jbyte tag = cp->tag_at(index).value();
	    switch (tag) {
	      case JVM_CONSTANT_Class :
	        ShouldNotReachHere();     // Only JVM_CONSTANT_ClassIndex should be present
	        break;
	      case JVM_CONSTANT_Fieldref :
	        // fall through
	      case JVM_CONSTANT_Methodref :
	        // fall through
	      case JVM_CONSTANT_InterfaceMethodref : {
	        if (!_need_verify) break;
	        int klass_ref_index = cp->klass_ref_index_at(index);
	        int name_and_type_ref_index = cp->name_and_type_ref_index_at(index);
	        check_property(valid_klass_reference_at(klass_ref_index),
	                       "Invalid constant pool index %u in class file %s",
	                       klass_ref_index,
	                       CHECK_(nullHandle));
	        check_property(valid_cp_range(name_and_type_ref_index, length) &&
	                       cp->tag_at(name_and_type_ref_index).is_name_and_type(),
	                       "Invalid constant pool index %u in class file %s",
	                       name_and_type_ref_index,
	                       CHECK_(nullHandle));
	        break;
	      }
	      case JVM_CONSTANT_String :
	        ShouldNotReachHere();     // Only JVM_CONSTANT_StringIndex should be present
	        break;
	      case JVM_CONSTANT_Integer :
	        break;
	      case JVM_CONSTANT_Float :
	        break;
	      case JVM_CONSTANT_Long :
	      case JVM_CONSTANT_Double :
	        index++;
	        check_property(
	          (index < length && cp->tag_at(index).is_invalid()),
	          "Improper constant pool long/double index %u in class file %s",
	          index, CHECK_(nullHandle));
	        break;
	      case JVM_CONSTANT_NameAndType : {
	        if (!_need_verify) break;
	        int name_ref_index = cp->name_ref_index_at(index);
	        int signature_ref_index = cp->signature_ref_index_at(index);
	        check_property(valid_symbol_at(name_ref_index),
	                 "Invalid constant pool index %u in class file %s",
	                 name_ref_index, CHECK_(nullHandle));
	        check_property(valid_symbol_at(signature_ref_index),
	                 "Invalid constant pool index %u in class file %s",
	                 signature_ref_index, CHECK_(nullHandle));
	        break;
	      }
	      case JVM_CONSTANT_Utf8 :
	        break;
	      case JVM_CONSTANT_UnresolvedClass :         // fall-through
	      case JVM_CONSTANT_UnresolvedClassInError:
	        ShouldNotReachHere();     // Only JVM_CONSTANT_ClassIndex should be present
	        break;
	      case JVM_CONSTANT_ClassIndex :
	        {
	          int class_index = cp->klass_index_at(index);
	          check_property(valid_symbol_at(class_index),
	                 "Invalid constant pool index %u in class file %s",
	                 class_index, CHECK_(nullHandle));
	          cp->unresolved_klass_at_put(index, cp->symbol_at(class_index));
	        }
	        break;
	      case JVM_CONSTANT_StringIndex :
	        {
	          int string_index = cp->string_index_at(index);
	          check_property(valid_symbol_at(string_index),
	                 "Invalid constant pool index %u in class file %s",
	                 string_index, CHECK_(nullHandle));
	          Symbol* sym = cp->symbol_at(string_index);
	          cp->unresolved_string_at_put(index, sym);
	        }
	        break;
	      case JVM_CONSTANT_MethodHandle :
	        {
	          int ref_index = cp->method_handle_index_at(index);
	          check_property(
	            valid_cp_range(ref_index, length) &&
	                EnableInvokeDynamic,
	              "Invalid constant pool index %u in class file %s",
	              ref_index, CHECK_(nullHandle));
	          constantTag tag = cp->tag_at(ref_index);
	          int ref_kind  = cp->method_handle_ref_kind_at(index);
	          switch (ref_kind) {
	          case JVM_REF_getField:
	          case JVM_REF_getStatic:
	          case JVM_REF_putField:
	          case JVM_REF_putStatic:
	            check_property(
	              tag.is_field(),
	              "Invalid constant pool index %u in class file %s (not a field)",
	              ref_index, CHECK_(nullHandle));
	            break;
	          case JVM_REF_invokeVirtual:
	          case JVM_REF_newInvokeSpecial:
	            check_property(
	              tag.is_method(),
	              "Invalid constant pool index %u in class file %s (not a method)",
	              ref_index, CHECK_(nullHandle));
	            break;
	          case JVM_REF_invokeStatic:
	          case JVM_REF_invokeSpecial:
	            check_property(tag.is_method() ||
	                           ((_major_version >= JAVA_8_VERSION) && tag.is_interface_method()),
	               "Invalid constant pool index %u in class file %s (not a method)",
	               ref_index, CHECK_(nullHandle));
	             break;
	          case JVM_REF_invokeInterface:
	            check_property(
	              tag.is_interface_method(),
	              "Invalid constant pool index %u in class file %s (not an interface method)",
	              ref_index, CHECK_(nullHandle));
	            break;
	          default:
	            classfile_parse_error(
	              "Bad method handle kind at constant pool index %u in class file %s",
	              index, CHECK_(nullHandle));
	          }
	          // Keep the ref_index unchanged.  It will be indirected at link-time.
	        }
	        break;
	      case JVM_CONSTANT_MethodType :
	        {
	          int ref_index = cp->method_type_index_at(index);
	          check_property(valid_symbol_at(ref_index) && EnableInvokeDynamic,
	                 "Invalid constant pool index %u in class file %s",
	                 ref_index, CHECK_(nullHandle));
	        }
	        break;
	      case JVM_CONSTANT_InvokeDynamic :
	        {
	          int name_and_type_ref_index = cp->invoke_dynamic_name_and_type_ref_index_at(index);
	          check_property(valid_cp_range(name_and_type_ref_index, length) &&
	                         cp->tag_at(name_and_type_ref_index).is_name_and_type(),
	                         "Invalid constant pool index %u in class file %s",
	                         name_and_type_ref_index,
	                         CHECK_(nullHandle));
	          // bootstrap specifier index must be checked later, when BootstrapMethods attr is available
	          break;
	        }
	      default:
	        fatal(err_msg("bad constant pool tag value %u",
	                      cp->tag_at(index).value()));
	        ShouldNotReachHere();
	        break;
	    } // end of switch
	  } // end of for

	  if (_cp_patches != NULL) {
	    // need to treat this_class specially...
	    assert(EnableInvokeDynamic, "");
	    int this_class_index;
	    {
	      cfs->guarantee_more(8, CHECK_(nullHandle));  // flags, this_class, super_class, infs_len
	      u1* mark = cfs->current();
	      u2 flags         = cfs->get_u2_fast();
	      this_class_index = cfs->get_u2_fast();
	      cfs->set_current(mark);  // revert to mark
	    }

	    for (index = 1; index < length; index++) {          // Index 0 is unused
	      if (has_cp_patch_at(index)) {
	        guarantee_property(index != this_class_index,
	                           "Illegal constant pool patch to self at %d in class file %s",
	                           index, CHECK_(nullHandle));
	        patch_constant_pool(cp, index, cp_patch_at(index), CHECK_(nullHandle));
	      }
	    }
	  }

	  if (!_need_verify) {
	    return cp;
	  }

	  // second verification pass - checks the strings are of the right format.
	  // but not yet to the other entries
	  for (index = 1; index < length; index++) {
	    jbyte tag = cp->tag_at(index).value();
	    switch (tag) {
	      case JVM_CONSTANT_UnresolvedClass: {
	        Symbol*  class_name = cp->unresolved_klass_at(index);
	        // check the name, even if _cp_patches will overwrite it
	        verify_legal_class_name(class_name, CHECK_(nullHandle));
	        break;
	      }
	      case JVM_CONSTANT_NameAndType: {
	        if (_need_verify && _major_version >= JAVA_7_VERSION) {
	          int sig_index = cp->signature_ref_index_at(index);
	          int name_index = cp->name_ref_index_at(index);
	          Symbol*  name = cp->symbol_at(name_index);
	          Symbol*  sig = cp->symbol_at(sig_index);
	          if (sig->byte_at(0) == JVM_SIGNATURE_FUNC) {
	            verify_legal_method_signature(name, sig, CHECK_(nullHandle));
	          } else {
	            verify_legal_field_signature(name, sig, CHECK_(nullHandle));
	          }
	        }
	        break;
	      }
	      case JVM_CONSTANT_InvokeDynamic:
	      case JVM_CONSTANT_Fieldref:
	      case JVM_CONSTANT_Methodref:
	      case JVM_CONSTANT_InterfaceMethodref: {
	        int name_and_type_ref_index = cp->name_and_type_ref_index_at(index);
	        // already verified to be utf8
	        int name_ref_index = cp->name_ref_index_at(name_and_type_ref_index);
	        // already verified to be utf8
	        int signature_ref_index = cp->signature_ref_index_at(name_and_type_ref_index);
	        Symbol*  name = cp->symbol_at(name_ref_index);
	        Symbol*  signature = cp->symbol_at(signature_ref_index);
	        if (tag == JVM_CONSTANT_Fieldref) {
	          verify_legal_field_name(name, CHECK_(nullHandle));
	          if (_need_verify && _major_version >= JAVA_7_VERSION) {
	            // Signature is verified above, when iterating NameAndType_info.
	            // Need only to be sure it's the right type.
	            if (signature->byte_at(0) == JVM_SIGNATURE_FUNC) {
	              throwIllegalSignature(
	                  "Field", name, signature, CHECK_(nullHandle));
	            }
	          } else {
	            verify_legal_field_signature(name, signature, CHECK_(nullHandle));
	          }
	        } else {
	          verify_legal_method_name(name, CHECK_(nullHandle));
	          if (_need_verify && _major_version >= JAVA_7_VERSION) {
	            // Signature is verified above, when iterating NameAndType_info.
	            // Need only to be sure it's the right type.
	            if (signature->byte_at(0) != JVM_SIGNATURE_FUNC) {
	              throwIllegalSignature(
	                  "Method", name, signature, CHECK_(nullHandle));
	            }
	          } else {
	            verify_legal_method_signature(name, signature, CHECK_(nullHandle));
	          }
	          if (tag == JVM_CONSTANT_Methodref) {
	            // 4509014: If a class method name begins with '<', it must be "<init>".
	            assert(name != NULL, "method name in constant pool is null");
	            unsigned int name_len = name->utf8_length();
	            assert(name_len > 0, "bad method name");  // already verified as legal name
	            if (name->byte_at(0) == '<') {
	              if (name != vmSymbols::object_initializer_name()) {
	                classfile_parse_error(
	                  "Bad method name at constant pool index %u in class file %s",
	                  name_ref_index, CHECK_(nullHandle));
	              }
	            }
	          }
	        }
	        break;
	      }
	      case JVM_CONSTANT_MethodHandle: {
	        int ref_index = cp->method_handle_index_at(index);
	        int ref_kind  = cp->method_handle_ref_kind_at(index);
	        switch (ref_kind) {
	        case JVM_REF_invokeVirtual:
	        case JVM_REF_invokeStatic:
	        case JVM_REF_invokeSpecial:
	        case JVM_REF_newInvokeSpecial:
	          {
	            int name_and_type_ref_index = cp->name_and_type_ref_index_at(ref_index);
	            int name_ref_index = cp->name_ref_index_at(name_and_type_ref_index);
	            Symbol*  name = cp->symbol_at(name_ref_index);
	            if (ref_kind == JVM_REF_newInvokeSpecial) {
	              if (name != vmSymbols::object_initializer_name()) {
	                classfile_parse_error(
	                  "Bad constructor name at constant pool index %u in class file %s",
	                  name_ref_index, CHECK_(nullHandle));
	              }
	            } else {
	              if (name == vmSymbols::object_initializer_name()) {
	                classfile_parse_error(
	                  "Bad method name at constant pool index %u in class file %s",
	                  name_ref_index, CHECK_(nullHandle));
	              }
	            }
	          }
	          break;
	          // Other ref_kinds are already fully checked in previous pass.
	        }
	        break;
	      }
	      case JVM_CONSTANT_MethodType: {
	        Symbol* no_name = vmSymbols::type_name(); // place holder
	        Symbol*  signature = cp->method_type_signature_at(index);
	        verify_legal_method_signature(no_name, signature, CHECK_(nullHandle));
	        break;
	      }
	      case JVM_CONSTANT_Utf8: {
	        assert(cp->symbol_at(index)->refcount() != 0, "count corrupted");
	      }
	    }  // end of switch
	  }  // end of for

	  return cp;
}


