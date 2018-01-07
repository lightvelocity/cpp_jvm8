/*
 * top.hpp
 *
 *  Created on: 2018年1月5日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_UTILITIES_TOP_HPP_
#define SHARE_VM_UTILITIES_TOP_HPP_

#include "oops/oopsHierarchy.hpp"
#include "runtime/globals.hpp"
#include "utilities/debug.hpp"
#include "utilities/exceptions.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/macros.hpp"
#include "utilities/ostream.hpp"
#include "utilities/sizes.hpp"
#if INCLUDE_ALL_GCS
#include "gc_implementation/g1/g1_globals.hpp"
#endif // INCLUDE_ALL_GCS
#ifdef COMPILER1
#include "c1/c1_globals.hpp"
#endif
#ifdef COMPILER2
#include "opto/c2_globals.hpp"
#endif



#endif /* SHARE_VM_UTILITIES_TOP_HPP_ */
