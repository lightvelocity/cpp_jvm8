/*
 * adlc.hpp
 *
 *  Created on: 2018年1月7日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_ADLC_ADLC_HPP_
#define SHARE_VM_ADLC_ADLC_HPP_

#undef assert
#define assert(cond, msg) { if(!(cond)) { fprintf(stderr, "assert fails %s %d: %s\n", __FILE__, __LINE__, msg); abort(); } }

#endif /* SHARE_VM_ADLC_ADLC_HPP_ */
