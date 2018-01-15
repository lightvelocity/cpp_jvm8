/*
 * os.hpp
 *
 *  Created on: 2018年1月7日
 *      Author: limaozhi
 */

#ifndef SHARE_VM_RUNTIME_OS_HPP_
#define SHARE_VM_RUNTIME_OS_HPP_

#include "jvmtifiles/jvmti.h"
#include "runtime/atomic.hpp"
#include "runtime/extendedPC.hpp"
#include "runtime/handles.hpp"
#include "utilities/top.hpp"
# include "jvm_windows.h"

class AgentLibrary;

// os defines the interface to operating system; this includes traditional
// OS services (time, I/O) as well as other functionality with system-
// dependent code.

typedef void (*dll_func)(...);

class Thread;
class JavaThread;
class Event;
class DLL;
class FileHandle;
template<class E> class GrowableArray;

// %%%%% Moved ThreadState, START_FN, OSThread to new osThread.hpp. -- Rose

// Platform-independent error return values from OS functions
enum OSReturn {
  OS_OK         =  0,        // Operation was successful
  OS_ERR        = -1,        // Operation failed
  OS_INTRPT     = -2,        // Operation was interrupted
  OS_TIMEOUT    = -3,        // Operation timed out
  OS_NOMEM      = -5,        // Operation failed for lack of memory
  OS_NORESOURCE = -6         // Operation failed for lack of nonmemory resource
};

enum ThreadPriority {        // JLS 20.20.1-3
  NoPriority       = -1,     // Initial non-priority value
  MinPriority      =  1,     // Minimum priority
  NormPriority     =  5,     // Normal (non-daemon) priority
  NearMaxPriority  =  9,     // High priority, used for VMThread
  MaxPriority      = 10,     // Highest priority, used for WatcherThread
                             // ensures that VMThread doesn't starve profiler
  CriticalPriority = 11      // Critical thread priority
};

// Executable parameter flag for os::commit_memory() and
// os::commit_memory_or_exit().
const bool ExecMem = true;

// Typedef for structured exception handling support
typedef void (*java_call_t)(JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread);


class os: AllStatic {
  friend class VMStructs;

 public:
  enum { page_sizes_max = 9 }; // Size of _page_sizes array (8 plus a sentinel)

 private:
  static OSThread*          _starting_thread;
  static address            _polling_page;
  static volatile int32_t * _mem_serialize_page;
  static uintptr_t          _serialize_page_mask;
 public:
  static size_t             _page_sizes[page_sizes_max];

 private:
  static void init_page_sizes(size_t default_page_size) {
    _page_sizes[0] = default_page_size;
    _page_sizes[1] = 0; // sentinel
  }

  static char*  pd_reserve_memory(size_t bytes, char* addr = 0,
                                size_t alignment_hint = 0);
   static char*  pd_attempt_reserve_memory_at(size_t bytes, char* addr);
   static void   pd_split_reserved_memory(char *base, size_t size,
                                       size_t split, bool realloc);
   static bool   pd_commit_memory(char* addr, size_t bytes, bool executable);
   static bool   pd_commit_memory(char* addr, size_t size, size_t alignment_hint,
                                  bool executable);
   // Same as pd_commit_memory() that either succeeds or calls
   // vm_exit_out_of_memory() with the specified mesg.
   static void   pd_commit_memory_or_exit(char* addr, size_t bytes,
                                          bool executable, const char* mesg);
   static void   pd_commit_memory_or_exit(char* addr, size_t size,
                                          size_t alignment_hint,
                                          bool executable, const char* mesg);
   static bool   pd_uncommit_memory(char* addr, size_t bytes);
   static bool   pd_release_memory(char* addr, size_t bytes);

   static char*  pd_map_memory(int fd, const char* file_name, size_t file_offset,
                            char *addr, size_t bytes, bool read_only = false,
                            bool allow_exec = false);
   static char*  pd_remap_memory(int fd, const char* file_name, size_t file_offset,
                              char *addr, size_t bytes, bool read_only,
                              bool allow_exec);
   static bool   pd_unmap_memory(char *addr, size_t bytes);
   static void   pd_free_memory(char *addr, size_t bytes, size_t alignment_hint);
   static void   pd_realign_memory(char *addr, size_t bytes, size_t alignment_hint);


 public:
  static void init(void);                      // Called before command line parsing
  static void init_before_ergo(void);          // Called after command line parsing
                                               // before VM ergonomics processing.
  static jint init_2(void);                    // Called after command line parsing
                                               // and VM ergonomics processing
  static void init_globals(void) {             // Called from init_globals() in init.cpp
    init_globals_ext();
  }
  static void init_3(void);                    // Called at the end of vm init

  // File names are case-insensitive on windows only
   // Override me as needed
   static int    file_name_strcmp(const char* s1, const char* s2);

   static bool getenv(const char* name, char* buffer, int len);
   static bool have_special_privileges();

   static jlong  javaTimeMillis();
   static jlong  javaTimeNanos();
   static void   javaTimeNanos_info(jvmtiTimerInfo *info_ptr);
   static void   run_periodic_checks();


