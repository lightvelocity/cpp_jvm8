/*
 * constantTag.hpp
 *
 *  Created on: 2018年1月4日
 *      Author: Administrator
 */

#ifndef SHARE_VM_UTILITIES_CONSTANTTAG_HPP_
#define SHARE_VM_UTILITIES_CONSTANTTAG_HPP_

enum {
  // See jvm.h for shared JVM_CONSTANT_XXX tags
  // NOTE: replicated in SA in vm/agent/sun/jvm/hotspot/utilities/ConstantTag.java
  // Hotspot specific tags
  JVM_CONSTANT_Invalid                  = 0,    // For bad value initialization
  JVM_CONSTANT_InternalMin              = 100,  // First implementation tag (aside from bad value of course)
  JVM_CONSTANT_UnresolvedClass          = 100,  // Temporary tag until actual use
  JVM_CONSTANT_ClassIndex               = 101,  // Temporary tag while constructing constant pool
  JVM_CONSTANT_StringIndex              = 102,  // Temporary tag while constructing constant pool
  JVM_CONSTANT_UnresolvedClassInError   = 103,  // Error tag due to resolution error
  JVM_CONSTANT_MethodHandleInError      = 104,  // Error tag due to resolution error
  JVM_CONSTANT_MethodTypeInError        = 105,  // Error tag due to resolution error
  JVM_CONSTANT_InternalMax              = 105   // Last implementation tag
};





#endif /* SHARE_VM_UTILITIES_CONSTANTTAG_HPP_ */
