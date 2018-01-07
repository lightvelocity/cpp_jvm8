/*
 * jni_x86.h
 *
 *  Created on: 2018年1月4日
 *      Author: Administrator
 */

// OK

#ifndef _JAVASOFT_JNI_MD_H_
#define _JAVASOFT_JNI_MD_H_

#define JNIEXPORT __declspec(dllexport)
#define JNIIMPORT __declspec(dllimport)
#define JNICALL   __stdcall

// FIXME ILP32 : int, long, pointer = 4B

typedef int         jint;  // 4B
typedef long long   jlong; // 8B
typedef signed char jbyte; // 1B

#endif /* !_JAVASOFT_JNI_MD_H_ */