   // Returns the elapsed time in seconds since the vm started.
   static double elapsedTime();

   // Returns real time in seconds since an arbitrary point
   // in the past.
   static bool getTimesSecs(double* process_real_time,
                            double* process_user_time,
                            double* process_system_time);

   // Interface to the performance counter
   static jlong elapsed_counter();
   static jlong elapsed_frequency();

   // The "virtual time" of a thread is the amount of time a thread has
   // actually run.  The first function indicates whether the OS supports
   // this functionality for the current thread, and if so:
   //   * the second enables vtime tracking (if that is required).
   //   * the third tells whether vtime is enabled.
   //   * the fourth returns the elapsed virtual time for the current
   //     thread.
   static bool supports_vtime();
   static bool enable_vtime();
   static bool vtime_enabled();
   static double elapsedVTime();

   // Return current local time in a string (YYYY-MM-DD HH:MM:SS).
   // It is MT safe, but not async-safe, as reading time zone
   // information may require a lock on some platforms.
   static char*      local_time_string(char *buf, size_t buflen);
   static struct tm* localtime_pd     (const time_t* clock, struct tm*  res);
   // Fill in buffer with current local time as an ISO-8601 string.
   // E.g., YYYY-MM-DDThh:mm:ss.mmm+zzzz.
   // Returns buffer, or NULL if it failed.
   static char* iso8601_time(char* buffer, size_t buffer_length);

   // Interface for detecting multiprocessor system
   static inline bool is_MP() {
     assert(_processor_count > 0, "invalid processor count");
     return _processor_count > 1 || AssumeMP;
   }
   static julong available_memory();
   static julong physical_memory();
   static bool has_allocatable_memory_limit(julong* limit);
   static bool is_server_class_machine();

   // number of CPUs
   static int processor_count() {
     return _processor_count;
   }
   static void set_processor_count(int count) { _processor_count = count; }

   // Returns the number of CPUs this process is currently allowed to run on.
   // Note that on some OSes this can change dynamically.
   static int active_processor_count();

   // Bind processes to processors.
   //     This is a two step procedure:
   //     first you generate a distribution of processes to processors,
   //     then you bind processes according to that distribution.
   // Compute a distribution for number of processes to processors.
   //    Stores the processor id's into the distribution array argument.
   //    Returns true if it worked, false if it didn't.
   static bool distribute_processes(uint length, uint* distribution);
   // Binds the current process to a processor.
   //    Returns true if it worked, false if it didn't.
   static bool bind_to_processor(uint processor_id);

   // Give a name to the current thread.
   static void set_native_thread_name(const char *name);

   // Interface for stack banging (predetect possible stack overflow for
   // exception processing)  There are guard pages, and above that shadow
   // pages for stack overflow checking.
   static bool uses_stack_guard_pages();
   static bool allocate_stack_guard_pages();
   static void bang_stack_shadow_pages();
   static bool stack_shadow_pages_available(Thread *thread, methodHandle method);

   // OS interface to Virtual Memory

   // Return the default page size.
   static int    vm_page_size();

   // Return the page size to use for a region of memory.  The min_pages argument
   // is a hint intended to limit fragmentation; it says the returned page size
   // should be <= region_max_size / min_pages.  Because min_pages is a hint,
   // this routine may return a size larger than region_max_size / min_pages.
   //
   // The current implementation ignores min_pages if a larger page size is an
   // exact multiple of both region_min_size and region_max_size.  This allows
   // larger pages to be used when doing so would not cause fragmentation; in
   // particular, a single page can be used when region_min_size ==
   // region_max_size == a supported page size.
   static size_t page_size_for_region(size_t region_min_size,
                                      size_t region_max_size,
                                      uint min_pages);
   // Return the largest page size that can be used
   static size_t max_page_size() {
     // The _page_sizes array is sorted in descending order.
     return _page_sizes[0];
   }

   // Methods for tracing page sizes returned by the above method; enabled by
   // TracePageSizes.  The region_{min,max}_size parameters should be the values
   // passed to page_size_for_region() and page_size should be the result of that
   // call.  The (optional) base and size parameters should come from the
   // ReservedSpace base() and size() methods.
   static void trace_page_sizes(const char* str, const size_t* page_sizes,
                                int count) PRODUCT_RETURN;
   static void trace_page_sizes(const char* str, const size_t region_min_size,
                                const size_t region_max_size,
                                const size_t page_size,
                                const char* base = NULL,
                                const size_t size = 0) PRODUCT_RETURN;

   static int    vm_allocation_granularity();
   static char*  reserve_memory(size_t bytes, char* addr = 0,
                                size_t alignment_hint = 0);
   static char*  reserve_memory(size_t bytes, char* addr,
                                size_t alignment_hint, MEMFLAGS flags);
   static char*  reserve_memory_aligned(size_t size, size_t alignment);
   static char*  attempt_reserve_memory_at(size_t bytes, char* addr);
   static void   split_reserved_memory(char *base, size_t size,
                                       size_t split, bool realloc);
   static bool   commit_memory(char* addr, size_t bytes, bool executable);
   static bool   commit_memory(char* addr, size_t size, size_t alignment_hint,
                               bool executable);
   // Same as commit_memory() that either succeeds or calls
   // vm_exit_out_of_memory() with the specified mesg.
   static void   commit_memory_or_exit(char* addr, size_t bytes,
                                       bool executable, const char* mesg);
   static void   commit_memory_or_exit(char* addr, size_t size,
                                       size_t alignment_hint,
                                       bool executable, const char* mesg);
   static bool   uncommit_memory(char* addr, size_t bytes);
   static bool   release_memory(char* addr, size_t bytes);

   enum ProtType { MEM_PROT_NONE, MEM_PROT_READ, MEM_PROT_RW, MEM_PROT_RWX };
   static bool   protect_memory(char* addr, size_t bytes, ProtType prot,
                                bool is_committed = true);

   static bool   guard_memory(char* addr, size_t bytes);
   static bool   unguard_memory(char* addr, size_t bytes);
   static bool   create_stack_guard_pages(char* addr, size_t bytes);
   static bool   pd_create_stack_guard_pages(char* addr, size_t bytes);
   static bool   remove_stack_guard_pages(char* addr, size_t bytes);

   static char*  map_memory(int fd, const char* file_name, size_t file_offset,
                            char *addr, size_t bytes, bool read_only = false,
                            bool allow_exec = false);
   static char*  remap_memory(int fd, const char* file_name, size_t file_offset,
                              char *addr, size_t bytes, bool read_only,
                              bool allow_exec);
   static bool   unmap_memory(char *addr, size_t bytes);
   static void   free_memory(char *addr, size_t bytes, size_t alignment_hint);
   static void   realign_memory(char *addr, size_t bytes, size_t alignment_hint);

   // NUMA-specific interface
   static bool   numa_has_static_binding();
   static bool   numa_has_group_homing();
   static void   numa_make_local(char *addr, size_t bytes, int lgrp_hint);
   static void   numa_make_global(char *addr, size_t bytes);
   static size_t numa_get_groups_num();
   static size_t numa_get_leaf_groups(int *ids, size_t size);
   static bool   numa_topology_changed();
   static int    numa_get_group_id();

   // Page manipulation
   struct page_info {
     size_t size;
     int lgrp_id;
   };
   static bool   get_page_info(char *start, page_info* info);
   static char*  scan_pages(char *start, char* end, page_info* page_expected, page_info* page_found);

   static char*  non_memory_address_word();
   // reserve, commit and pin the entire memory region
   static char*  reserve_memory_special(size_t size, size_t alignment,
                                        char* addr, bool executable);
   static bool   release_memory_special(char* addr, size_t bytes);
   static void   large_page_init();
   static size_t large_page_size();
   static bool   can_commit_large_page_memory();
   static bool   can_execute_large_page_memory();

   // OS interface to polling page
   static address get_polling_page()             { return _polling_page; }
   static void    set_polling_page(address page) { _polling_page = page; }
   static bool    is_poll_address(address addr)  { return addr >= _polling_page && addr < (_polling_page + os::vm_page_size()); }
   static void    make_polling_page_unreadable();
   static void    make_polling_page_readable();

   // Routines used to serialize the thread state without using membars
   static void    serialize_thread_states();

   // Since we write to the serialize page from every thread, we
   // want stores to be on unique cache lines whenever possible
   // in order to minimize CPU cross talk.  We pre-compute the
   // amount to shift the thread* to make this offset unique to
   // each thread.
   static int     get_serialize_page_shift_count() {
     return SerializePageShiftCount;
   }

   static void     set_serialize_page_mask(uintptr_t mask) {
     _serialize_page_mask = mask;
   }

   static unsigned int  get_serialize_page_mask() {
     return _serialize_page_mask;
   }

   static void    set_memory_serialize_page(address page);

   static address get_memory_serialize_page() {
     return (address)_mem_serialize_page;
   }

   static inline void write_memory_serialize_page(JavaThread *thread) {
     uintptr_t page_offset = ((uintptr_t)thread >>
                             get_serialize_page_shift_count()) &
                             get_serialize_page_mask();
     *(volatile int32_t *)((uintptr_t)_mem_serialize_page+page_offset) = 1;
   }

   static bool    is_memory_serialize_page(JavaThread *thread, address addr) {
     if (UseMembar) return false;
     // Previously this function calculated the exact address of this
     // thread's serialize page, and checked if the faulting address
     // was equal.  However, some platforms mask off faulting addresses
     // to the page size, so now we just check that the address is
     // within the page.  This makes the thread argument unnecessary,
     // but we retain the NULL check to preserve existing behaviour.
     if (thread == NULL) return false;
     address page = (address) _mem_serialize_page;
     return addr >= page && addr < (page + os::vm_page_size());
   }

   static void block_on_serialize_page_trap();





#endif /* SHARE_VM_RUNTIME_OS_HPP_ */
