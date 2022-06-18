// The MIT License (MIT)
//
// Copyright(c) 2021, Damien Feneyrou <dfeneyrou@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

// Palanteer is a 3-parts solution to C++ software quality improvement.
//   This header is one part: the C++ single-header instrumentation library.

// The text descriptions in this header are partial and do not aim at replacing
//  the official documentation nor at presenting properly this tool.
// Moreover, the APIs are mixed with the implementation (single header) so not easy
// to read.
// If you start with Palanteer, please read the documentation first (and not this file).
// The documentation covers:
//  - the exhaustive presentation of the Palanteer suite
//  - the complete instrumentation API
//  - the complete scripting API
//  - the associated viewer


//-----------------------------------------------------------------------------
// Library configuration
//-----------------------------------------------------------------------------

// Configuration in the section below is applicable only in the file which
// implements the Palanteer service. You shall "implement" it exactly once.
// Ex:
//    #define PL_IMPLEMENTATION 1
//    #include "palanteer.h"

#if PL_IMPLEMENTATION==1 || PL_EXPORT==1

// Enable installing some signal (ABRT, FPE, ILL, SEGV, INT, TERM) handlers
//  If enabled, last collected events before crash will be flushed, which helps further investigation.
//  Default is enabled.
#ifndef PL_IMPL_CATCH_SIGNALS
#define PL_IMPL_CATCH_SIGNALS 1
#endif

// Default collection buffer size. Events are written in these double bank buffers,
//  and are regularly harvested by a dedicated internal thread.
//  Too small a size and your threads may have to busy wait until the buffer has free space
//  Too big a size and you waste memory
#ifndef PL_IMPL_COLLECTION_BUFFER_BYTE_QTY
#define PL_IMPL_COLLECTION_BUFFER_BYTE_QTY 5000000
#endif

// Default quantity of pre-allocated dynamic strings per collection cycle.
//  Note that threads will busy-wait if pool is empty.
#ifndef PL_IMPL_DYN_STRING_QTY
#define PL_IMPL_DYN_STRING_QTY 1024
#endif

// The maximum byte size of a received remote request (at least 64 bytes)
// Such buffer will be allocated twice (once for remote request reception, once for CLI parameter work buffer)
#ifndef PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY
#define PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY (8*1024)
#endif

// The maximum byte size of a remote CLI response (at least 64 bytes)
// Such buffer will be allocated 3 times (CLI response building, generic command response and one for lock free sending)
#ifndef PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY
#define PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY (8*1024)
#endif

// The byte size of the buffer to send a batch of newly seen strings (at least 128 bytes or the size of the longest string), allocated once
#ifndef PL_IMPL_STRING_BUFFER_BYTE_QTY
#define PL_IMPL_STRING_BUFFER_BYTE_QTY (8*1024)
#endif

// The expected known string quantity which defines the initial allocation of the lookup hash->string.
// If exceeded, a reallocation occurs (with rehashing), which is sometimes not desired in memory constrained environments.
#ifndef PL_IMPL_MAX_EXPECTED_STRING_QTY
#define PL_IMPL_MAX_EXPECTED_STRING_QTY 4096
#endif

// The maximum CLI quantity in the system
// The size of 1 CLI is (64+16*PL_IMPL_CLI_MAX_PARAM_QTY) bytes plus the declaration strings
// The default values give a total of 24 KB (without the declaration strings nor the code of the handlers)
#ifndef PL_IMPL_MAX_CLI_QTY
#define PL_IMPL_MAX_CLI_QTY 128
#endif

// The maximum quantity of parameters for a CLI
#ifndef PL_IMPL_CLI_MAX_PARAM_QTY
#define PL_IMPL_CLI_MAX_PARAM_QTY 8
#endif

// Under Windows (else not applicable), sockets require a global initialization (WSA)
//  Set this variable to 0  or comment below to disabled this management inside Palanteer
#ifndef PL_IMPL_MANAGE_WINDOWS_SOCKET
#define PL_IMPL_MANAGE_WINDOWS_SOCKET 1
#endif

// Enable the collection of the kernel scheduler thread context switches
//  Default is enabled for Linux and Windows
//  Note that effective activation requires privileged rights (run as root or administrator)
#ifndef PL_IMPL_CONTEXT_SWITCH
#define PL_IMPL_CONTEXT_SWITCH 1
#endif

// Dynamic memory collection by overloading operator new & delete  is enabled by default
//  This may create some issues if your program also overload them.
#ifndef PL_IMPL_OVERLOAD_NEW_DELETE
#define PL_IMPL_OVERLOAD_NEW_DELETE 1
#endif

// Stacktrace logging when a crash occurs
//   On linux, stack trace logging is disabled by default as it requires libunwind.so (stack unwinding) and libdw.so from elfutils (elf and DWARF infos reading)
//    (apt install libunwind-dev libdw-dev)
//   On Windows, stacktrace logging is enabled by default as base system libraries cover the requirements.
//   Note 1: The executable shall contain debug information. If no debug information is present, the stacktrace will just be a list of addresses.
//          If the non-stripped executable version is available, they can be manually decoded with 'addr2line' or equivalent tool.
//   Note 2: The "external strings" feature has no effect on the stacktrace logging, as dynamic strings are used.
#if defined(_WIN32) && !defined(PL_IMPL_STACKTRACE)
#define PL_IMPL_STACKTRACE 1
#endif

// Stacktrace logging function. Default is a built-in handler for Linux/Windows OS.
#ifndef PL_IMPL_STACKTRACE_FUNC
#if PL_IMPL_STACKTRACE==1
#define PL_IMPL_STACKTRACE_FUNC() plPriv::crashLogStackTrace()
#else
#define PL_IMPL_STACKTRACE_FUNC()
#endif // if defined(PL_IMPL_STACKTRACE)
#endif // ifndef PL_IMPL_STACKTRACE

// Display the stacktrace on console using terminal colors. Enabled by default.
#ifndef PL_IMPL_CONSOLE_COLOR
#define PL_IMPL_CONSOLE_COLOR 1
#endif

// Exit function when a crash occurs, called after logging the crash in Palanteer, flushing the recording and restoring signals
// Default is a call to abort().
#ifndef PL_IMPL_CRASH_EXIT_FUNC
#define PL_IMPL_CRASH_EXIT_FUNC() quick_exit(1)
#endif

// Print error function (assertions, signals).
//  On Windows, shall probably be redirected on a MessageBox
#ifndef PL_IMPL_PRINT_STDERR
#define PL_IMPL_PRINT_STDERR(msg, isCrash, isLastFromCrash) fprintf(stderr, "%s", msg)
#endif

// Enable the automatic instrumentation of functions (only GCC). Disabled by default.
// In addition to setting this Palanteer flag, it is required to add some compiler flags for all files to be auto-instrumented:
//   * -g     (or equivalent to get debug symbols. This is independent on the optimization level)
//   * -finstrument-functions
//   * -finstrument-functions-exclude-file-list=palanteer.h,include/c++,/bits/   (valid for Debian 11)
// Note that palanteer.h *must* be excluded to avoid "Larsen" effects. Same for some inlined standard library functions used when logging.
// The symptom of not properly respecting these constraints is a "segmentation fault" due to a stack overflow.
// The exclusion list above works at least for a Debian 11 distribution.
// Some more points shall be taken into account:
//  * intensive instrumentation may alter the observation of the performances in real conditions
//  * inlined functions are also instrumented, even if no more present
//    It is thus recommended to exclude the STL (see exclude file or function list flags, the "/include/c++" part), as it both
//      floods events due to its code structure, and uses unreadable names
//  * the mechanism to resolve the function names is the use of external strings (not done at run-time for efficiency reasons).
//     - The external string tool can be used to extract also the ELF debug symbols, independently of the use of the external string feature.
//       ex: ./tools/stringLookupGenerator.py --exe "C++ example" ./bin/testprogram > ./bin/testprogram.txt
//     - The viewer shall be configured to get the path of the external string file and update it at recording time.
//     - This lookup shall be generated during the build after the generated ELF, as it is tighly linked to it
//
// Warning: Auto instrumentation allows a quick and easy glance at the program behavior. It is great to understand the dynamic of a program.
// However, the measured timings shall be considered with caution due to the potentially heavy and non-optimal logging altering
//   the dynamic behavior of the program.
#ifndef PL_IMPL_AUTO_INSTRUMENT
#define PL_IMPL_AUTO_INSTRUMENT 0
#endif

// In case PL_IMPL_AUTO_INSTRUMENT==1, this flag enables the correction of the base address of function selected at runtime (PIC or PIE)
// The default value is 1, as 64 bits ELF have PIE enabled by default these days.
// On 32 bits ELF, this is usually not the case and this flag shall be set to zero (absolute function addresses)
#ifndef PL_IMPL_AUTO_INSTRUMENT_PIC
#define PL_IMPL_AUTO_INSTRUMENT_PIC 1
#endif

#endif // ifdef PL_IMPLEMENTATION


// Configuration in the section below is applicable in all files where Palanteer is used
// So declare the customisations __in all files__ (not scalable), encapsulate this file (best), or modify this file.

// A "compact" software implies the following constraints:
//  - not more than 65535 unique strings
//  - no logging of 64bit types (double, u64...), they are converted to 32 bits equivalent
//  - Forced PL_SHORT_STRING_HASH=1 (32 bits string hash) is acceptable
//  - Forced PL_SHORT_DATE=1 (32 bits clock value) is acceptable
// The benefit is a 50% reduction of the network bandwidth or of the .pltraw size in case of disk storage
#ifndef PL_COMPACT_MODEL
#define PL_COMPACT_MODEL 0
#endif

// If compact model in use, force short string hash and short date (prerequisites)
#if PL_COMPACT_MODEL==1
#ifdef PL_SHORT_STRING_HASH
#undef PL_SHORT_STRING_HASH
#endif
#define PL_SHORT_STRING_HASH 1
#ifdef PL_SHORT_DATE
#undef PL_SHORT_DATE
#endif
#define PL_SHORT_DATE 1
#endif

// The "external string" mode is very different from the standard one:
//   - no Palanteer-related static string is present in the program nor in the record.
//      . it obfuscates the instrumentation (compile time process)
//      . the program size is reduced along with its .rodata section
//      . Note: dynamic strings are still processed normally and stay visible in the program and the record
//   - BUT in order to view the record, you have to provide an additional file to "resolve" these strings
//      . basically a lookup hashId->string content
//   - The string lookup file can be generated by running the provided tool 'tools/stringLookupGenerator.py' on your sources.
//      . through a basic parsing, it extracts all Palanteer related strings, compute their hash, and
//        store the hashId->string couple in the lookup file
//      . this (fast) process shall typically be a part of your build system
#ifndef PL_EXTERNAL_STRINGS
#define PL_EXTERNAL_STRINGS 0
#endif

// Maximum dynamic string sizes. Required as their storage is preallocated. Larger strings will be truncated
//  Note: stacktraces are sent as dynamic strings, so strings shall be large enough
#ifndef PL_DYN_STRING_MAX_SIZE
#define PL_DYN_STRING_MAX_SIZE 512
#endif

// By default, 64 bits string hashes are used to ensures virtually no collision.
//  Declare __in all files__ or set here this variable to 1 to use 32 bits string hashes instead
//  The only 'gain' is for 32 bits systems and only when using dynamic strings (that you should avoid)
//   or recording context switches: run-time computation speed will be better
//  Note that there is no reduction on storage size
#ifndef PL_SHORT_STRING_HASH
#define PL_SHORT_STRING_HASH 0
#endif

// Declare __in all files__ or set here this variable to 1 to reduce the code size of the
//  assertions, at the price of less information displayed
#ifndef PL_SIMPLE_ASSERT
#define PL_SIMPLE_ASSERT 0
#endif

// Enables the support of "virtual" threads (like fibers or in DES simulators). Disabled by default.
// To make it work, the following actions are required:
// 1) the framework hook on virtual thread creation should call plDeclareVirtualThread(...). Example of name: "Fibers/Fiber 14"
// 2) the framework hook on switching virtual threads shall call plAttachVirtualThread(...) and plDetachVirtualThread(...)
// See the documentation for details. Not enabled by default due to some extra memory allocations.
#ifndef PL_VIRTUAL_THREADS
#define PL_VIRTUAL_THREADS 0
#endif

// [Platform specific - or just choice]
// A short date is a date coded only on 32 bits (instead of 64 bits).
// This flag does 2 things:
//  - it advertised the 32 bits wrap to the server
//  - it switches the return type of the "get clock" function to uint32_t
// Use this flag is your high resolution clock is 32 bits, so that the wrap will be handled automatically
// There is no space or bandwidth gain, unless you use the PL_COMPACT_MODEL flag, which implies this one
#ifndef PL_SHORT_DATE
#define PL_SHORT_DATE 0
#endif

// [Platform specific]
// Clock function. By default, a predefined high performance clock is used (Windows / Linux)
//  You can set your own by defining PL_GET_CLOCK_TICK_FUNC.
//  Its prototype is without parameter and returning a monotonic unsigned integer of size
//  uint32_t if PL_SHORT_DATE==1 else uint64_t. See getClockTick() definition below, as an example.
// Note: Context switch collection may not work with user-defined clocks if clocks are not matching
#ifndef PL_GET_CLOCK_TICK_FUNC
#define PL_GET_CLOCK_TICK_FUNC() plPriv::getClockTick()
#endif

// [Platform specific]
// This function is used only in case of PL_SHORT_DATE=1 with multi-stream and returns a non wrapping global system time
// It is used once at connection time to provide to the server the ordering of origin dates per stream
// This global system time
//  - shall be coded on a u64 and be in nanosecond unit. The origin does not matter as long as all dates are positive.
//  - shall be consistent across all streams ("global")
//  - precision shall be at least 1/4 of the date wrapping period
#ifndef PL_GET_SYSTEM_CLOCK_NS
#define PL_GET_SYSTEM_CLOCK_NS() std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#endif

// [Platform specific]
// System thread ID getter
// Implemented for both Windows and Linux
#ifndef PL_GET_SYS_THREAD_ID
#if defined(__unix__)
#define PL_GET_SYS_THREAD_ID() syscall(SYS_gettid)
#endif
#if defined(_WIN32)
#define PL_GET_SYS_THREAD_ID() GetCurrentThreadId()
#endif
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <cstdint> // For sized types like uintXXX_t
#include <cstddef>

#if USE_PL==1

// Windows base header (hard to avoid this include...)
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers. If it is a problem, just comment it
#include <windows.h>
#endif

#include <cstddef>   // For size_t
#include <cstdlib>   // For abort(), quick_exit...
#include <cinttypes> // For platform independent printf/scanf
#include <csignal> // For raising signals in crash handler

#ifdef __unix__
#include <unistd.h>  // For getpid() in the display of the gdb helper
#endif
#ifdef _WIN32
#include <processthreadsapi.h>  // For GetCurrentProcessId()
#endif
#endif // if USE_PL==1

#if USE_PL==1
#include <cstdio>  // For snprintf etc...
#include <cstdarg> // For va_list used in logs
#include <atomic>  // Lock-free thread safety is done with atomics
#include <thread>  // For this_thread::yield()
#endif

#if (USE_PL==1 && (PL_NOCONTROL==0 || PL_NOEVENT==0)) || PL_EXPORT==1
#include <chrono>  // For time_since_epoch()
#include <cassert> // For static_assert()
#include <cstring> // For string copy (dynamic strings)
#if defined(__unix__)
#include <unistd.h>     // For syscall(SYS_gettid)
#include <sys/syscall.h>
#endif
#ifdef _WIN32
#include <processthreadsapi.h> // For GetCurrentThreadId() in the core logging
#endif
#endif // if (USE_PL==1 && (PL_NOCONTROL==0 || PL_NOEVENT==0)) || PL_EXPORT==1

#if (USE_PL==1 && PL_NOCONTROL==0) || PL_EXPORT==1
#include <cstdarg>  // For variable argument in the CLI response creation
#endif // if if (USE_PL==1 && PL_NOCONTROL==0) || PL_EXPORT==1


//-----------------------------------------------------------------------------
// Public assertions interface
//-----------------------------------------------------------------------------

#if USE_PL==1 && PL_NOASSERT==0

// Public assertion macros. Default is full information (not "simple")
#if PL_SIMPLE_ASSERT==0
// These assertions allow to easily dump the values which contribute to the condition.
// Ex: plAssert(a<b);                                         // Standard form
//     plAssert(a<b, "A shall always be less than b");        // Documented form
//     plAssert(a<b, a, b);                                   // Extended form showing the values of 'a' and 'b' when assertion is failed
//     plAssert(a<b, "A shall always be less than b", a, b);  // Displays up to 9 parameters... Ought to be enough for anybody (tm)
#if PL_EXTERNAL_STRINGS==0
#define plAssert(cond_,...) if(PL_UNLIKELY(!(cond_)))                  \
        plPriv::failedAssert(PL_FILENAME, __LINE__, PL_ASSERT_FUNCTION, #cond_, PL_PRIV_CALL_OVERLOAD(PL_PRIV_ASSERT_PARAM,"",##__VA_ARGS__) )
#else // if PL_EXTERNAL_STRINGS==0
#define plAssert(cond_,...) if(PL_UNLIKELY(!(cond_)))                  \
        plPriv::failedAssertEs(PL_STRINGHASH(PL_BASEFILENAME), __LINE__, PL_STRINGHASH(#cond_), PL_PRIV_CALL_OVERLOAD(PL_PRIV_ASSERT_PARAM_ES,"",##__VA_ARGS__) )
#endif // if PL_EXTERNAL_STRINGS==0

#else // if PL_SIMPLE_ASSERT==0
// These lighter assertions enforce the standard behavior, ignoring all additional information after the condition
// The only benefit is the reduction of the code size due to the non usage of variadic templates at the price of less informative assertions
// Ex: plAssert(a<b, a, b)  is equivalent of plAssert(a<b);
#if PL_EXTERNAL_STRINGS==0
#define plAssert(cond_,...) if(PL_UNLIKELY(!(cond_)))                   \
        plPriv::failedAssertSimple(PL_FILENAME, __LINE__, PL_ASSERT_FUNCTION, #cond_)
#else // if PL_EXTERNAL_STRINGS==0
// Note: The function name is constexpr only on recent compiler (gcc 8.3 does not support it) so not included in this external string assertion variant
#define plAssert(cond_,...) if(PL_UNLIKELY(!(cond_))) \
        plPriv::failedAssertSimpleEs(PL_STRINGHASH(PL_BASEFILENAME), __LINE__, PL_STRINGHASH(#cond_))
#endif // if PL_EXTERNAL_STRINGS==0

#endif // if PL_SIMPLE_ASSERT==0

// Same but per custom group
#define plgAssert(group_,cond_,...) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plAssert(cond_, ##__VA_ARGS__),do {} while(0))

#else // if USE_PL==1 && PL_NOASSERT==0

// Deactivated case
#define plAssert(cond_, ...)          PL_UNUSED(cond_)
#define plgAssert(group_, cond_, ...) PL_UNUSED(cond_)

#endif // if USE_PL==1 && PL_NOASSERT==0



//-----------------------------------------------------------------------------
// Public event service interface
//-----------------------------------------------------------------------------

// Collection statistic structure
struct plStats {
    uint32_t collectBufferSizeByteQty;     // Configured collection buffer size
    uint32_t collectBufferMaxUsageByteQty; // Maximum used size in the collection buffer
    uint32_t collectDynStringQty;          // Configured dynamic string qty
    uint32_t collectDynStringMaxUsageQty;  // Maximum used dynamic string qty
    uint32_t sentBufferQty;                // Buffer qty sent to the server
    uint32_t sentByteQty;                  // Byte qty sent to the server
    uint32_t sentEventQty;                 // Event qty sent to server
    uint32_t sentStringQty;                // Unique string qty sent to server
};

enum plMode { PL_MODE_CONNECTED, PL_MODE_STORE_IN_FILE, PL_MODE_INACTIVE};

enum plLogLevel {
    PL_LOG_LEVEL_ALL   = 0,  // Just for clarity
    PL_LOG_LEVEL_DEBUG = 0,
    PL_LOG_LEVEL_INFO  = 1,
    PL_LOG_LEVEL_WARN  = 2,
    PL_LOG_LEVEL_ERROR = 3,
    PL_LOG_LEVEL_NONE  = 4
};


// If USE_PL==0 or not defined, the full Palanteer service is disabled (events, remote control, palanteer assertions)
#if USE_PL==1

// This configuration is considered only when plInitAndStart is called, and if mode is PL_MODE_STORE_IN_FILE
void plSetFilename(const char* filename);

// This configuration is considered only when plInitAndStart is called, and if mode is PL_MODE_CONNECTED
void plSetServer(const char* serverAddr, int serverPort);

// This function dynamically sets the minimum level of the log to be recorded. Default is "debug" (i.e. all logs).
void plSetLogLevelRecord(plLogLevel level);

// This function dynamically sets the minimum level of the log to be displayed on consoled. Default is "warn".
void plSetLogLevelConsole(plLogLevel level);

// The service shall be initialized once, before any usage of event logging.
//  The 'appName' is the application name seen by the server
//  Even if the mode is PL_MODE_INACTIVE, it is expected to call this function for the installation of signals handlers
//   and the initialization of symbol decoding for the stacktrace (crash case)
//  The build name is optional, it is your identifier of the program version, or anything else.
//  The serverConnectionTimeoutMsec is the timeout in millisecond for the server connection, before giving up (only for the PL_MODE_CONNECTED case)
//   A negative value means waiting forever. Default value is no wait.
void plInitAndStart(const char* appName, plMode mode=PL_MODE_CONNECTED,
                    const char* buildName=0, int serverConnectionTimeoutMsec=0);

// This function stops and uninitializes the event logging service (typically before exiting the program).
void plStopAndUninit(void);

// This getter returns statistics on the collection process (can be called at any moment)
plStats plGetStats(void);

// This function is specific to the support of the virtual threads
// It should be called once at virtual thread creation
// The externalVirtualThreadId can have any value but shall uniquely identify the virtual thread.
void plDeclareVirtualThread(uint32_t externalVirtualThreadId, const char* format, ...);

// This function is specific to the support of the virtual threads
// It must be called when attaching a virtual threads to the current worker thread.
// The worker thread shall currently not run any virtual thread ('detach' shall be called between 2 virtual threads)
bool plAttachVirtualThread(uint32_t externalVirtualThreadId);

// This function is specific to the support of the virtual threads
// It must be called when a virtual threads is detached from the current worker thread.
// The flag `isSuspended` indicates if the virtual thread is suspended or simply finished.
// In doubt, set it to false.
void plDetachVirtualThread(bool isSuspended);

#else // if USE_PL==1

#define plSetFilename(filename_) PL_UNUSED(filename_)
#define plSetServer(serverAddr_, serverPort_)
#define plSetLogLevelRecord(level) PL_UNUSED(level)
#define plSetLogLevelConsole(level) PL_UNUSED(level)
#define plInitAndStart(appName_, ...) PL_UNUSED(appName_)
#define plStopAndUninit()
#define plGetStats() plStats { 0, 0, 0, 0, 0, 0, 0, 0 }
#define plDeclareVirtualThread(externalVirtualThreadId_, format_, ...)  do { PL_UNUSED(externalVirtualThreadId_); PL_UNUSED(format_); } while(0)
#define plDetachVirtualThread(isSuspended_) PL_UNUSED(isSuspended_);
#define plAttachVirtualThread(externalVirtualThreadId_) false && (externalVirtualThreadId_)

#endif


// If PL_NOEVENT==1, the event service is removed at compile-time
// Note: the remote control and the assertion service are independent
#if USE_PL==1 && PL_NOEVENT==0

// Checks if the service is currently enabled
#define plIsEnabled()        PL_IS_ENABLED_()
#define plgIsEnabled(group_) (PLG_IS_COMPILE_TIME_ENABLED_(group_) && PL_IS_ENABLED_())

// Sets the name of the current thread. Only first call is taken into account
// Calls can happen before service is started and are persistent across multiple starts. This eases the "on demand" profiling
#define plDeclareThread(name_) plPriv::declareThread(PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:(name_), PL_STORE_COLLECT_CASE_, false);
#define plDeclareThreadDyn(format_,...)                                 \
    do { char name_[PL_DYN_STRING_MAX_SIZE];                            \
        plPriv::formatDynString(name_, format_,##__VA_ARGS__);          \
        plPriv::declareThread(0, name_, PL_STORE_COLLECT_CASE_, true); \
    } while(0)
#define plgDeclareThread(group_, name_)    PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plDeclareThread(name_),do {} while(0))
#define plgDeclareThreadDyn(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plDeclareThreadDyn(name_),do {} while(0))

// Closes itself automatically at end of scope
#define plScope(name_)             PL_SCOPE_(name_, __LINE__)
#define plgScope(group_, name_)    PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plScope(name_),do {} while(0))
#define plScopeDyn(name_)          PL_SCOPE_DYN_(name_, __LINE__)
#define plgScopeDyn(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plScopeDyn(name_),do {} while(0))

// Closes itself automatically at end of scope
// Note: plFunction() works only in most recent compiler (gcc>=9.1, clang>=3.6). Before that, __func__ was not constexpr
// Note2: plFunction() is not compatible with the external string (the provided script cannot recover the function names from the call in the source)
#define plFunction()           PL_SCOPE_(__func__, __LINE__)
#define plgFunction(group_)    PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plFunction(),do {} while(0))
#define plFunctionDyn()        PL_SCOPE_DYN_(__func__, __LINE__)
#define plgFunctionDyn(group_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plFunctionDyn(),do {} while(0))

// Tracks the scope between begin and end calls (which shall have exactly the same name)
#define plBegin(name_)                                                  \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plEnd(name_)                                                    \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgBegin(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plBegin(name_),do {} while(0))
#define plgEnd(group_, name_)   PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plEnd(name_),do {} while(0))
#define plBeginDyn(name_)                                               \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRawDynName(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                       PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plEndDyn(name_)                                                 \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRawDynName(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                       PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgBeginDyn(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plBeginDyn(name_),do {} while(0))
#define plgEndDyn(group_, name_)   PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plEndDyn(name_),do {} while(0))

// Log a numeric event with a name and a value. Optionally, the words after "##" in the name is the unit (for grouping curves)
#define plData(name_, value_)                                          \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogData(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, PL_STORE_COLLECT_CASE_, value_); \
    } while(0)
#define plgData(group_, name_, value_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plData(name_, value_), do {} while(0))

// Log a static string event with a name. Optionally, the words after "##" in the name is the unit (for grouping curves)
#define plText(name_, msg_)          plData(name_, plMakeString(msg_))
#define plgText(group_, name_, msg_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plText(name_, msg_),do {} while(0))

// Log a numeric event with the name and the value of a numeric variable.
#define plVar(...)         do { if(PL_IS_ENABLED_()) { PL_PRIV_CALL_OVERLOAD(PL_PRIV_VARS_PARAM,##__VA_ARGS__) } } while(0)
#define plgVar(group_,...) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plVar(__VA_ARGS__),do {} while(0))

// Log a batch of numeric events with plVar_() for each listed variable
#define plVar_(value_) plData(#value_, value_)
#define PL_PRIV_VARS_PARAM0() static_assert(0, "plVar requires at least one parameter")
#define PL_PRIV_VARS_PARAM1(v1) plVar_(v1);
#define PL_PRIV_VARS_PARAM2(v1, v2)  plVar_(v1); plVar_(v2);
#define PL_PRIV_VARS_PARAM3(v1, v2, v3)  plVar_(v1); plVar_(v2); plVar_(v3);
#define PL_PRIV_VARS_PARAM4(v1, v2, v3, v4)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4);
#define PL_PRIV_VARS_PARAM5(v1, v2, v3, v4, v5)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4); plVar_(v5);
#define PL_PRIV_VARS_PARAM6(v1, v2, v3, v4, v5, v6)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4); plVar_(v5); plVar_(v6);
#define PL_PRIV_VARS_PARAM7(v1, v2, v3, v4, v5, v6, v7)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4); plVar_(v5); plVar_(v6); plVar_(v7);
#define PL_PRIV_VARS_PARAM8(v1, v2, v3, v4, v5, v6, v7, v8)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4); plVar_(v5); plVar_(v6); plVar_(v7); plVar_(v8);
#define PL_PRIV_VARS_PARAM9(v1, v2, v3, v4, v5, v6, v7, v8, v9)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4); plVar_(v5); plVar_(v6); plVar_(v7); plVar_(v8); plVar_(v9);
#define PL_PRIV_VARS_PARAM10(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)  plVar_(v1); plVar_(v2); plVar_(v3); plVar_(v4); plVar_(v5); plVar_(v6); plVar_(v7); plVar_(v8); plVar_(v9); plVar_(v10);

// Log "logs"
#define plLogDebug(category_, format_, ...)                             \
    do {                                                                \
        (void)sizeof(printf(format_, ##__VA_ARGS__));  /* Check consistency of printf parameters. "-Werror=format" is recommended */ \
        plPriv::eventLogLog(PL_STRINGHASH(format_), PL_STRINGHASH(category_), PL_EXTERNAL_STRINGS?0:(format_), PL_EXTERNAL_STRINGS?0:(category_), \
                            PL_LOG_LEVEL_DEBUG,##__VA_ARGS__);          \
    } while(0)
#define plgLogDebug(group_, category_, format_, ...) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLogDebug(category_, format_,##__VA_ARGS__),do {} while(0))

#define plLogInfo(category_, format_, ...)                              \
    do {                                                                \
        (void)sizeof(printf(format_, ##__VA_ARGS__));  /* Check consistency of printf parameters. "-Werror=format" is recommended */ \
        plPriv::eventLogLog(PL_STRINGHASH(format_), PL_STRINGHASH(category_), PL_EXTERNAL_STRINGS?0:(format_), PL_EXTERNAL_STRINGS?0:(category_), \
                            PL_LOG_LEVEL_INFO,##__VA_ARGS__);           \
    } while(0)
#define plgLogInfo(group_, category_, format_, ...) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLogInfo(category_, format_,##__VA_ARGS__),do {} while(0))

#define plLogWarn(category_, format_, ...)     \
    do {                                                                \
        (void)sizeof(printf(format_, ##__VA_ARGS__));  /* Check consistency of printf parameters. "-Werror=format" is recommended */ \
        plPriv::eventLogLog(PL_STRINGHASH(format_), PL_STRINGHASH(category_), PL_EXTERNAL_STRINGS?0:(format_), PL_EXTERNAL_STRINGS?0:(category_), \
                            PL_LOG_LEVEL_WARN,##__VA_ARGS__);           \
    } while(0)
#define plgLogWarn(group_, category_, format_, ...) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLogWarn(category_, format_,##__VA_ARGS__),do {} while(0))

#define plLogError(category_, format_, ...)      \
    do {                                                                \
        (void)sizeof(printf(format_, ##__VA_ARGS__));  /* Check consistency of printf parameters. "-Werror=format" is recommended */ \
        plPriv::eventLogLog(PL_STRINGHASH(format_), PL_STRINGHASH(category_), PL_EXTERNAL_STRINGS?0:(format_), PL_EXTERNAL_STRINGS?0:(category_), \
                            PL_LOG_LEVEL_ERROR,##__VA_ARGS__);          \
    } while(0)
#define plgLogError(group_, category_, format_, ...) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLogError(category_, format_,##__VA_ARGS__),do {} while(0))


// DEPRECATED plMarker API: use plLog<level> instead. Will be removed in a future release
// Current implementation is a wrapper on plLogWarn, but the compatibility is not 100%:
//   If your msg_ string is not static, you have to replace plMarkerDyn("category", dynamicString) with plMarkerDyn("category", "%s", dynamicString)
#define plMarker(category_, msg_)                   plLogWarn(category_, msg_)
#define plgMarker(group_, category_, msg_)          plgLogWarn(group_, category_, msg_)
#define plMarkerDyn(category_, msg_, ...)           plLogWarn(category_, msg_,##__VA_ARGS__)
#define plgMarkerDyn(group_, category_, msg_ ,...)  plgLogWarn(group_, category_, msg_,##__VA_ARGS__)


// Mutex tracking macros (non re-entrant mutexes only)

// Start waiting for a lock
// Shall be placed just before the OS waiting call
// Call to plLockState must be called to stop the waiting
#define plLockWait(name_)                                               \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_LOCK_WAIT, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockWait(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockWait(name_),do {} while(0))
#define plLockWaitDyn(name_)                                           \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRawDynName(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, (name_), __LINE__, \
                                       PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_LOCK_WAIT, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockWaitDyn(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockWaitDyn(name_),do {} while(0))
#define plLockWaitStatic(pl_string_name_)                               \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), (pl_string_name_).hash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:((pl_string_name_).value), __LINE__, \
                                PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_LOCK_WAIT, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockWaitStatic(group_, pl_string_name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockWaitStatic(pl_string_name_),do {} while(0))

// Set the lock state
// Shall be called just after the "wait for lock" to stop the waiting phase.
// Shall be placed just before any "unlock" call (to prevent any race condition in traces)
#define plLockState(name_, state_)                                      \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                PL_STORE_COLLECT_CASE_, (state_)? PL_FLAG_TYPE_LOCK_ACQUIRED : PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockState(group_, name_, state_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockState(name_, state_),do {} while(0))
#define plLockStateDyn(name_, state_)                                   \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRawDynName(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, name_, __LINE__, \
                                       PL_STORE_COLLECT_CASE_, (state_)? PL_FLAG_TYPE_LOCK_ACQUIRED : PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockStateDyn(group_, name_, state_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockStateDyn(name_, state_),do {} while(0))
#define plLockStateStatic(pl_string_name_, state_)                      \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), (pl_string_name_).hash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:((pl_string_name_).value), __LINE__, \
                                PL_STORE_COLLECT_CASE_, (state_)? PL_FLAG_TYPE_LOCK_ACQUIRED : PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockStateStatic(group_, pl_string_name_, state_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockStateStatic(pl_string_name_, state_),do {} while(0))

// Set the lock state and automatically unlocks if needed at the end of the scope (matches std::unique_lock behavior)
#define plLockScopeState(name_, state_)             PL_SCOPE_LOCK_(name_, state_, __LINE__)
#define plgLockScopeState(group_, name_, state_)    PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockScopeState(name_, state_),do {} while(0))
#define plLockScopeStateDyn(name_, state_)          PL_SCOPE_LOCK_DYN_(name_, state_, __LINE__)
#define plgLockScopeStateDyn(group_, name_, state_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockScopeStateDyn(name_, state_),do {} while(0))

// Lock notify
// Shall be placed just before any "notify" call (i.e. semaphore posting, condition variable notify etc...)
#define plLockNotify(name_)                                             \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, \
                                PL_STORE_COLLECT_CASE_, PL_FLAG_TYPE_LOCK_NOTIFIED, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockNotify(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockNotify(name_),do {} while(0))
#define plLockNotifyDyn(name_)                                          \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRawDynName(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, name_, __LINE__, \
                                       PL_STORE_COLLECT_CASE_, PL_FLAG_TYPE_LOCK_NOTIFIED, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockNotifyDyn(group_, name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockNotifyDyn(name_),do {} while(0))
#define plLockNotifyStatic(pl_string_name_)                             \
    do { if(PL_IS_ENABLED_())                                           \
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), (pl_string_name_).hash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:((pl_string_name_).value), __LINE__, \
                                PL_STORE_COLLECT_CASE_, PL_FLAG_TYPE_LOCK_NOTIFIED, PL_GET_CLOCK_TICK_FUNC()); \
    } while(0)
#define plgLockNotifyStatic(group_, pl_string_name_) PL_PRIV_IF(PLG_IS_COMPILE_TIME_ENABLED_(group_), plLockNotifyStatic(pl_string_name_),do {} while(0))

// Detailed memory location. All allocations inside the scope will be associated to the provided name
#define plMemPush(name_)                                                \
    do { if(PL_IS_ENABLED_()) {                                         \
            plPriv::ThreadContext_t* tCtx = &plPriv::threadCtx;         \
            if((tCtx->memLocQty)<PL_MEM_MAX_LOC_PER_THREAD) tCtx->memLocStack[tCtx->memLocQty] = { name_, PL_STRINGHASH(name_) }; \
            tCtx->memLocQty++;                                          \
        } } while(0)
#define plMemPop()                                                      \
    do { if(PL_IS_ENABLED_()) {                                         \
            plPriv::ThreadContext_t* tCtx = &plPriv::threadCtx;         \
            if(tCtx->memLocQty>0) --tCtx->memLocQty;                    \
        } } while(0)

// Debug macro to enable logging only in the scope
#define plScopeEnable() PL_SCOPE_ENABLE_(__LINE__)

#else // if USE_PL==1 && PL_NOEVENT==0

// Empty macros
#define plIsEnabled()        0
#define plgIsEnabled(group_) 0
#define plScopeEnable()                             do { } while(0)
#define plDeclareThread(name_)                      do { } while(0)
#define plgDeclareThread(group_, name_)             do { } while(0)
#define plDeclareThreadDyn(format_, ...)            do { } while(0)
#define plgDeclareThreadDyn(group_, format_, ...)   do { } while(0)
#define plFunction()                                do { } while(0)
#define plgFunction(group_)                         do { } while(0)
#define plFunctionDyn()                             do { } while(0)
#define plgFunctionDyn(group_)                      do { } while(0)
#define plScope(name_)                              do { } while(0)
#define plgScope(group_, name_)                     do { } while(0)
#define plScopeDyn(name_)                           do { } while(0)
#define plgScopeDyn(group_, name_)                  do { } while(0)
#define plBegin(name_)                              do { } while(0)
#define plgBegin(group_, name_)                     do { } while(0)
#define plEnd(name_)                                do { } while(0)
#define plgEnd(group_, name_)                       do { } while(0)
#define plBeginDyn(name_)                           do { } while(0)
#define plgBeginDyn(group_, name_)                  do { } while(0)
#define plEndDyn(name_)                             do { } while(0)
#define plgEndDyn(group_, name_)                    do { } while(0)
#define plData(name_, value_)                       do { } while(0)
#define plgData(group_, name_, value_)              do { } while(0)
#define plText(name_, msg_)                         do { } while(0)
#define plgText(group_, name_, msg_)                do { } while(0)
#define plMarker(category_, msg_)                   do { } while(0)
#define plgMarker(group_, category_, msg_)          do { } while(0)
#define plMarkerDyn(category_, msg_, ...)           do { } while(0)
#define plgMarkerDyn(group_, category_, msg_,...)   do { } while(0)
#define plLogDebug(category_, format_,...)          do { } while(0)
#define plgLogDebug(group_, category_, format_,...) do { } while(0)
#define plLogInfo(category_, format_,...)           do { } while(0)
#define plgLogInfo(group_, category_, format_,...)  do { } while(0)
#define plLogWarn(category_, format_, ...)          do { } while(0)
#define plgLogWarn(group_, category_,format_, ...)  do { } while(0)
#define plLogError(category_, format_,...)          do { } while(0)
#define plgLogError(group_, category_, format_,...) do { } while(0)
#define plVar(...)                                  do { } while(0)
#define plgVar(...)                                 do { } while(0)
#define plLockWait(name_)                           do { } while(0)
#define plgLockWait(group_, name_)                  do { } while(0)
#define plLockWaitDyn(name_)                        do { } while(0)
#define plgLockWaitDyn(group_, name_)               do { } while(0)
#define plLockWaitStatic(name_)                     do { } while(0)
#define plgLockWaitStatic(group_, name_)            do { } while(0)
#define plLockState(name_, state_)                  do { } while(0)
#define plgLockState(group_, name_, state_)         do { } while(0)
#define plLockStateDyn(name_, state_)               do { } while(0)
#define plgLockStateDyn(group_, name_, state_)      do { } while(0)
#define plLockStateStatic(name_, state_)            do { } while(0)
#define plgLockStateStatic(group_, name_, state_)   do { } while(0)
#define plLockScopeState(name_, state_)             do { } while(0)
#define plgLockScopeState(group_, name_, state_)    do { } while(0)
#define plLockScopeStateDyn(name_, state_)          do { } while(0)
#define plgLockScopeStateDyn(group_, name_, state_) do { } while(0)
#define plLockNotify(name_)                         do { } while(0)
#define plgLockNotify(group_, name_)                do { } while(0)
#define plLockNotifyDyn(name_)                      do { } while(0)
#define plgLockNotifyDyn(group_, name_)             do { } while(0)
#define plLockNotifyStatic(name_)                   do { } while(0)
#define plgLockNotifyStatic(group_, name_)          do { } while(0)
#define plMemPush(name_)                            do { } while(0)
#define plMemPop()                                  do { } while(0)

#endif // if USE_PL==1 && PL_NOEVENT==0


// Optimization of the branching, mainly for assertions
#if defined(__GNUC__) || defined(__clang__)
#define PL_LIKELY(x)   (__builtin_expect(!!(x), 1))
#define PL_UNLIKELY(x) (__builtin_expect(!!(x), 0))
#define PL_NOINLINE      __attribute__ ((noinline))
#define PL_NOINSTRUMENT  __attribute__ ((no_instrument_function))
#define PL_NORETURN      __attribute__ ((__noreturn__))
#else
#define PL_LIKELY(x)   (x)
#define PL_UNLIKELY(x) (x)
#define PL_NOINLINE
#define PL_NOINSTRUMENT
#define PL_NORETURN
#endif

#if defined(__GNUC__)
#define PL_PRINTF_CHECK(formatStringIndex_, firstArgIndex_) __attribute__((__format__(__printf__, formatStringIndex_, firstArgIndex_)))
#define PL_PRINTF_FORMAT_STRING
#elif _MSC_VER
#define PL_PRINTF_CHECK(formatStringIndex_, firstArgIndex_)
#define PL_PRINTF_FORMAT_STRING _Printf_format_string_
#else
#define PL_PRINTF_CHECK(formatStringIndex_, firstArgIndex_)
#define PL_PRINTF_FORMAT_STRING
#endif

#define PL_UNUSED(x) ((void)(x))

// May be overridden in the Makefile, for instance to provide a more relevant path only from project's root
#ifndef PL_FILENAME
#define PL_FILENAME __FILE__
#endif

// Best possible function name for assertions
#if defined(__GNUC__) || defined(__clang__)
  #define PL_ASSERT_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
  #define PL_ASSERT_FUNCTION __FUNCSIG__
#else
  #define PL_ASSERT_FUNCTION __func__
#endif

// DLL export
#if defined(__GNUC__) || defined(__clang__)
#define PL_DLL_EXPORT __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#define PL_DLL_EXPORT __declspec(dllexport)
#else
#define PL_DLL_EXPORT
#endif


//-----------------------------------------------------------------------------
// Internal base macros & helpers
//-----------------------------------------------------------------------------

// Macro and templates to isolate the file basename at compile-time (note: the full path is still stored in rodata section)
namespace plPriv {
    template <size_t S> inline constexpr size_t getFilenameOffset(const char (& str)[S], size_t i=S-1)
    { return (str[i] == '/'  || str[i] == '\\' ) ? i+1 : (i>0 ? getFilenameOffset(str, i-1) : 0); }
    template <size_t S> inline constexpr size_t getFilenameOffset(const wchar_t (& str)[S], size_t i=S-1)
    { return (str[i] == L'/' || str[i] == L'\\') ? i+1 : (i>0 ? getFilenameOffset(str, i-1) : 0); }
}
#define PL_BASEFILENAME &PL_FILENAME[plPriv::getFilenameOffset(PL_FILENAME)]

// Conditional inclusion macro trick
#define PL_PRIV_IF(cond, foo1, foo2)       PL_PRIV_IF_IMPL(cond, foo1, foo2)
#define PL_PRIV_IF_IMPL(cond, foo1, foo2)  PL_PRIV_IF_IMPL2(cond,foo1, foo2)
#define PL_PRIV_IF_IMPL2(cond, foo1, foo2) PL_PRIV_IF_ ## cond (foo1, foo2)
#define PL_PRIV_IF_0(foo1, foo2) foo2
#define PL_PRIV_IF_1(foo1, foo2) foo1

// Variadic macro trick (from 0 up to 10 arguments)
#define PL_PRIV_EXPAND(x)                          x
#define PL_PRIV_PREFIX(...)                        0,##__VA_ARGS__
#define PL_PRIV_LASTOF12(a,b,c,d,e,f,g,h,i,j,k,l,...)  l
#define PL_PRIV_SUB_NBARG(...)                     PL_PRIV_EXPAND(PL_PRIV_LASTOF12(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0))
#define PL_PRIV_NBARG(...)                         PL_PRIV_SUB_NBARG(PL_PRIV_PREFIX(__VA_ARGS__))
#define PL_PRIV_GLUE(x, y) x y
#define PL_PRIV_OVERLOAD_MACRO2(name, count) name##count
#define PL_PRIV_OVERLOAD_MACRO1(name, count) PL_PRIV_OVERLOAD_MACRO2(name, count)
#define PL_PRIV_OVERLOAD_MACRO(name, count)  PL_PRIV_OVERLOAD_MACRO1(name, count)
#define PL_PRIV_CALL_OVERLOAD(name, ...)     PL_PRIV_GLUE(PL_PRIV_OVERLOAD_MACRO(name, PL_PRIV_NBARG(__VA_ARGS__)), (__VA_ARGS__))

#define PLG_IS_COMPILE_TIME_ENABLED_(group_) PL_GROUP_ ## group_

namespace plPriv {

    // Definition of the clock tick type
#if PL_SHORT_DATE==1
    typedef uint32_t clockType_t;
#else
    typedef uint64_t clockType_t;
#endif

    // Hash salt in case of collision (very unlikely with 64 bits hash)
#ifndef PL_HASH_SALT
#define PL_HASH_SALT 0
#endif

    // Definition of the string hash depending on the desired size
#if PL_SHORT_STRING_HASH==0
#define PL_FNV_HASH_OFFSET_ (14695981039346656037ULL+PL_HASH_SALT)
#define PL_FNV_HASH_PRIME_  1099511628211ULL
#define PL_PRI_HASH PRIX64
    typedef uint64_t hashStr_t;
#else
#define PL_FNV_HASH_OFFSET_ (2166136261UL+PL_HASH_SALT)
#define PL_FNV_HASH_PRIME_  16777619UL
#define PL_PRI_HASH PRIX32
    typedef uint32_t hashStr_t;
#endif

    // Static string management
    // Used string hash functions is Fowler–Noll–Vo (trade-off between: easy to do at compile time / performances / spreading power)
    constexpr hashStr_t fnv1a_(const char* s, hashStr_t offset) {
        return (!(*s))? offset : fnv1a_(s+1, (offset^((hashStr_t)(*s)))*PL_FNV_HASH_PRIME_);
    }
    template <hashStr_t V> struct forceCompileTimeElseError_ { static constexpr hashStr_t compileTimeValue = V; };
} // namespace plPriv

#define PL_STRINGHASH(s) plPriv::forceCompileTimeElseError_<plPriv::fnv1a_(s,PL_FNV_HASH_OFFSET_)>::compileTimeValue

// Definition of the hashed string type
// Useful to hash some strings at compile time and use them later
struct plString_t {
    plString_t(void) = default;
    plString_t(const char* value_, plPriv::hashStr_t hash_) : value(value_), hash(hash_) {}
    operator const char*() const { return value; }
    const char*       value; // May be null (case of external strings)
    plPriv::hashStr_t hash;  // Zero means no hash
};

// Macro to create a plHashString (does hash computation at compile time)
#define plMakeString(s) plString_t(PL_EXTERNAL_STRINGS?0:(s), PL_STRINGHASH(s))

#if USE_PL==1
// Break inside this function in debugger
void PL_NORETURN plCrash(const char* message);
#endif // if USE_PL==1

//-----------------------------------------------------------------------------
// Internal assertions
//-----------------------------------------------------------------------------

#if USE_PL==1 && PL_NOASSERT==0

#if PL_EXTERNAL_STRINGS==0
namespace plPriv {
    constexpr int CRASH_MSG_SIZE = 1024;
    // Per displayed type
    template<typename T> inline void printParamType_(char* infoStr, int& offset, const char* name, T  param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %s is not a numeric or string type (enum?)\n", name); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; PL_UNUSED(param); }
    template<typename T> inline void printParamType_(char* infoStr, int& offset, const char* name, T* param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %-7s %-20s = %p\n", "pointer", name, param); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; }
    template<> inline void printParamType_<bool>(char* infoStr, int& offset, const char* name, bool param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %-7s %-20s = %s\n", "bool", name, param?"true":"false"); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; }
    template<> inline void printParamType_<char>(char* infoStr, int& offset, const char* name, char* param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %s\n", param); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; PL_UNUSED(name); }
    template<> inline void printParamType_<const char>(char* infoStr, int& offset, const char* name, const char* param)
    {  offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %s\n", param); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; PL_UNUSED(name); }
#define PL_DECLARE_ASSERT_TYPE(type_, code_, display_)                  \
    template<> inline void printParamType_<type_>(char* infoStr, int& offset, const char* name, type_ param) \
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %-7s %-20s = %" #code_ "\n", #display_, name, param); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; }
    PL_DECLARE_ASSERT_TYPE(char,           d, s8);
    PL_DECLARE_ASSERT_TYPE(unsigned char,  u, u8);
    PL_DECLARE_ASSERT_TYPE(short int,      d, s16);
    PL_DECLARE_ASSERT_TYPE(unsigned short, u, u16);
    PL_DECLARE_ASSERT_TYPE(int,            d, int);
    PL_DECLARE_ASSERT_TYPE(unsigned int,   u, u32);
    PL_DECLARE_ASSERT_TYPE(long,           ld, s64);
    PL_DECLARE_ASSERT_TYPE(unsigned long,  lu, u64);
    PL_DECLARE_ASSERT_TYPE(long long,      lld, s64);
    PL_DECLARE_ASSERT_TYPE(unsigned long long, llu, u64);
    PL_DECLARE_ASSERT_TYPE(float,          f,  float);
    PL_DECLARE_ASSERT_TYPE(double,         lf, double);
    template<typename T> inline void printParams_(char* infoStr, int& offset, const char* name, T param)
    { if(name) { printParamType_(infoStr, offset, name, param); } }
    template<typename T, typename... Args> inline void printParams_(char* infoStr, int& offset, const char* name, T value, Args... args)
    { if(name) { printParamType_(infoStr, offset, name, value); } printParams_(infoStr, offset, args...); }

    // Variadic template based assertion display
    template<typename... Args> void PL_NOINLINE PL_NORETURN failedAssert(const char* filename, int lineNbr, const char* function, const char* condition, Args... args)
    {
        char infoStr[CRASH_MSG_SIZE];
        int offset = snprintf(infoStr, sizeof(infoStr), "[PALANTEER] Assertion failed: %s\n  On function: %s\n  On file    : %s(%d)\n", condition, function, filename, lineNbr);
        if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1;
        printParams_(infoStr, offset, args...); // Recursive display of provided items
        plCrash(infoStr);
    }

} // namespace plPriv

// Macro to stringify the additional parameters of the enhanced assertions
// Note: in C99 and C++11, zero parameter is a problem, hence the forced & dummy first parameter "".
#define PL_PRIV_ASSERT_PARAM0()   nullptr, 0
#define PL_PRIV_ASSERT_PARAM1(v1) nullptr, 0
#define PL_PRIV_ASSERT_PARAM2(v1, v2)  nullptr, 0, #v2, v2
#define PL_PRIV_ASSERT_PARAM3(v1, v2, v3)  nullptr, 0, #v2, v2, #v3, v3
#define PL_PRIV_ASSERT_PARAM4(v1, v2, v3, v4)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4
#define PL_PRIV_ASSERT_PARAM5(v1, v2, v3, v4, v5)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4, #v5, v5
#define PL_PRIV_ASSERT_PARAM6(v1, v2, v3, v4, v5, v6)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4, #v5, v5, #v6, v6
#define PL_PRIV_ASSERT_PARAM7(v1, v2, v3, v4, v5, v6, v7)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4, #v5, v5, #v6, v6, #v7, v7
#define PL_PRIV_ASSERT_PARAM8(v1, v2, v3, v4, v5, v6, v7, v8)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4, #v5, v5, #v6, v6, #v7, v7, #v8, v8
#define PL_PRIV_ASSERT_PARAM9(v1, v2, v3, v4, v5, v6, v7, v8, v9)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4, #v5, v5, #v6, v6, #v7, v7, #v8, v8, #v9, v9
#define PL_PRIV_ASSERT_PARAM10(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)  nullptr, 0, #v2, v2, #v3, v3, #v4, v4, #v5, v5, #v6, v6, #v7, v7, #v8, v8, #v9, v9, #v10, v10

#else // if PL_EXTERNAL_STRINGS==0

namespace plPriv {

    constexpr int CRASH_MSG_SIZE = 1024;
    // Per displayed type
    template<typename T> inline void printParamTypeEs_(char* infoStr, int& offset, hashStr_t nameHash, T  param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - @@%016" PL_PRI_HASH "@@ is not a numeric or string type (enum?)\n", nameHash); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; PL_UNUSED(param); }
    template<typename T> inline void printParamTypeEs_(char* infoStr, int& offset, hashStr_t nameHash, T* param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - pointer @@%016" PL_PRI_HASH "@@ = %p\n", nameHash, param); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; }
    template<> inline void printParamTypeEs_<bool>(char* infoStr, int& offset, hashStr_t nameHash, bool param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - bool    @@%016" PL_PRI_HASH "@@ = %s\n", nameHash, param?"true":"false"); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; }
    template<> inline void printParamTypeEs_<char>(char* infoStr, int& offset, hashStr_t nameHash, char* param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - @@%016" PL_PRI_HASH "@@\n", nameHash); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; PL_UNUSED(param); }
    template<> inline void printParamTypeEs_<const char>(char* infoStr, int& offset, hashStr_t nameHash, const char* param)
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - @@%016" PL_PRI_HASH "@@\n", nameHash); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; PL_UNUSED(param); }
#define PL_DECLARE_ASSERT_TYPE(type_, code_, display_)                  \
    template<> inline void printParamTypeEs_<type_>(char* infoStr, int& offset, hashStr_t nameHash, type_ param) \
    { offset += snprintf(infoStr+offset, CRASH_MSG_SIZE-offset, "    - %-7s @@%016" PL_PRI_HASH "@@ = %" #code_ "\n", #display_, nameHash, param); if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1; }
    PL_DECLARE_ASSERT_TYPE(char,           d, s8);
    PL_DECLARE_ASSERT_TYPE(unsigned char,  d, u8);
    PL_DECLARE_ASSERT_TYPE(short int,      d, s16);
    PL_DECLARE_ASSERT_TYPE(unsigned short, d, u16);
    PL_DECLARE_ASSERT_TYPE(int,            d, int);
    PL_DECLARE_ASSERT_TYPE(unsigned int,   u, u32);
    PL_DECLARE_ASSERT_TYPE(long,           ld, s64);
    PL_DECLARE_ASSERT_TYPE(unsigned long,  lu, u64);
    PL_DECLARE_ASSERT_TYPE(long long,      lld, s64);
    PL_DECLARE_ASSERT_TYPE(unsigned long long, llu, u64);
    PL_DECLARE_ASSERT_TYPE(float,          f,  float);
    PL_DECLARE_ASSERT_TYPE(double,         lf, double);
    template<typename T> inline void printParamsEs_(char* infoStr, int& offset, hashStr_t nameHash, T param)
    { if(nameHash) { printParamTypeEs_(infoStr, offset, nameHash, param); } }
    template<typename T, typename... Args> inline void printParamsEs_(char* infoStr, int& offset, hashStr_t nameHash, T value, Args... args)
    { if(nameHash) { printParamTypeEs_(infoStr, offset, nameHash, value); } printParamsEs_(infoStr, offset, args...); }

    // Variadic template based assertion display
    template<typename... Args> void PL_NOINLINE PL_NORETURN failedAssertEs(hashStr_t filenameHash, int lineNbr, hashStr_t conditionHash, Args... args)
    {
        char infoStr[CRASH_MSG_SIZE];
        int  offset = snprintf(infoStr, sizeof(infoStr), "[PALANTEER] Assertion failed: @@%016" PL_PRI_HASH "@@\n  On file @@%016" PL_PRI_HASH "@@(%d)\n",
                               conditionHash, filenameHash, lineNbr);
        if(offset>CRASH_MSG_SIZE-1) offset = CRASH_MSG_SIZE-1;
        printParamsEs_(infoStr, offset, args...); // Recursive display of provided items
        plCrash(infoStr);
    }

} // namespace plPriv

// Macro to stringify the additional parameters of the enhanced assertions
// Note: in C99 and C++11, zero parameter is a problem, hence the forced & dummy first parameter "".
#define PL_PRIV_ASSERT_PARAM_ES0()   (plPriv::hashStr_t)0, 0
#define PL_PRIV_ASSERT_PARAM_ES1(v1) (plPriv::hashStr_t)0, 0
#define PL_PRIV_ASSERT_PARAM_ES2(v1, v2)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2
#define PL_PRIV_ASSERT_PARAM_ES3(v1, v2, v3)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3
#define PL_PRIV_ASSERT_PARAM_ES4(v1, v2, v3, v4)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4
#define PL_PRIV_ASSERT_PARAM_ES5(v1, v2, v3, v4, v5)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4, PL_STRINGHASH(#v5), v5
#define PL_PRIV_ASSERT_PARAM_ES6(v1, v2, v3, v4, v5, v6)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4, PL_STRINGHASH(#v5), v5, PL_STRINGHASH(#v6), v6
#define PL_PRIV_ASSERT_PARAM_ES7(v1, v2, v3, v4, v5, v6, v7)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4, PL_STRINGHASH(#v5), v5, PL_STRINGHASH(#v6), v6, PL_STRINGHASH(#v7), v7
#define PL_PRIV_ASSERT_PARAM_ES8(v1, v2, v3, v4, v5, v6, v7, v8)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4, PL_STRINGHASH(#v5), v5, PL_STRINGHASH(#v6), v6, PL_STRINGHASH(#v7), v7, PL_STRINGHASH(#v8), v8
#define PL_PRIV_ASSERT_PARAM_ES9(v1, v2, v3, v4, v5, v6, v7, v8, v9)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4, PL_STRINGHASH(#v5), v5, PL_STRINGHASH(#v6), v6, PL_STRINGHASH(#v7), v7, PL_STRINGHASH(#v8), v8, PL_STRINGHASH(#v9), v9
#define PL_PRIV_ASSERT_PARAM_ES10(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)  (plPriv::hashStr_t)0, 0, PL_STRINGHASH(#v2), v2, PL_STRINGHASH(#v3), v3, PL_STRINGHASH(#v4), v4, PL_STRINGHASH(#v5), v5, PL_STRINGHASH(#v6), v6, PL_STRINGHASH(#v7), v7, PL_STRINGHASH(#v8), v8, PL_STRINGHASH(#v9), v9, PL_STRINGHASH(#v10), v10

#endif // if PL_EXTERNAL_STRINGS==0

#endif // if USE_PL==1 && PL_NOASSERT==0

#if USE_PL==1 && PL_NOASSERT==0
namespace plPriv {
#if PL_EXTERNAL_STRINGS==0
    void PL_NORETURN failedAssertSimple(const char* filename, int lineNbr, const char* function, const char* condition);
#else
    void PL_NORETURN failedAssertSimpleEs(hashStr_t filenameHash, int lineNbr, hashStr_t conditionHash);
#endif // if PL_EXTERNAL_STRINGS==0
}
#endif // if USE_PL==1 && PL_NOASSERT==0



//-----------------------------------------------------------------------------
// Internal helpers
//-----------------------------------------------------------------------------

#if USE_PL==1

namespace plPriv {

    // Simple fixed allocation array. Use template size if non-null, or the dynamic max size
    template <typename T, int N>
    class Array {
    public:
        Array(int dynMaxSize=0) : _maxSize(N? N : dynMaxSize) { _array = new T[_maxSize]; }
        ~Array(void) { delete[] _array; }
        void clear(void)      { _size = 0; }
        void resize(int size) { plAssert(size<=_maxSize, size, _maxSize); _size = size; }
        int  size(void) const { return _size; }
        int  capacity(void)   const { return _maxSize; }
        int  free_space(void) const { return _maxSize-_size; }
        inline T&       operator[](int index)       { return _array[index]; }
        inline const T& operator[](int index) const { return _array[index]; }
    private:
        Array(const Array<T,N>& other); // To please static analyzers
        Array<T,N>& operator=(Array<T,N> other);
        int _maxSize = 0;
        int _size    = 0;
        T*  _array   = 0;
    };

    // Memory pool helper (for the dynamic strings)
    template<typename T>
    class MemoryPool {
    public:
        MemoryPool(int size, std::atomic<int>* notifyEmpty) : _size(size), _notifyEmpty(notifyEmpty) {
            _fifo   = new T*[_size];
            _buffer = new char[_size*sizeof(T)];
            for(int i=0; i<_size-1; ++i) _fifo[i] = (T*)&_buffer[i*sizeof(T)];
            // Free elements are in [tail;head[ hence the (size-1) item population
            _head = size-1;
            _tail = 0;
        }
        ~MemoryPool(void) { delete[] _fifo; delete[] _buffer; }
        // Concurrent calls
        T* get(void) {
            int expected = _tail.load();
            while(expected==_head.load()) { _notifyEmpty->store(1); std::this_thread::yield(); expected = _tail.load(); } // Empty FIFO
            int desired  = (expected+1)%_size;
            T*  chunk    = _fifo[expected];
            while(!_tail.compare_exchange_weak(expected, desired)) {
                expected = _tail.load();
                while(expected==_head.load()) { _notifyEmpty->store(1); std::this_thread::yield(); expected = _tail.load(); } // Empty FIFO
                desired  = (expected+1)%_size;
                chunk    = _fifo[expected];
            }
            return chunk;
        }
        // Single call (from collection thread)
        void release(T* chunk) {
            plAssert(chunk>=(T*)_buffer && chunk<=((T*)_buffer)+_size);
            int expected = _head.load();
            int desired  = (expected+1)%_size;
            _fifo[expected] = chunk; // Shall be free by design
            while(!_head.compare_exchange_weak(expected, desired)) { // Case of collision with a 'get' call
                expected = _head.load();
                desired  = (expected+1)%_size;
                _fifo[expected] = chunk;
            }
        }
        int getUsed(void) const { return (_size-1+_tail.load()-_head.load())%_size; } // Race condition in the "good" direction, so ok
        int getSize(void) const { return _size; }
    private:
        MemoryPool(const MemoryPool<T>& other); // To please static analyzers
        MemoryPool<T>& operator=(MemoryPool<T> other);
        char* _buffer;
        T**   _fifo;
        int   _size;
        std::atomic<int>  _head;
        std::atomic<int>  _tail;
        std::atomic<int>* _notifyEmpty = 0; // Set to 1 when pool is empty
    };

#if defined(__unix__) && PL_NOEVENT==0 && PL_IMPLEMENTATION==1 && PL_IMPL_CONTEXT_SWITCH==1
    static
    uint64_t
    parseNumber(char*& ptr)
    {
        uint64_t val   = 0;
        int afterComma = -1;
        while(1) {
            if(*ptr>='0' && *ptr<='9') {
                val = val*10 + (*(ptr++)-'0');
                if(afterComma>=0) ++afterComma;
            } else if(*ptr=='.') {
                ++ptr;
                afterComma = 0;
            }
            else break;
        }
        // We want nanoseconds if a comma is found
        while(afterComma>=0 && afterComma<9) {
            val *= 10;
            ++afterComma;
        }
        return val;
    }

    static
    void
    parseString(char*& ptr, char* dstStr, int maxLength)
    {
        const char* startPtr = ptr;
        while(*ptr && *ptr!='\n' && *ptr!=' ' && ptr-startPtr<maxLength-1) *dstStr++ = *ptr++;
        *dstStr = 0;
    }
#endif

} // namespace plPriv

#endif // if USE_PL==1



//-----------------------------------------------------------------------------
// Public remote control service interface
//-----------------------------------------------------------------------------

// Note: for a full description of the CLI declaration and usage, please refer to the official documentation.

#if (USE_PL==1 && PL_NOCONTROL==0) || PL_EXPORT==1
namespace plPriv {
    // Forward declaration of the manager class
    class CliManager;

    // Remote CLI parameter types
    enum plCliParamTypes : uint8_t { PL_INTEGER, PL_FLOAT, PL_STRING, PL_TYPE_QTY };
} // namespace plPriv

#endif // if (USE_PL==1 && PL_NOCONTROL==0) || PL_EXPORT==1

#if USE_PL==1 && PL_NOCONTROL==0

class plCliIo {
public:
    // Parameters
    int64_t     getParamInt   (int paramIdx) const {
        plAssert(paramIdx>=0 && paramIdx<_paramQty, "Wrong parameter index", paramIdx, _paramQty);
        plAssert(_paramTypes[paramIdx]==plPriv::plCliParamTypes::PL_INTEGER, "This parameter is not declared as an integer", paramIdx);
        char* tmp = (char*)&_paramValues[paramIdx]; // Removes strict-aliasing warning
        return *(int64_t*)tmp;
    }

    double      getParamFloat (int paramIdx) const {
        plAssert(paramIdx>=0 && paramIdx<_paramQty, "Wrong parameter index", paramIdx, _paramQty);
        plAssert(_paramTypes[paramIdx]==plPriv::plCliParamTypes::PL_FLOAT, "This parameter is not declared as a float", paramIdx);
        char* tmp = (char*)&_paramValues[paramIdx]; // Removes strict-aliasing warning
        return *(double*)tmp;
    }
    const char* getParamString(int paramIdx) const {
        plAssert(paramIdx>=0 && paramIdx<_paramQty, "Wrong parameter index", paramIdx, _paramQty);
        plAssert(_paramTypes[paramIdx]==plPriv::plCliParamTypes::PL_STRING, "This parameter is not declared as a string", paramIdx);
        return (const char*)_paramValues[paramIdx];
    }

    // Output
    void PL_PRINTF_CHECK(2, 3) setErrorState(PL_PRINTF_FORMAT_STRING const char* format=0, ...)
    {
        _execStatus = false;
        if(!format) return;
        _response.clear();
        va_list args;
        va_start(args, format);
        int writtenQty = vsnprintf(&_response[0], _response.free_space(), format, args);
        va_end(args);
        if(writtenQty>_response.free_space()-1) writtenQty = _response.free_space()-1;
        _response.resize(_response.size()+writtenQty);
    }
    bool PL_PRINTF_CHECK(2, 3) addToResponse(PL_PRINTF_FORMAT_STRING const char* format, ...)
    {
        if(_response.free_space()<=0) return false;
        va_list args;
        va_start(args, format);
        int writtenQty = vsnprintf(&_response[_response.size()], _response.free_space(), format, args);
        va_end(args);
        if(writtenQty>_response.free_space()-1) writtenQty = _response.free_space()-1;
        _response.resize(_response.size()+writtenQty);
        return _response.size()<_response.capacity()-1;
    }
    void clearResponse(void) { _response[0] = 0; _response.clear(); }

    // Introspection for generic wrappers
    uint64_t getCliNameHash(void) const { return _cliNameHash; }
    int  getParamQty(void) const { return _paramQty; };
    bool isParamInt(int paramIdx) const {
        plAssert(paramIdx>=0 && paramIdx<_paramQty, "Wrong parameter index", paramIdx, _paramQty);
        return _paramTypes[paramIdx]==plPriv::plCliParamTypes::PL_INTEGER;
    }
    bool isParamFloat(int paramIdx) const {
        plAssert(paramIdx>=0 && paramIdx<_paramQty, "Wrong parameter index", paramIdx, _paramQty);
        return _paramTypes[paramIdx]==plPriv::plCliParamTypes::PL_FLOAT;
    }
    bool isParamString(int paramIdx) const  {
        plAssert(paramIdx>=0 && paramIdx<_paramQty, "Wrong parameter index", paramIdx, _paramQty);
        return _paramTypes[paramIdx]==plPriv::plCliParamTypes::PL_STRING;
    }

private:
    plCliIo(int responseBufferByteQty, int maxParamQty) : _response(responseBufferByteQty) {
        _paramTypes  = new plPriv::plCliParamTypes[maxParamQty];
        _paramValues = new uint64_t[maxParamQty];
    }
    ~plCliIo(void) { delete[] _paramTypes; delete[] _paramValues; }
    friend plPriv::CliManager;
    plPriv::hashStr_t        _cliNameHash;
    plPriv::Array<char, 0>   _response;
    plPriv::plCliParamTypes* _paramTypes  = 0;
    uint64_t*                _paramValues = 0;
    int  _paramQty;
    bool _execStatus;
};


// Remote CLI handler prototype
//  When implementing a remote CLI handler, the provided "communication helper" object 'cio' provide both
//  the input parameters and the interface to provide the answer (status and text).
//  When the handler is called, the text answer is empty and the status 'successful'. The text answer can grow
//  up to PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY minus a few bytes of overhead.
typedef void (*plCliHandler_t)(plCliIo& cio);

// Declares a new CLI (Command Line Interface) named 'name' associated with the 'plCliHandler_t'
//  - 'name', 'specParams' and 'description' are static strings
//  - 'specParams' is a string defining the remote CLI parameters. Its syntax is a space separated list of
//      '<paramName>=<int|float|string>' optionally followed by '[[default value]]' to provide a default value.
//      Ex: "colorIndex=int alpha=float[[1.0] name=string[[bright green]]"
//  - Note that a 'string' shall not contains any space or zero. It can contain space if enclosed in a double bracket/
//  - Warning: 'specParams' cannot be masked by the external string feature as its content is used internally
#define plRegisterCli(handler, name, specParams, description)            \
    plPriv::registerCli(handler, PL_EXTERNAL_STRINGS?0:(name), specParams, PL_EXTERNAL_STRINGS?0:(description), \
                       PL_STRINGHASH(name), PL_STRINGHASH(specParams), PL_STRINGHASH(description))

namespace plPriv {
    // Declaration of the private CLI registration function (which wraps the CliManager)
    void registerCli(plCliHandler_t handler, const char* name, const char* specParams, const char* description,
                     hashStr_t nameHash, hashStr_t specParamsHash, hashStr_t descriptionHash);
}

// Declares a step point for the current thread.
//  In 'live control' mode, the user can freeze or free the thread execution inside this function, allowing
//  a step by step execution.
//  The control is performed externally (from viewer GUI (debugging purpose) or remote script (testing purpose))
void plFreezePoint(void);

#else // if USE_PL==1 && PL_NOCONTROL==0

// Empty communication helper object
class plCliIo {
public:
    int64_t     getParamInt   (int paramIdx) const { PL_UNUSED(paramIdx); return 0 ; }
    double      getParamFloat (int paramIdx) const { PL_UNUSED(paramIdx); return 0.; }
    const char* getParamString(int paramIdx) const { PL_UNUSED(paramIdx); return 0 ; }
    void PL_PRINTF_CHECK(2, 3) setErrorState(PL_PRINTF_FORMAT_STRING const char* format=0, ...) { PL_UNUSED(format); }
    bool PL_PRINTF_CHECK(2, 3) addToResponse(PL_PRINTF_FORMAT_STRING const char* format, ...)   { PL_UNUSED(format); return true; }
    void clearResponse(void) { }
    // Introspection for generic wrappers
    uint64_t getCliNameHash(void) const { return 0; }
    int      getParamQty   (void) const { return 0; };
    bool     isParamInt    (int paramIdx) const { (void)paramIdx; return false; }
    bool     isParamFloat  (int paramIdx) const { (void)paramIdx; return false; }
    bool     isParamString (int paramIdx) const { (void)paramIdx; return false; }
};

// Empty declaration in case CLI is not present
#define plRegisterCli(...)

#define plFreezePoint()

#endif // if USE_PL==1 && PL_NOCONTROL==0



//-----------------------------------------------------------------------------
// Event service internals
//-----------------------------------------------------------------------------

// Used by the client service and the server
#if (USE_PL==1 && PL_NOEVENT==0) || PL_EXPORT==1

// Event flags
#define PL_FLAG_TYPE_DATA_NONE       0
#define PL_FLAG_TYPE_DATA_S32        1
#define PL_FLAG_TYPE_DATA_U32        2
#define PL_FLAG_TYPE_DATA_FLOAT      3
#define PL_FLAG_TYPE_DATA_S64        4
#define PL_FLAG_TYPE_DATA_U64        5
#define PL_FLAG_TYPE_DATA_DOUBLE     6
#define PL_FLAG_TYPE_DATA_STRING     7
#define PL_FLAG_TYPE_DATA_TIMESTAMP  8
#define PL_FLAG_TYPE_DATA_QTY        9
#define PL_FLAG_TYPE_THREADNAME      9
#define PL_FLAG_TYPE_MEMORY_FIRST   10  // Memory infos are spread on 2 events
#define PL_FLAG_TYPE_ALLOC_PART     10
#define PL_FLAG_TYPE_DEALLOC_PART   11
#define PL_FLAG_TYPE_WITH_TIMESTAMP_FIRST 12
#define PL_FLAG_TYPE_ALLOC          12
#define PL_FLAG_TYPE_DEALLOC        13
#define PL_FLAG_TYPE_MEMORY_LAST    13
#define PL_FLAG_TYPE_CSWITCH        14
#define PL_FLAG_TYPE_SOFTIRQ        15
#define PL_FLAG_TYPE_LOCK_FIRST     16
#define PL_FLAG_TYPE_LOCK_WAIT      16
#define PL_FLAG_TYPE_LOCK_ACQUIRED  17
#define PL_FLAG_TYPE_LOCK_RELEASED  18
#define PL_FLAG_TYPE_LOCK_NOTIFIED  19
#define PL_FLAG_TYPE_LOCK_LAST      19
#define PL_FLAG_TYPE_LOG            20
#define PL_FLAG_TYPE_WITH_TIMESTAMP_LAST 20
#define PL_FLAG_TYPE_LOG_PARAM      21
#define PL_FLAG_TYPE_MASK           0x1F
#define PL_FLAG_SCOPE_BEGIN         0x20
#define PL_FLAG_SCOPE_END           0x40
#define PL_FLAG_SCOPE_MASK          0x60

#define PL_CSWITCH_CORE_NONE 0xFF

#endif // if (USE_PL==1 && PL_NOEVENT==0) || PL_EXPORT==1


#if USE_PL==1 || PL_EXPORT==1

// Library version
#define PALANTEER_VERSION "0.7.dev1"
#define PALANTEER_VERSION_NUM 601  // Monotonic number. 100 per version component. Official releases are multiple of 100

// Client-Server protocol version
#define PALANTEER_CLIENT_PROTOCOL_VERSION 3

// Maximum thread quantity is 254 (server limitation for efficient storage)
#define PL_MAX_THREAD_QTY 254

// Maximum memory detail stack depth
#define PL_MEM_MAX_LOC_PER_THREAD  32

// Protocol TLVs used at connection time with the server
#define PL_TLV_PROTOCOL              0 /* Mandatory and shall be first */
#define PL_TLV_CLOCK_INFO            1 /* Mandatory */
#define PL_TLV_APP_NAME              2 /* Mandatory */
#define PL_TLV_HAS_BUILD_NAME        3
#define PL_TLV_HAS_LANG_NAME         4
#define PL_TLV_HAS_EXTERNAL_STRING   5
#define PL_TLV_HAS_SHORT_STRING_HASH 6
#define PL_TLV_HAS_NO_CONTROL        7
#define PL_TLV_HAS_SHORT_DATE        8
#define PL_TLV_HAS_COMPACT_MODEL     9
#define PL_TLV_HAS_HASH_SALT        10
#define PL_TLV_HAS_AUTO_INSTRUMENT  11
#define PL_TLV_HAS_CSWITCH_INFO     12
#define PL_TLV_QTY                  13

#endif

#if USE_PL==1
namespace plPriv {

    // Event structure for immediate storage in buffer
    // Max size is 8*8= 64 bytes on 64 bits,  9*4=36 bytes on 32 bits arch
    struct EventInt {
        uint8_t     threadId;
        uint8_t     flags;
        uint16_t    lineNbr;
        uint32_t    extra;
        hashStr_t   filenameHash;
        hashStr_t   nameHash;
        const char* filename;
        const char* name;
        union {
            int32_t  vInt;
            uint32_t vU32;
            float    vFloat;
            plString_t vString;  // Contains 1 pointer and 1 hashStr_t
#if PL_COMPACT_MODEL==0
            int64_t  vS64;
            uint64_t vU64;
            double   vDouble;
#endif
        };
        uint32_t writeAck;  // Used to detect that the event writing is really done
        // For 64bits arch, an additional uint32_t padding is implicitely added
    };

#if PL_COMPACT_MODEL==1
#define PL_PRIV_RAW_FIELD vU32
#define PL_PRIV_EVENTEXT_SIZE 12
    typedef uint32_t bigRawData_t;
    typedef uint16_t nameData_t;
#else
#define PL_PRIV_RAW_FIELD vU64
#define PL_PRIV_EVENTEXT_SIZE 24
    typedef uint64_t bigRawData_t;
    typedef uint32_t nameData_t;
#endif

    struct DynString_t { char dummy[PL_DYN_STRING_MAX_SIZE]; };

    struct MemLocation { const char* memStr; hashStr_t memHash; };

    struct ThreadInfo_t {
        uint32_t  pid      = 0;  // OS Process ID, to track the context switches
#if PL_VIRTUAL_THREADS==1
        bool      isSuspended;
        bool      isBeginSent;
#endif
        hashStr_t nameHash = 0;
        char      name[PL_DYN_STRING_MAX_SIZE] = {0};  // Thread name persistency
    };

    // Global event collection service context, accessible from everywhere
    struct GlobalContext_t {
        GlobalContext_t(int dynStringQty) : bankAndIndex(0), nextThreadId(0), isBufferSaturated(0), isDynStringPoolEmpty(0),
                                            dynStringPool(dynStringQty, &isDynStringPoolEmpty) { }
        alignas(64) std::atomic<uint32_t> bankAndIndex = { 0 }; // Force this often used (R/W) atomic in its own cache line (64 is conservative) for performance reasons
        alignas(64) std::atomic<uint32_t> nextThreadId = { 0 };
        EventInt* collectBuffers[2]       = { 0, 0 };
        bool     enabled                  = false;
        bool     collectEnabled           = false;
        int      collectBufferMaxEventQty = 0;
        std::atomic<int> isBufferSaturated = { 0 };
        std::atomic<int> isDynStringPoolEmpty = { 0 };
        plLogLevel minLogLevelRecord  = PL_LOG_LEVEL_DEBUG;
        plLogLevel minLogLevelConsole = PL_LOG_LEVEL_WARN;
        uint64_t   originNs = 0ULL;  // Low resolution (used for logs)
        MemoryPool<DynString_t> dynStringPool;
        ThreadInfo_t            threadInfos[PL_MAX_THREAD_QTY];
    };
    extern GlobalContext_t globalCtx;

    // Per thread context (using Thread Local Storage)
    struct ThreadContext_t {
        uint32_t    id         = 0xFFFFFFFF;
#if PL_VIRTUAL_THREADS==1
        uint32_t    realId     = 0xFFFFFFFF;  // Saves the initial Id, to be able to go back to the OS thread
        hashStr_t   realRscNameHash = 0;      // Name of the worker thread resource
#endif
        int         memLocQty  = 0;
        MemLocation memLocStack[PL_MEM_MAX_LOC_PER_THREAD];
    };
    extern thread_local ThreadContext_t threadCtx;

#if PL_NOEVENT==0 || PL_NOCONTROL==0

    inline uint8_t getThreadId(void) {
        ThreadContext_t* tCtx = &threadCtx;
        if(tCtx->id==0xFFFFFFFF) {
            tCtx->id     = globalCtx.nextThreadId.fetch_add(1);
#if PL_VIRTUAL_THREADS==1
            tCtx->realId = tCtx->id; // Saved for restoration later
#endif
            if(tCtx->id<PL_MAX_THREAD_QTY) globalCtx.threadInfos[tCtx->id].pid = PL_GET_SYS_THREAD_ID();
        }
        return (uint8_t)tCtx->id;
    }

    // Dynamic string hashing
    inline hashStr_t hashString(const char* s, int maxCharQty=-1) {
        hashStr_t strHash = PL_FNV_HASH_OFFSET_;
        while(*s && maxCharQty--) strHash = (strHash^((hashStr_t)(*s++)))*PL_FNV_HASH_PRIME_;
        return (strHash==0)? 1 : strHash; // Zero is a reserved value
    }

#endif // if PL_NOEVENT==0 || PL_NOCONTROL==0

    // Clock implementation
    // Note: The effective clock frequency will be measured/calibrated at initialization time. This is also convenient for custom clock getter.
    inline clockType_t getClockTick(void) {
        // RDTSC instruction is ~7x times more precise than the std::chrono or Windows' QPC timer.
        //  And nowadays it is reliable (it was not the case on older processors where its frequency was changing with power plans).
        //  The potential wrong instruction order (no fence here) is considered as noise on the timestamp, with the benefit of a much smaller average resolution.
#if defined(_WIN32)
        // Windows
        // Context switch events are using QPC, so a date conversion for these events is needed on Windows
        return (clockType_t) int64_t(__rdtsc());
#elif defined(__x86_64__)
        // Linux
        // Kernel events can be configured to use RDTSC too, so no extra work on converting clocks under Linux
        uint64_t low, high;
        asm volatile ("rdtsc" : "=a" (low), "=d" (high));
#if PL_SHORT_DATE==1
        return (clockType_t)low;
#else
        return (high<<32)+low;
#endif
#else
        // C++11 standard case (slower but more portable)
        return (clockType_t) std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
    }

} // namespace plPriv

#endif // if USE_PL==1


#if USE_PL==1 && PL_NOEVENT==0

#define PL_SCOPE__(name_, ext_) plPriv::TimedScope timedScope##ext_(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__)
#define PL_SCOPE_(name_, ext_)  PL_SCOPE__(name_, ext_)

#define PL_SCOPE_DYN__(name_, ext_)  plPriv::TimedScopeDyn timedScope##ext_(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, name_, __LINE__)
#define PL_SCOPE_DYN_(name_, ext_)   PL_SCOPE_DYN__(name_, ext_)

#define PL_SCOPE_LOCK__(name_, state_, ext_) plPriv::TimedLock timedLock##ext_(PL_STRINGHASH(PL_BASEFILENAME), PL_STRINGHASH(name_), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, PL_EXTERNAL_STRINGS?0:(name_), __LINE__, state_)
#define PL_SCOPE_LOCK_(name_, state_, ext_)  PL_SCOPE_LOCK__(name_, state_, ext_)

#define PL_SCOPE_LOCK_DYN__(name_, state_, ext_) plPriv::TimedLockDyn timedLock##ext_(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, name_, __LINE__, state_)
#define PL_SCOPE_LOCK_DYN_(name_, state_, ext_)  PL_SCOPE_LOCK_DYN__(name_, state_, ext_)

// Intermediate event macros
#define PL_STORE_COLLECT_CASE_ 0
#define PL_IS_ENABLED_() ((!PL_STORE_COLLECT_CASE_ && plPriv::globalCtx.enabled) || (PL_STORE_COLLECT_CASE_ && plPriv::globalCtx.collectEnabled))

namespace plPriv {

    // Dynamic string helper (allocation + copy)
    inline char* getDynString(const char* s) {
        char* allocStr = (char*)globalCtx.dynStringPool.get(); // Note: may busy wait if pool is empty
        int   copySize = (int)strlen(s)+1; if(copySize>PL_DYN_STRING_MAX_SIZE) copySize = PL_DYN_STRING_MAX_SIZE;
        memcpy(allocStr, s, copySize); allocStr[PL_DYN_STRING_MAX_SIZE-1] = 0;
        return allocStr;
    }

    // Dynamic string formatter (just to handle the case with and without arguments)
    inline void formatDynString(char* dynString, const char* format) {
        int minSize = (int)(strlen(format)+1);
        if(minSize>PL_DYN_STRING_MAX_SIZE) minSize = PL_DYN_STRING_MAX_SIZE;
        memcpy(dynString, format, minSize);
        dynString[PL_DYN_STRING_MAX_SIZE-1] = 0;
    }
    template<typename... Args>
    inline void formatDynString(char* dynString, const char* format, Args... args) {
        (void)snprintf(dynString, PL_DYN_STRING_MAX_SIZE, format, args...);
    }

    // Some masks
    constexpr uint32_t EVTBUFFER_MASK_INDEX = 0x7FFFFFFF;  // Event index in the current buffer bank
    constexpr uint32_t EVTBUFFER_MASK_BANK  = 0x80000000;  // Bank index

    inline EventInt& eventLogBase(uint32_t bi, hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_, int lineNbr_, int flags_) {
        EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX];
        e.threadId     = getThreadId();
        e.flags        = (uint8_t)flags_;
        e.lineNbr      = (uint16_t)lineNbr_;
        e.filenameHash = filenameHash_;
        e.nameHash     = nameHash_;
        e.filename     = filename_;
        e.name         = name_;
        return e;
    }

    inline void eventCheckOverflow(uint32_t bi) {
        if((int)(bi&EVTBUFFER_MASK_INDEX)>=globalCtx.collectBufferMaxEventQty) {
            while((int)(globalCtx.bankAndIndex.load()&EVTBUFFER_MASK_INDEX)>=globalCtx.collectBufferMaxEventQty) {
                globalCtx.isBufferSaturated.store(1); std::this_thread::yield();
            }
        }
    }

    inline void eventLogRaw(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                            int lineNbr_, bool doSkipOverflowCheck_, int flags_, bigRawData_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, flags_);
        e.PL_PRIV_RAW_FIELD = v;
        e.writeAck = 1;  // Tells that all previous data have been written
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogRawDynName(hashStr_t filenameHash_, const char* filename_, const char* name_,
                                   int lineNbr_, bool doSkipOverflowCheck_, int flags_, bigRawData_t v) {
        const char* allocStr = getDynString(name_);
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, 0, filename_, allocStr, lineNbr_, flags_);
        e.PL_PRIV_RAW_FIELD = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogRawDynName(hashStr_t filenameHash_, const char* filename_, plString_t name_,
                                   int lineNbr_, bool doSkipOverflowCheck_, int flags_, bigRawData_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, name_.hash? name_.hash:1, filename_, name_.value, lineNbr_, flags_);
        e.PL_PRIV_RAW_FIELD = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogRawDynFile(hashStr_t nameHash_, const char* filename_, const char* name_,
                                   int lineNbr_, bool doSkipOverflowCheck_, int flags_, bigRawData_t v) {
        const char* allocStr = getDynString(filename_);
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, 0, nameHash_? nameHash_:1, allocStr, name_, lineNbr_, flags_);
        e.PL_PRIV_RAW_FIELD = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    template<typename... Args>
    inline void eventLogRawDynFile(hashStr_t nameHash_, const char* format_, const char* name_,
                                   int lineNbr_, bool doSkipOverflowCheck_, int flags_, bigRawData_t v,
                                   Args... args)
    {
        const char* allocStr = getDynString(""); // We know exactly the allocated size for this pointer
        snprintf((char*)allocStr, PL_DYN_STRING_MAX_SIZE, format_, args...);
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, 0, nameHash_? nameHash_:1, allocStr, name_, lineNbr_, flags_);
        e.PL_PRIV_RAW_FIELD = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogRawDynFile(hashStr_t nameHash_, plString_t filename_,const char* name_,
                                   int lineNbr_, bool doSkipOverflowCheck_, int flags_, bigRawData_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filename_.hash? filename_.hash:1, nameHash_? nameHash_:1, filename_.value, name_, lineNbr_, flags_);
        e.PL_PRIV_RAW_FIELD = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogAlloc(void* ptr, uint32_t size) {
        // Memory events are too big to fit in one event (8 bytes pointer + 4 bytes size + 8 bytes date + location details), so they are spread on two.
        // First part: memory pointer and size
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, PL_STRINGHASH(""), PL_STRINGHASH(""), PL_EXTERNAL_STRINGS?0:"", PL_EXTERNAL_STRINGS?0:"", 0, PL_FLAG_TYPE_ALLOC_PART);
        e.extra = size;
        e.PL_PRIV_RAW_FIELD = (bigRawData_t)((uintptr_t)ptr);
        e.writeAck = 1;
        // Second part: location name and date
        ThreadContext_t* tCtx = &threadCtx;
        bi = globalCtx.bankAndIndex.fetch_add(1);
        if(tCtx->memLocQty==0) {
            EventInt& e2 = eventLogBase(bi, PL_STRINGHASH(""), PL_STRINGHASH(""), "", "", 0, PL_FLAG_TYPE_ALLOC);
            e2.PL_PRIV_RAW_FIELD = PL_GET_CLOCK_TICK_FUNC();
            e2.writeAck = 1;
        } else {
            const MemLocation& ml = tCtx->memLocStack[tCtx->memLocQty-1];
            EventInt& e2 = eventLogBase(bi, PL_STRINGHASH(""), ml.memHash, "", ml.memStr, 0, PL_FLAG_TYPE_ALLOC);
            e2.PL_PRIV_RAW_FIELD = PL_GET_CLOCK_TICK_FUNC();
            e2.writeAck = 1;
        }
        eventCheckOverflow(bi);
    }

    inline void eventLogDealloc(void* ptr) {
        // Memory events are too big to fit in one event (8 bytes pointer + 4 bytes size + 8 bytes date + location details), so they are spread on two.
        // First part: memory pointer
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, PL_STRINGHASH(""), PL_STRINGHASH(""), PL_EXTERNAL_STRINGS?0:"", PL_EXTERNAL_STRINGS?0:"", 0, PL_FLAG_TYPE_DEALLOC_PART);
        e.extra = 0;
        e.PL_PRIV_RAW_FIELD  = (bigRawData_t)((uintptr_t)ptr);
        e.writeAck = 1;
        // Second part: location name and date
        ThreadContext_t* tCtx = &threadCtx;
        bi = globalCtx.bankAndIndex.fetch_add(1);
        if(tCtx->memLocQty==0) {
            EventInt& e2 = eventLogBase(bi, PL_STRINGHASH(""), PL_STRINGHASH(""), "", "", 0, PL_FLAG_TYPE_DEALLOC);
            e2.PL_PRIV_RAW_FIELD = PL_GET_CLOCK_TICK_FUNC();
            e2.writeAck = 1;
        } else {
            const MemLocation& ml = tCtx->memLocStack[tCtx->memLocQty-1];
            EventInt& e2 = eventLogBase(bi, PL_STRINGHASH(""), ml.memHash, "", ml.memStr, 0,PL_FLAG_TYPE_DEALLOC);
            e2.PL_PRIV_RAW_FIELD = PL_GET_CLOCK_TICK_FUNC();
            e2.writeAck = 1;
        }
        eventCheckOverflow(bi);
    }

    // Idle            : threadId =PL_CSWITCH_CORE_NONE and sysThreadId=0
    // External process: threadId =PL_CSWITCH_CORE_NONE and sysThreadId=N strictly positif
    // Internal process: threadId!=PL_CSWITCH_CORE_NONE and sysThreadID=N/A
    inline void eventLogCSwitch(int threadId_, int sysThreadId_, int oldCoreId_, int newCoreId_, clockType_t timestamp_) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX];
        e.threadId     = (uint8_t)threadId_;
        e.flags        = PL_FLAG_TYPE_CSWITCH;
        e.lineNbr      = (uint16_t)((oldCoreId_<<8) | newCoreId_);
        e.extra        = sysThreadId_;
        e.filenameHash = PL_STRINGHASH("");
        e.nameHash     = PL_STRINGHASH("");
        e.filename     = "";
        e.name         = "";
        e.PL_PRIV_RAW_FIELD = (clockType_t)timestamp_;
        e.writeAck     = 1;
        eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, int32_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_S32);
        e.vInt  = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, uint32_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_U32);
        e.vU32  = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, int64_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
#if PL_COMPACT_MODEL==1
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_S32);
        e.vInt  = (int32_t)v;
#else
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_S64);
        e.vS64  = v;
#endif
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, uint64_t v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
#if PL_COMPACT_MODEL==1
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_U32);
        e.vU32  = (uint32_t)v;
#else
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_U64);
        e.vU64  = v;
#endif
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, float v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_FLOAT);
        e.vFloat = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, double v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
#if PL_COMPACT_MODEL==1
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_FLOAT);
        e.vFloat  = (float)v;
#else
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_DOUBLE);
        e.vDouble = v;
#endif
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, void* v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
#if PL_COMPACT_MODEL==1
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_U32);
#else
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_U64);
#endif
        e.PL_PRIV_RAW_FIELD  = (bigRawData_t)((uintptr_t)v);
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, const plString_t& v) {
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_STRING);
        e.vString = v;
        e.writeAck = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }

    inline void eventLogData(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_,
                             int lineNbr_, bool doSkipOverflowCheck_, const char* v) {
        const char* allocStr = getDynString(v);
        uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
        EventInt& e = eventLogBase(bi, filenameHash_? filenameHash_:1, nameHash_? nameHash_:1, filename_, name_, lineNbr_, PL_FLAG_TYPE_DATA_STRING);
        e.vString.hash  = 0;
        e.vString.value = allocStr;
        e.writeAck      = 1;
        if(!doSkipOverflowCheck_) eventCheckOverflow(bi);
    }


    // Logs

#define EVENT_LOG_STORE_PARAM_IMPL(paramType_t, storedParamType_t, extraCast, flagType) \
    template<typename... Args>                                          \
    inline void                                                         \
    eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, paramType_t value, Args... args) \
    {                                                                   \
        if(4+dataOffset+sizeof(storedParamType_t)>PL_PRIV_EVENTEXT_SIZE) { \
            EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX]; \
            e.lineNbr  = paramTypes;                                    \
            e.writeAck = 1;                                             \
            eventCheckOverflow(bi);                                     \
            bi = globalCtx.bankAndIndex.fetch_add(1);                   \
            EventInt& e2 = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX]; \
            e2.threadId  = getThreadId();                               \
            e2.flags     = PL_FLAG_TYPE_LOG_PARAM;                      \
            paramTypes   = 0;                                           \
            paramIdx     = 0;                                           \
            dataOffset   = 0;                                           \
        }                                                               \
        /* Update the u16 type area, which can hold up to 4 times 3 bits, the top bit being "is it the last param event?" */ \
        paramTypes |= flagType<<(3*paramIdx);                           \
        /* Raw write inside the event (C++ limits the unamed struct & union usage, which would have made it less hacky...) */ \
        uint8_t* payload = ((uint8_t*)(&(globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX])))+4; \
        *((storedParamType_t*)(payload+dataOffset)) = (storedParamType_t) extraCast value; \
        eventLogStoreParam(bi, paramTypes, paramIdx+1, dataOffset+sizeof(storedParamType_t), args...); \
    }


    inline void
    eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset)
    {
        (void)paramIdx; (void)dataOffset;
        EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX];
        e.lineNbr  = 0x8000 | paramTypes;
        e.writeAck = 1;
        eventCheckOverflow(bi);
    }

    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, int32_t  value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, uint32_t value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, int64_t  value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, uint64_t value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, float    value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, double   value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, void*    value, Args... args);
    template<typename... Args> void eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, const char* value, Args... args);
    EVENT_LOG_STORE_PARAM_IMPL(int32_t,  int32_t,  , PL_FLAG_TYPE_DATA_S32);
    EVENT_LOG_STORE_PARAM_IMPL(uint32_t, uint32_t, , PL_FLAG_TYPE_DATA_U32);
    EVENT_LOG_STORE_PARAM_IMPL(float,    float,    , PL_FLAG_TYPE_DATA_FLOAT);
#if PL_COMPACT_MODEL==1
    EVENT_LOG_STORE_PARAM_IMPL(void*,    uint32_t,   (uintptr_t), PL_FLAG_TYPE_DATA_U32);
    EVENT_LOG_STORE_PARAM_IMPL(int64_t,  int32_t,  , PL_FLAG_TYPE_DATA_S32);
    EVENT_LOG_STORE_PARAM_IMPL(uint64_t, uint32_t, , PL_FLAG_TYPE_DATA_U32);
    EVENT_LOG_STORE_PARAM_IMPL(double,   float,    , PL_FLAG_TYPE_DATA_FLOAT);
#else
    EVENT_LOG_STORE_PARAM_IMPL(void*,    uint64_t,   (uintptr_t), PL_FLAG_TYPE_DATA_U64);
    EVENT_LOG_STORE_PARAM_IMPL(int64_t,  int64_t,  , PL_FLAG_TYPE_DATA_S64);
    EVENT_LOG_STORE_PARAM_IMPL(uint64_t, uint64_t, , PL_FLAG_TYPE_DATA_U64);
    EVENT_LOG_STORE_PARAM_IMPL(double,   double,   , PL_FLAG_TYPE_DATA_DOUBLE);
#endif

    template<typename... Args> void
    eventLogStoreParam(uint32_t bi, uint16_t paramTypes, int paramIdx, int dataOffset, const char* value, Args... args)
    {
        if(dataOffset+8>PL_PRIV_EVENTEXT_SIZE-4) {
            EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX];
            e.lineNbr  = paramTypes;
            e.writeAck = 1;
            eventCheckOverflow(bi);
            bi = globalCtx.bankAndIndex.fetch_add(1);
            EventInt& e2 = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX];
            e2.threadId  = getThreadId();
            e2.flags     = PL_FLAG_TYPE_LOG_PARAM;
            paramTypes = 0;
            paramIdx = 0;
            dataOffset = 0;
        }
        /* Update the u16 type area, which can hold up to 4 times 3 bits, the top bit being "is it the last param event?" */
        paramTypes |= PL_FLAG_TYPE_DATA_STRING<<(3*paramIdx);
        /* Raw write inside the event (C++ limits the unamed struct & union usage, which would have made it less hacky...) */
        uint8_t* payload = ((uint8_t*)&globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX])+4;
        *((const char**)(payload+dataOffset)) = getDynString(value);
        eventLogStoreParam(bi, paramTypes, paramIdx+1, dataOffset+8, args...);
    }

    template<typename... Args> void eventLogConsoleDisplay(plLogLevel level, const char* category_, const char* format, ...);

    template<typename... Args>
    inline void
    eventLogLog(hashStr_t formatHash_, hashStr_t categoryHash_, const char* format_, const char* category_, plLogLevel level, Args... args)
    {
        if(PL_IS_ENABLED_() && level>=globalCtx.minLogLevelRecord) {
            eventLogRaw(formatHash_, categoryHash_, format_, category_, (int)level, PL_STORE_COLLECT_CASE_, PL_FLAG_TYPE_LOG, PL_GET_CLOCK_TICK_FUNC());
            uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
            EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX]; \
            e.threadId  = getThreadId();
            e.flags     = PL_FLAG_TYPE_LOG_PARAM;
            eventLogStoreParam(bi, 0, 0, 0, args...); // Recursive storage of provided parameters
        }
#if PL_EXTERNAL_STRINGS==0
        if(level>=globalCtx.minLogLevelConsole) {
            eventLogConsoleDisplay(level, category_, format_, args...);
        }
#endif
    }


    inline void
    eventLogLog(hashStr_t formatHash_, hashStr_t categoryHash_, const char* format_, const char* category_, plLogLevel level)
    {
        if(PL_IS_ENABLED_() && level>=globalCtx.minLogLevelRecord) {
            eventLogRaw(formatHash_, categoryHash_, format_, category_, 0x8000 | (int)level, PL_STORE_COLLECT_CASE_, PL_FLAG_TYPE_LOG, PL_GET_CLOCK_TICK_FUNC());
        }
#if PL_EXTERNAL_STRINGS==0
        if(level>=globalCtx.minLogLevelConsole) {
            eventLogConsoleDisplay(level, category_, format_);
        }
#endif
    }


    // Thread declaration function
    inline void declareThread(hashStr_t nameHash_, const char* name_, bool doSkipOverflowCheck_, bool isDynName) {
        plAssert(nameHash_ || name_);
        ThreadContext_t* tCtx = &threadCtx;
        if(tCtx->id==0xFFFFFFFF) plPriv::getThreadId();  // Ensure that the thread owns an internal Id
        if(tCtx->id>=PL_MAX_THREAD_QTY || plPriv::globalCtx.threadInfos[tCtx->id].nameHash!=0) return;

        // Store the thread name for persistency across delayed or multiple starts
        plPriv::ThreadInfo_t& ti = plPriv::globalCtx.threadInfos[tCtx->id];
        int nameLength = name_? (int)strlen(name_) : 0;
        if(nameLength>PL_DYN_STRING_MAX_SIZE-1) nameLength = PL_DYN_STRING_MAX_SIZE-1;
        if(nameLength) memcpy(ti.name, name_, nameLength+1);
        ti.name[PL_DYN_STRING_MAX_SIZE-1] = 0;
        ti.nameHash = nameHash_? nameHash_ : plPriv::hashString(name_);

        // Send the declared thread name only if the service is running. Yes, there is a race condition leading to a harmless double thread name sending
        if(PL_IS_ENABLED_()) {
            if(isDynName) plPriv::eventLogRawDynName(PL_STRINGHASH(""),     PL_EXTERNAL_STRINGS?0:"", name_, 0, doSkipOverflowCheck_, PL_FLAG_TYPE_THREADNAME, 0);
            else          plPriv::eventLogRaw(PL_STRINGHASH(""), nameHash_, PL_EXTERNAL_STRINGS?0:"", name_, 0, doSkipOverflowCheck_, PL_FLAG_TYPE_THREADNAME, 0);
        }

#if PL_VIRTUAL_THREADS==1
        // With enabled virtual threads, the name hash is also stored in the TLS so that we can use it as a "fiber worker resource" name
        plPriv::threadCtx.realRscNameHash = plPriv::hashString(name_);
#endif
    }

    // Automatic scope closing using RAII
    struct TimedScope {
        TimedScope(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_, int lineNbr_) :
            filenameHash(filenameHash_), nameHash(nameHash_), filename(filename_), name(name_), lineNbr(lineNbr_)
        { if(PL_IS_ENABLED_()) eventLogRaw(filenameHash_, nameHash_, filename_, name_, lineNbr_, false, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); }
        ~TimedScope(void)
        { if(PL_IS_ENABLED_()) eventLogRaw(filenameHash, nameHash, filename, name, lineNbr, false, PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); }
        hashStr_t   filenameHash;
        hashStr_t   nameHash;
        const char* filename;
        const char* name;
        int         lineNbr;
    };
    struct TimedScopeDyn {
        TimedScopeDyn(hashStr_t filenameHash_, const char* filename_, const char* name_, int lineNbr_) :
            filenameHash(filenameHash_), filename(filename_), name(name_, 0), lineNbr(lineNbr_)
        { if(PL_IS_ENABLED_()) eventLogRawDynName(filenameHash_, filename_, name_, lineNbr_, false, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); }
        TimedScopeDyn(hashStr_t filenameHash_, const char* filename_, plString_t name_, int lineNbr_) :
            filenameHash(filenameHash_), filename(filename_), name(name_), lineNbr(lineNbr_)
        { if(PL_IS_ENABLED_()) eventLogRawDynName(filenameHash_, filename_, name_, lineNbr_, false, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC()); }
        ~TimedScopeDyn(void)
        { if(PL_IS_ENABLED_()) {
                if(name.hash) eventLogRawDynName(filenameHash, filename, name,       lineNbr, false, PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC());
                else          eventLogRawDynName(filenameHash, filename, name.value, lineNbr, false, PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC());
            }
        }
        hashStr_t   filenameHash;
        const char* filename;
        plString_t  name;
        int         lineNbr;
    };
    struct TimedLock {
        TimedLock(hashStr_t filenameHash_, hashStr_t nameHash_, const char* filename_, const char* name_, int lineNbr_, int state_) :
            filenameHash(filenameHash_), nameHash(nameHash_), filename(filename_), name(name_), lineNbr(lineNbr_)
        { if(PL_IS_ENABLED_()) eventLogRaw(filenameHash_, nameHash_, filename_, name_, lineNbr_, false, (state_)? PL_FLAG_TYPE_LOCK_ACQUIRED : PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); }
        ~TimedLock(void)
        { if(PL_IS_ENABLED_()) eventLogRaw(filenameHash, nameHash, filename, name, lineNbr, false, PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); }
        hashStr_t   filenameHash;
        hashStr_t   nameHash;
        const char* filename;
        const char* name;
        int         lineNbr;
    };
    struct TimedLockDyn {
        TimedLockDyn(hashStr_t filenameHash_, const char* filename_, const char* name_, int lineNbr_, int state_) :
            filenameHash(filenameHash_), filename(filename_), name(name_, 0), lineNbr(lineNbr_)
        { if(PL_IS_ENABLED_()) eventLogRawDynName(filenameHash_, filename_, name_, lineNbr_, false, (state_)? PL_FLAG_TYPE_LOCK_ACQUIRED : PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); }
        TimedLockDyn(hashStr_t filenameHash_, const char* filename_, plString_t name_, int lineNbr_, int state_) :
            filenameHash(filenameHash_), filename(filename_), name(name_), lineNbr(lineNbr_)
        { if(PL_IS_ENABLED_()) eventLogRawDynName(filenameHash_, filename_, name_, lineNbr_, false, (state_)? PL_FLAG_TYPE_LOCK_ACQUIRED : PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC()); }
        ~TimedLockDyn(void)
        { if(PL_IS_ENABLED_()) {
                if(name.hash) eventLogRawDynName(filenameHash, filename, name,       lineNbr, false, PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC());
                else          eventLogRawDynName(filenameHash, filename, name.value, lineNbr, false, PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC());
            }
        }
        hashStr_t   filenameHash;
        const char* filename;
        plString_t  name;
        int         lineNbr;
    };

} // namespace plPriv

#else // // if USE_PL==1 && PL_NOEVENT==0
#define PL_IS_ENABLED_() false

#endif // if USE_PL==1 && PL_NOEVENT==0



//-----------------------------------------------------------------------------
// [IMPLEMENTATION] Exported declaration
//-----------------------------------------------------------------------------

#if (PL_IMPLEMENTATION==1 && USE_PL==1 && (PL_NOCONTROL==0 || PL_NOEVENT==0)) || PL_EXPORT==1

namespace plPriv {

    // Remote CLI returned status
    enum plRemoteStatus { PL_OK, PL_ERROR, PL_CLI_ERROR };

    // Block types of data exchange with the server
    // All block start with the following header (big endian):
    //   <2B: synchro magic>: 'P' 'L'
    //   <2B: bloc type>
    enum DataType {
        PL_DATA_TYPE_STRING,    // Notif: <4B: string quantity> [ <8B string hash> <null terminated string> ]*(string qty)
        PL_DATA_TYPE_EVENT,     // Notif: <4B: event qty> [ 16B: EventExt structure in local endianness ]*(event qty)
        PL_DATA_TYPE_EVENT_AUX, // Same as PL_DATA_TYPE_EVENT, but semantically different (its reception is not counted as a collection loop)
        PL_DATA_TYPE_CONTROL    // Both ways: <4B: command byte qty> [1B: bytes]*(command byte qty)
    };

    // Remote control commands
    // All requests and answers start with the following header (big endian):  <2B: command type> then add what is described below
    // All strings are null terminated
    enum RemoteCommandType {
        // Unsollicited notification (so without request)
        PL_NTF_FROZEN_THREAD,    // Notif: <8B thread index bitmap>
        PL_NTF_DECLARE_CLI,      // Notif: <2B cli quantity> [ <2B: name string idx> <2B: specParams string idx> <2B: description string idx> ]*(cli quantity)
        // Commands (i.e. request + response)
        PL_CMD_SET_FREEZE_MODE,  // Request: <1B: 0=off (default) 1=on>   Response: <2B: plRemoteStatus>
        PL_CMD_STEP_CONTINUE,    // Request: <8B thread index bitmap>     Response: <2B: plRemoteStatus>
        PL_CMD_SET_MAX_LATENCY,  // Request: <2B latency ms>              Response: <2B: plRemoteStatus>
        PL_CMD_KILL_PROGRAM,     // Request: None                         Response: None (killed before sent)
        PL_CMD_CALL_CLI,         // Request:  <2B command  quantity> [ <full command string> ]*(command qty)
                                 // Response: <2B response quantity> [ <2B plRemoteStatus> <response string> ]*(multiple, until response byte qty reached)
                                 // Note: If the response buffer is full, not all commands are called so response qty<=command quantity
    };

    // Event structure for external world
    // Compact model (12 bytes)
     struct EventExtCompact {
	    // Header: 4 bytes
        uint8_t  threadId;
        uint8_t  flags;
        uint16_t lineNbr;
		// 4 bytes
		union {
			struct {
				union {
					uint16_t filenameIdx;
					struct {
						uint8_t  prevCoreId;  // Context switch
						uint8_t  newCoreId;
					};
				} ;
				uint16_t nameIdx;
			};
            uint32_t memSize;  // Memory case only. The unused fields filenameIdx and nameIdx are overriden
        };
		// Value: 4 bytes
        union {
            int32_t  vInt;
            uint32_t vU32;
            float    vFloat;
            uint32_t vStringIdx;
        };
    };

    // Full model (24 bytes)
    struct EventExtFull {
	    // Header: 4 bytes
        uint8_t  threadId;
        uint8_t  flags;
        uint16_t lineNbr;

        // 4 bytes
        union {
            uint32_t filenameIdx;
            struct {
                uint8_t  prevCoreId;  // Context switch
                uint8_t  newCoreId;
                uint16_t reserved;
            };
        };
        // 4 bytes
        union {
            uint32_t nameIdx;
            uint32_t memSize;  // Memory case only
        };
        // 4 bytes
        uint32_t reserved2;  // Explicit padding for portability
        // 8 bytes
        union {
            int32_t  vInt;
            uint32_t vU32;
            int64_t  vS64;
            uint64_t vU64;
            float    vFloat;
            double   vDouble;
            uint32_t vStringIdx;
        };
    };

#if PL_COMPACT_MODEL==1 && PL_EXPORT==0
    typedef EventExtCompact EventExt;
#else
    typedef EventExtFull    EventExt;
#endif

} // namespace plPriv

#endif //if (PL_IMPLEMENTATION==1 && USE_PL==1 && (PL_NOCONTROL==0 || PL_NOEVENT==0)) || PL_EXPORT==1




// =======================================================================================================
// =======================================================================================================
// From here: implementation only
// =======================================================================================================
// =======================================================================================================

#if USE_PL==1 && PL_IMPLEMENTATION==1

#include <mutex>              // Used with conditional variable
#include <condition_variable> // For the thread freeze feature and Tx thread synchro

// Stack trace
// ============
#if PL_IMPL_STACKTRACE==1

#if defined(__unix__)
#define UNW_LOCAL_ONLY
#include <elfutils/libdwfl.h>  // Elf reading (need package libdw-dev)
#include <libunwind.h>         // Stack unwinding (need package libunwind-dev)
#include <cxxabi.h>            // For demangling names
#include <cstring>             // For strcmp
#include <unistd.h>            // For getpid()
#endif // if defined(__unix__)

#if defined(_WIN32)
#include <errhandlingapi.h> // For the HW exceptions
#include <dbghelp.h>        // For the symbol decoding
#pragma comment(lib, "DbgHelp.lib")
#endif // if defined(_WIN32)

#endif // if PL_IMPL_STACKTRACE==1


#if PL_NOCONTROL==0 || PL_NOEVENT==0

#if PL_IMPL_AUTO_INSTRUMENT==1 && PL_IMPL_AUTO_INSTRUMENT_PIC==1 && defined(__unix__)
#include <dlfcn.h>  // For Dl_info and dladdr
#endif

// Networking
// ==========
#if defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PL_PRIV_IS_SOCKET_VALID(s) ((s)>=0)
#define PL_PRIV_SOCKET_ERROR (-1)
#endif // if defined(__unix__)

#if defined(_WIN32)
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <ws2tcpip.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#define PL_PRIV_IS_SOCKET_VALID(s) ((SOCKET)(s)!=INVALID_SOCKET)
#define PL_PRIV_SOCKET_ERROR SOCKET_ERROR
#endif // if defined(_WIN32)

#endif // if PL_NOCONTROL==0 || PL_NOEVENT==0

// Tracing of kernel scheduler context switch (requires root/administrator privileges to work)
// ===========================================================================================
#if PL_IMPL_CONTEXT_SWITCH==1 && PL_NOEVENT==0

#if defined(__unix__)
#include <fcntl.h>  // flags for open
#include <poll.h>   // pollfd and poll
#endif // if defined(__unix__)

#if defined(_WIN32)
#define INITGUID // "Causes definition of SystemTraceControlGuid in evntrace.h"
#include <windows.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <evntcons.h>
#pragma comment(lib,"Advapi32.lib")
#endif // if defined(_WIN32)

#endif // if PL_IMPL_CONTEXT_SWITCH==1 && PL_NOEVENT==0


// Group for Palanteer client instrumentation (default is enabled)
#ifndef PL_GROUP_PL_VERBOSE
#define PL_GROUP_PL_VERBOSE 1
#endif

// Group for Palanteer windows context switch callback tracing (default is disabled)
#if defined(_WIN32)
#ifndef PL_GROUP_PL_VERBOSE_CS_CBK
#define PL_GROUP_PL_VERBOSE_CS_CBK 0
#endif
#endif


namespace plPriv {

    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] Simple assertions
    // =======================================================================================================

#if PL_NOASSERT==0

#if PL_EXTERNAL_STRINGS==0
    void PL_NOINLINE
    failedAssertSimple(const char* filename, int lineNbr, const char* function, const char* condition)
    {
        char infoStr[1024];
        snprintf(infoStr, sizeof(infoStr), "[PALANTEER] Assertion failed: %s\n  On function: %s\n  On file    : %s(%d)\n", condition, function, filename, lineNbr);
        plCrash(infoStr);
    }
#else
    void PL_NOINLINE
    failedAssertSimpleEs(hashStr_t filenameHash, int lineNbr, hashStr_t conditionHash)
    {
        char infoStr[1024];
        snprintf(infoStr, sizeof(infoStr), "[PALANTEER] Assertion failed: @@%016" PL_PRI_HASH "@@\n  On file @@%016" PL_PRI_HASH "@@(%d)\n",
                 conditionHash, filenameHash, lineNbr);
        plCrash(infoStr);
    }
#endif // if PL_EXTERNAL_STRINGS==0
#endif // if PL_NOASSERT==0


    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] Helpers
    // =======================================================================================================

    // Simple flat hashmap with linear open addressing.
    // Requirements: simple, fast, with few allocations, and specialized for our string->index problem:
    //   no deletion, key=hash, hash is never zero, hash is 'well spread enough' to avoid clusters,
    //   value is a trivially copyable structure, we never insert twice the same key, table size is a power of two
    template <class T>
    class FlatHashTable {
    public:
        FlatHashTable(int size=PL_IMPL_MAX_EXPECTED_STRING_QTY) {  // This initial value should allow a control on the potential reallocation
            int sizePo2 = 1; while(sizePo2<size) sizePo2 *= 2;
            rehash(sizePo2);
        } // Start with a reasonable size (and 32 KB on a 64 bits platform)
        ~FlatHashTable(void) { delete[] _nodes; }
        void clear(void) { _size = 0; for(int i=0; i<_maxSize; ++i) _nodes[i].hash = 0; }
        void insert(hashStr_t hash, T value) {
            unsigned int idx = (unsigned int)hash&_mask;
            while(PL_UNLIKELY(_nodes[idx].hash)) idx = (idx+1)&_mask; // Always stops because load factor < 1
            _nodes[idx].hash  = hash; // Never zero, so "non empty"
            _nodes[idx].value = value;
            _size += 1;
            if(_size*3>_maxSize*2) rehash(2*_maxSize); // Max load factor is 0.66
        }
        bool find(hashStr_t hash, T& value) {
            unsigned int idx = (unsigned int)hash&_mask;
            while(1) { // Always stops because load factor <= 0.66
                if(_nodes[idx].hash==hash) { value = _nodes[idx].value; return true; }
                if(_nodes[idx].hash==0) return false; // Empty node
                idx = (idx+1)&_mask;
            }
            return false; // Never reached
        }
        bool exist(hashStr_t hash) {
            unsigned int idx = (unsigned int)hash&_mask;
            while(1) { // Always stops because load factor <= 0.66
                if(_nodes[idx].hash==hash) return true;
                if(_nodes[idx].hash==0)    return false; // Empty node
                idx = (idx+1)&_mask;
            }
            return false; // Never reached
        }
        bool replace(hashStr_t hash, const T& newValue) {
            unsigned int idx = (unsigned int)hash&_mask;
            while(1) { // Always stops because load factor <= 0.66
                if(_nodes[idx].hash==hash) { _nodes[idx].value = newValue; return true; }
                if(_nodes[idx].hash==0) return false; // Empty node
                idx = (idx+1)&_mask;
            }
            return false; // Never reached
        }
        void rehash(int maxSize) {
            int   oldSize = _maxSize;
            Node* old     = _nodes;
            _nodes   = new Node[maxSize]; // 'hash' are set to zero (=empty)
            _maxSize = maxSize;
            _mask    = (unsigned int)(maxSize-1);
            _size    = 0;
            for(int i=0; i<oldSize; ++i) { // Transfer the previous filled nodes
                if(old[i].hash==0) continue;
                insert(old[i].hash, old[i].value);
            }
            delete[] old;
        }
    private:
        struct Node {
            hashStr_t hash = 0; // Hash and key are the same here
            T         value;
        };
        Node* _nodes   = 0;
        unsigned int _mask = 0;
        int   _size    = 0;
        int   _maxSize = 0;
        FlatHashTable(const FlatHashTable& other);     // To please static analyzers
        FlatHashTable& operator=(FlatHashTable other); // To please static analyzers
    };



    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] CLI manager
    // =======================================================================================================

#if PL_NOCONTROL==0

    class CliManager {
    public:
        CliManager(void) : _cio(PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY, PL_IMPL_CLI_MAX_PARAM_QTY),
                           _hashToIdx(2*PL_IMPL_MAX_CLI_QTY) { }  // Reallocation controlled by the initial size

        void registerCli(plCliHandler_t handler, const char* name, const char* specParams, const char* description,
                         hashStr_t nameHash, hashStr_t specParamsHash, hashStr_t descriptionHash) {
            plAssert(handler && nameHash && specParams && specParamsHash && descriptionHash,
                     "One of these parameter is null", handler, nameHash, (void*)specParams, specParamsHash, descriptionHash);
            Cli newCli;
            newCli.handler                 = handler;
            newCli.strings.name            = name;
            newCli.strings.specParams      = specParams;
            newCli.strings.description     = description;
            newCli.strings.nameHash        = nameHash;
            newCli.strings.specParamsHash  = specParamsHash;
            newCli.strings.descriptionHash = descriptionHash;

            // Parse the parameters definition as a space separated list of '<name>=<type>', with optional '[[default parameters]]' just after the type
            const char* s = specParams; skipSpace(s);
            while(*s) {
                // Parameter name
                const char* sNameStart = getWord(s, true);
                plAssert(*s=='=', "The syntax of CLI parameters is a comma separated list of 'name=<type>', with <type> among int,float or string", name);
                plAssert(newCli.paramQty<PL_IMPL_CLI_MAX_PARAM_QTY, "Maximum CLI parameter quantity exceeded. Please increase PL_IMPL_CLI_MAX_PARAM_QTY, which is currently",
                         PL_IMPL_CLI_MAX_PARAM_QTY, "for the CLI: ", name);
                newCli.parameters[newCli.paramQty].specStartIdx = (uint16_t)(sNameStart-specParams);
                newCli.parameters[newCli.paramQty].length       = (uint16_t)(s-sNameStart);

                // Parameter type
                const char* sTypeStart = getWord(++s, false, true);
                if     (s-sTypeStart==3 && !strncmp(sTypeStart, "int",    3)) newCli.parameters[newCli.paramQty++].type = PL_INTEGER;
                else if(s-sTypeStart==5 && !strncmp(sTypeStart, "float",  5)) newCli.parameters[newCli.paramQty++].type = PL_FLOAT;
                else if(s-sTypeStart==6 && !strncmp(sTypeStart, "string", 6)) newCli.parameters[newCli.paramQty++].type = PL_STRING;
                else plAssert(0, "Allowed parameter types are 'int', 'float' or 'string'", name);
                skipSpace(s);

                // Parameter optional default value
                if(*s=='[' && *(s+1)=='[') {
                    Parameter& p = newCli.parameters[newCli.paramQty-1];
                    p.hasDefaultValue = 1;
                    if(p.type==PL_INTEGER) {
                        int readQty = sscanf(s, "[[%" PRId64 "]]", &p.defaultValue);
                        plAssert(readQty==1, "Unable to parse the integer default value of the parameter", newCli.paramQty-1);
                        int tmp; getString(s, tmp);
                    } else if(p.type==PL_FLOAT) {
                        int readQty = sscanf(s, "[[%lf]]", (double*)&p.defaultValue);
                        plAssert(readQty==1, "Unable to parse the float default value of the parameter", newCli.paramQty-1);
                        int tmp; getString(s, tmp);
                    } else if(p.type==PL_STRING) {
                        int stringLength = 0;
                        p.defaultValue = (uintptr_t)getString(s, stringLength); // Points inside the "spec".
                        p.defaultStringLength = (uint16_t)(stringLength+1); // Add the null termination
                    }
                    skipSpace(s);
                }
            }

            // Store the new remote CLI
            uint32_t  cliIndex;
            _storageMx.lock(); // CLI declaration can be done from any thread
            if(_hashToIdx.find(nameHash, cliIndex)) {
                _storageMx.unlock();
                plAssert(0, "This remote CLI name has been declared twice", name);
            }
            _hashToIdx.insert(nameHash, _cliArray.size());
            if(_cliArray.free_space()<=0) {
                _storageMx.unlock();
                plAssert(0, "Maximum CLI quantity exceeded. Please increase PL_IMPL_MAX_CLI_QTY, which is currently", PL_IMPL_MAX_CLI_QTY);
            }
            _cliArray.resize(_cliArray.size()+1);
            _cliArray[_cliArray.size()-1] = newCli;
            _storageMx.unlock();
        }

        const char* execute(const char* request, plRemoteStatus& status) {
            plAssert(request, "CLI request string is empty");
            _paramBuffer.clear();
            _cio._response.clear();
            _cio._response[0] = 0;
            _cio._execStatus = true;
            for(int i=0; i<PL_IMPL_CLI_MAX_PARAM_QTY; ++i) _cio._paramTypes[i] = PL_TYPE_QTY;
#define CLI_INPUT_ERROR(fmt, ...) { status = PL_ERROR; _cio.setErrorState(fmt, __VA_ARGS__); return &_cio._response[0]; }

            // Get the associated remote CLI
            const char* s          = request; skipSpace(s);
            const char* sNameStart = getWord(s);
            hashStr_t   hashedName = hashString(sNameStart, (int)(s-sNameStart));
            uint32_t    cliIndex;
            Cli* cli = 0;
            {   // Lock scope
                std::unique_lock<std::mutex> lk(_storageMx); // Lock the access to the CLI list, as we may have simultaneous new CLI declaration
                if(!_hashToIdx.find(hashedName, cliIndex))
                    CLI_INPUT_ERROR("Unknown command '%.*s'", (int)(s-sNameStart), sNameStart);
                cli = &_cliArray[cliIndex];
                _cio._cliNameHash = cli->strings.nameHash;
                _cio._paramQty    = cli->paramQty;

                // Prepare the default parameters and the string buffer reservation
                int foundParamQty = 0;
                for(int paramIdx=0; paramIdx<cli->paramQty; ++paramIdx) {
                    const Parameter& p = cli->parameters[paramIdx];
                    if(!p.hasDefaultValue) continue;
                    if(p.type==PL_STRING) {
                        _cio._paramValues[paramIdx] = (uintptr_t)&_paramBuffer[_paramBuffer.size()];
                        _paramBuffer.resize(_paramBuffer.size()+p.defaultStringLength);
                        memcpy(&_paramBuffer[_paramBuffer.size()-p.defaultStringLength], (char*)p.defaultValue, p.defaultStringLength);
                        _paramBuffer[_paramBuffer.size()-1] = 0;
                    } else _cio._paramValues[paramIdx] = p.defaultValue;
                    _cio._paramTypes[paramIdx] = p.type;
                    ++foundParamQty;
                }

                // Parse the parameters
                skipSpace(s);
                while(*s) {
                    // Find the parameter in the specification
                    skipSpace(s);
                    sNameStart = getWord(s, true);
                    if(*s!='=')
                        CLI_INPUT_ERROR("Parameter '%.*s' has no value ('=' missing)", (int)(s-sNameStart), sNameStart);
                    int paramIdx = -1;
                    for(int i=0; i<cli->paramQty; ++i) {
                        const Parameter& p = cli->parameters[i];
                        if(!strncmp(sNameStart, &cli->strings.specParams[p.specStartIdx], s-sNameStart)) {
                            if(paramIdx==-1) paramIdx = i; // First match found
                            else CLI_INPUT_ERROR("Ambiguous parameter '%.*s'", (int)(s-sNameStart), sNameStart); // Second match found, so ambiguous param name...
                        }
                    }
                    if(paramIdx<0)
                        CLI_INPUT_ERROR("Unknown parameter '%.*s'", (int)(s-sNameStart), sNameStart);
                    ++s;

                    // Parse the value
                    if(_cio._paramTypes[paramIdx]==PL_TYPE_QTY) {
                        _cio._paramTypes[paramIdx] = cli->parameters[paramIdx].type;
                        ++foundParamQty;
                    }
                    if(cli->parameters[paramIdx].type==PL_INTEGER) {
                        if(sscanf(s, "%" PRId64, &_cio._paramValues[paramIdx])!=1 || !skipValue(s))
                            CLI_INPUT_ERROR("Parameter '%.*s' is not a valid integer", (int)(s-sNameStart-1), sNameStart);
                    } else if(cli->parameters[paramIdx].type==PL_FLOAT) {
                        if(sscanf(s, "%lf", (double*)&_cio._paramValues[paramIdx])!=1 || !skipValue(s))
                            CLI_INPUT_ERROR("Parameter '%.*s' is not a valid float", (int)(s-sNameStart-1), sNameStart);
                    } else if(cli->parameters[paramIdx].type==PL_STRING) {
                        _cio._paramValues[paramIdx] = (uintptr_t)&_paramBuffer[_paramBuffer.size()];
                        int stringLength = 0;
                        const char* s2 = getString(s, stringLength);
                        _paramBuffer.resize(_paramBuffer.size()+(stringLength+1)); // +1 for null terminaison
                        memcpy(&_paramBuffer[_paramBuffer.size()-(stringLength+1)], s2, stringLength);
                        _paramBuffer[_paramBuffer.size()-1] = 0;
                    } else plAssert(0, "Bug, unknown type");
                    skipSpace(s);
                }
                if(foundParamQty!=cli->paramQty)
                    CLI_INPUT_ERROR("%d parameters are missing", cli->paramQty-foundParamQty);

            } // End of locking on CLI list

            // Call the handler and store the returned status
            cli->handler(_cio);
            status = _cio._execStatus? PL_OK : PL_CLI_ERROR;

            // Return the response string
            return &_cio._response[0];
        }

        struct CliStrings { // 3*8+3*8 = 48 bytes worse case
            const char* name        = 0;
            const char* specParams  = 0;
            const char* description = 0;
            hashStr_t   nameHash;
            hashStr_t   specParamsHash;
            hashStr_t   descriptionHash;
        };
        const CliStrings& getCliStrings(int index) const { plAssert(index>=0 && index<_cliArray.size(), index, _cliArray.size()); return _cliArray[index].strings; }
        int               getCliQty(void)          const { std::unique_lock<std::mutex> lk(_storageMx); return _cliArray.size(); }

    private:
        void skipSpace(const char*& s) {
            while(*s==' ') ++s;
        }
        const char* getWord(const char*& s, bool doStopOnEqual=false, bool doStopOnBracket=false) {
            const char* s2 = s; while(*s && *s!=' ' && (!doStopOnEqual || *s!='=') && (!doStopOnBracket || *s!='[')) ++s; return s2;
        }
        bool skipValue(const char*& s) { // Skip a numerical value (float or decimal integer) and return if there is a valid separator afterwards
            if(*s=='-') ++s;
            while(*s=='.' || (*s>='0' && *s<='9')) ++s;
            return (*s==0 || *s==' ');
        }
        const char* getString(const char*& s, int& length) {
            const char* s2 = s; length = 0;
            bool isExtended = (*s2=='[') && (*(s2+1)=='[');           // An extended string is [[list of words]].
            if(!isExtended) { getWord(s); length = (int)(s-s2); return s2; } // The 'not extended string' is a simple word
            s2 += 2; s += 2;
            while(*s && !(*s==']' && *(s+1)==']')) ++s;
            length = (int)(s-s2); s += 2;
            return s2;
        }
        struct Parameter { // 16 bytes
            uint16_t specStartIdx, length; // Name of the parameter, taken from the parameter specification string
            plCliParamTypes type;
            uint8_t  hasDefaultValue = 0;     // Used as a boolean and has a controllable and explicit size.
            uint16_t defaultStringLength = 0; // Used only in case of default string param
            uint64_t defaultValue;
        };
        struct Cli { // 48 + 8 + 4(+4 alignment) + paramQty*16 = 64+16*PL_IMPL_CLI_MAX_PARAM_QTY
            CliStrings     strings;
            plCliHandler_t handler  = 0;
            int            paramQty = 0;
            Parameter      parameters[PL_IMPL_CLI_MAX_PARAM_QTY];
        };

        plCliIo        _cio;
        FlatHashTable<uint32_t> _hashToIdx;
        Array<Cli,  PL_IMPL_MAX_CLI_QTY> _cliArray;
        Array<char, PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY> _paramBuffer;  // Working buffer for execution (to store string parameters)
        mutable std::mutex _storageMx;
    };
#endif // if PL_NOCONTROL==0


    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] Global context
    // =======================================================================================================

    // Signals handler type
    typedef void (*plSignalHandler_t)(int);

#if defined(__unix__)
    typedef int socket_t; // Abstract type needed for easier cross-platform support
#endif
#if defined(_WIN32)
    typedef uint32_t socket_t ;
#endif
#if defined(_WIN32) && PL_IMPL_STACKTRACE==1
    extern "C" { typedef unsigned long (__stdcall *rtlWalkFrameChain_t)( void**, unsigned long, unsigned long ); }
#endif
#if PL_NOEVENT==0 && PL_VIRTUAL_THREADS==1
    struct VirtualThreadCtx {
        bool      isSuspended;
        bool      isBeginSent;
        hashStr_t nameHash;
    };
#endif

    constexpr int SWITCH_CTX_BUFFER_SIZE = 64*1024;

    // Global context for logging
    GlobalContext_t globalCtx(PL_IMPL_DYN_STRING_QTY);

    // Thread local context for logging
    thread_local ThreadContext_t threadCtx;

    // Implementation-only context
    static struct {
        // Start parameters
        plMode  mode;
        char    filename [256]  = "record.pltraw";
        char    serverAddr[64]  = "127.0.0.1";
        int     serverPort      = 59059;
        plStats stats = { 0, 0, 0, 0, 0, 0, 0, 0 };
        bool    doNotUninit = false; // Emergency exit shall not clean ressources
        bool    hasAutoInstrument = false;  // Can be overriden by language glue layer
        // Threads and connection
#if PL_NOCONTROL==0 || PL_NOEVENT==0
        std::thread*     threadServerTx = 0;
        std::thread*     threadServerRx = 0;
        std::atomic<int> threadServerFlagStop = { 0 };
#if PL_IMPL_CUSTOM_COM_LAYER==0
        FILE*            fileHandle = 0;
        socket_t         serverSocket = (socket_t)PL_PRIV_SOCKET_ERROR;
#if defined(_WIN32)
        bool             wsaInitialized = false;
#endif
#endif // if PL_IMPL_CUSTOM_COM_LAYER==0
        // Tx thread init synchronization
        bool                    rxIsStarted = false;
        bool                    txIsStarted = false;
        std::mutex              threadInitMx;
        std::condition_variable threadInitCv;
        std::condition_variable threadInitCvTx;
        std::mutex              txThreadSyncMx;
        std::condition_variable txThreadSyncCv;
        int                     txThreadId = -1; // To avoid locks when this TX thread asserts or crashes
        // Data collection
        double        tickToNs = 1.;
        clockType_t   lastSentEventBufferTick = 0;
        clockType_t   lastWrapCheckDateTick = 0;
        uint32_t      lastDateWrapQty       = 0;
        clockType_t   bankPreDateTick   [2] = {0, 0};
        uint32_t      bankPreDateWrapQty[2] = {0, 0};
        uint8_t*      allocCollectBuffer = 0;
        uint8_t*      sendBuffer = 0;
        FlatHashTable<uint32_t> lkupStringToIndex;
        uint32_t      stringUniqueId = 0;
        uint32_t      prevBankAndIndex = 1UL<<31;
        double        maxSendingLatencyNs = 100000000.;
        std::mutex    logDisplayMx;
        // Automatic instrumentation
#if PL_IMPL_AUTO_INSTRUMENT==1
        char*     baseVirtualAddress = (char*)-1;
        hashStr_t hashedAppName = 0;
#endif
        // Communication buffers
#if PL_NOCONTROL==0
        uint8_t reqBuffer[PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY];
        uint8_t rspBuffer[PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY];
#else
        uint8_t reqBuffer[1];
        uint8_t rspBuffer[1];
#endif
        Array<uint8_t, PL_IMPL_STRING_BUFFER_BYTE_QTY> strBuffer;   // For batched strings sending
        std::atomic<int> rspBufferSize = { 0 };
        int              lastSentCliQty = 0;
#if PL_NOCONTROL==0
        CliManager cliManager;
#endif
        // Freeze feature
        uint64_t                frozenLastThreadBitmap = 0; // To track state changes
        std::atomic<uint64_t>   frozenThreadBitmap  = { 0 };
        std::atomic<uint64_t>   frozenThreadBitmapChange = { 0 };
        std::atomic<int>        frozenThreadEnabled = { 0 };
        std::mutex              frozenThreadMx; // Shared with all thread's conditional variables
        std::condition_variable frozenThreadCv[PL_MAX_THREAD_QTY]; // Used to freeze threads independently
        // Context switches
        bool   cswitchPollEnabled = false;
#if defined(__unix__) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
        char*  cswitchPollBuffer = 0;
        pollfd cswitchPollFd;
#endif
#if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
        std::thread* cswitchTraceLoggerThread = 0;
        TRACEHANDLE  cswitchSessionHandle;
        TRACEHANDLE  cswitchConsumerHandle;
        EVENT_TRACE_PROPERTIES* cswitchProperties = 0;
        uint64_t qpcRef;
        uint64_t rdtscRef;
        double   qpcToRdtsc;
#endif

#endif // if PL_NOCONTROL==0 || PL_NOEVENT==0

#if PL_NOEVENT==0 && PL_VIRTUAL_THREADS==1
        FlatHashTable<uint32_t> vThreadLkupExtToCtx;
#endif

        // Signals and HW exceptions
        bool              signalHandlersSaved   = false;
        plSignalHandler_t signalsOldHandlers[7] = { 0 };
#if defined(_WIN32)
        PVOID      exceptionHandler = 0;
#if PL_IMPL_STACKTRACE==1
        // Stacktrace
        rtlWalkFrameChain_t rtlWalkFrameChain = 0;
#endif
#endif // if defined(_WIN32)
    } implCtx;


    //-----------------------------------------------------------------------------
    // [PRIVATE IMPLEMENTATION] Misc. functions
    //-----------------------------------------------------------------------------

#if PL_NOEVENT==0 && PL_EXTERNAL_STRINGS==0

    template<typename... Args>
    void
    eventLogConsoleDisplay(plLogLevel level, const char* category_, const char* format, ...)
    {
#if PL_IMPL_CONSOLE_COLOR==1
        constexpr const char* levelStr[4] = { "[\033[37mdebug\033[m]", "[\033[32minfo \033[m]", "[\033[33m\033[1mwarn \033[m]", "[\033[1m\033[41merror\033[m]" };
#define CAT_COLOR       "\033[37m"
#define CAT_COLOR_RESET "\033[m"
#else
        constexpr const char* levelStr[4] = { "[debug]", "[info ]", "[warn ]", "[error]" };
#define CAT_COLOR       ""
#define CAT_COLOR_RESET ""
#endif

        uint64_t relativeDate = PL_GET_SYSTEM_CLOCK_NS()-globalCtx.originNs;
        int hours     = (int)(relativeDate/3600000000000ULL);
        relativeDate -= 3600000000000ULL*(uint64_t)hours;
        int minutes   = (int)(relativeDate/60000000000ULL);
        relativeDate -= 60000000000ULL*(uint64_t)minutes;
        int seconds   = (int)(relativeDate/1000000000ULL);
        relativeDate -= 1000000000ULL*(uint64_t)seconds;
        {
            std::lock_guard<std::mutex> lk(implCtx.logDisplayMx);
            printf("[%02d:%02d:%02d.%09d] %s [" CAT_COLOR "%s" CAT_COLOR_RESET "] ", hours, minutes, seconds, (int)relativeDate, levelStr[level], category_);

            va_list ap;
            va_start(ap, format);
            (void)vprintf(format, ap);
            va_end (ap);
            printf("\n");
        }
    }

#endif // if PL_NOEVENT==0 && PL_EXTERNAL_STRINGS==0


    //-----------------------------------------------------------------------------
    // [PRIVATE IMPLEMENTATION] Crash and stacktrace handlers
    //-----------------------------------------------------------------------------

#if defined(__unix__) && PL_IMPL_STACKTRACE==1
    void
    crashLogStackTrace(void)
    {
        plScope("CRASH Stacktrace");
        char msgStr[PL_DYN_STRING_MAX_SIZE];

        // Initialize libunwind
        unw_context_t uc;
        unw_getcontext(&uc);
        unw_cursor_t cursor;
        unw_init_local(&cursor, &uc);
        char localMsgStr[512];
        unw_word_t offset;
        unw_word_t ip;

        // Initialize DWARF reading
        char* debugInfoPath = NULL;
        Dwfl_Callbacks callbacks = { };
        callbacks.find_elf       = dwfl_linux_proc_find_elf;
        callbacks.find_debuginfo = dwfl_standard_find_debuginfo;
        callbacks.debuginfo_path = &debugInfoPath;
        Dwfl* dwfl = dwfl_begin(&callbacks);
        if(!dwfl || dwfl_linux_proc_report(dwfl, getpid())!=0 || dwfl_report_end(dwfl, NULL, NULL)!=0) {
            return;
        }

        const int skipDepthQty = 2; // No need to display the bottom machinery
        int depth = 0;
        // Loop on stack depth
        while(unw_step(&cursor)>0) {
            unw_get_reg(&cursor, UNW_REG_IP, &ip);

            if(depth>=skipDepthQty) {
                Dwarf_Addr   addr   = (uintptr_t)(ip-4);
                Dwfl_Module* module = dwfl_addrmodule(dwfl, addr);
                Dwfl_Line*   line   = dwfl_getsrc(dwfl, addr);

                if(line) {
                    Dwarf_Addr addr2; int lineNbr; int status;
                    const char* filename = dwfl_lineinfo(line, &addr2, &lineNbr, NULL, NULL, NULL);
                    char*  demangledName = abi::__cxa_demangle(dwfl_module_addrname(module, addr), 0, 0, &status);
                    // Filename and line first in the potentially truncated remote log (demangled function name may be long)
                    snprintf(msgStr, PL_DYN_STRING_MAX_SIZE, "   #%-2d %s(%d) : %s", depth-skipDepthQty,
                             filename? strrchr(filename,'/')+1:"<unknown>", filename?lineNbr:0,
                             status?dwfl_module_addrname(module, addr):demangledName);
                    snprintf(localMsgStr, sizeof(localMsgStr),
#if PL_IMPL_CONSOLE_COLOR==1
                             "  \033[93m#%-2d \033[0m%s(%d) : \033[36m%s\033[0m\n",
#else
                             "  #%-2d %s(%d) : %s\n",
#endif
                             depth-skipDepthQty, filename? strrchr(filename,'/')+1:"<unknown>", filename? lineNbr:0,
                             status? dwfl_module_addrname(module, addr):demangledName);
                    if(status==0) free(demangledName);
                }
                else {
                    snprintf(msgStr, PL_DYN_STRING_MAX_SIZE, "   #%-2d 0x%" PRIX64 " : %s", depth-skipDepthQty, ip-4, dwfl_module_addrname(module, addr));
                    snprintf(localMsgStr, sizeof(localMsgStr),
#if PL_IMPL_CONSOLE_COLOR==1
                             "  \033[93m#%-2d \033[0m0x%" PRIX64 " : \033[36m%s\033[0m\n",
#else
                             "  #%-2d 0x%" PRIX64 " : %s\n",
#endif
                             depth-skipDepthQty, ip-4, dwfl_module_addrname(module, addr));
                }

                // Log
                plData("CRASH", msgStr);
                PL_IMPL_PRINT_STDERR(localMsgStr, true, false);
            }

            // Next unwinding
            localMsgStr[0] = 0;
            unw_get_proc_name(&cursor, localMsgStr, sizeof(localMsgStr), &offset); // Fails if there is no debug symbols
            if(!strcmp(localMsgStr,"main")) break;
            ++depth;
        } // End of unwinding

        // End session
        PL_IMPL_PRINT_STDERR("\n", true, false);
        dwfl_end(dwfl);
    }
#endif // if defined(__unix__) && PL_IMPL_STACKTRACE==1

#if defined(_WIN32) && PL_IMPL_STACKTRACE==1
    void
    crashLogStackTrace(void)
    {
        plScope("CRASH Stacktrace");
        char msgStr[PL_DYN_STRING_MAX_SIZE];
        char localMsgStr[512];
        char tmpStr[32];
        char depthStr[8];

        // Get the addresses of the stacktrace
        PVOID stacktrace[64]; // 64 levels of depth should be enough for everyone
        int   foundStackDepth = implCtx.rtlWalkFrameChain? implCtx.rtlWalkFrameChain(stacktrace, 64, 0) : 0;

        // Some required windows structures for the used APIs
        IMAGEHLP_LINE64 line; line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement = 0;
        constexpr int MaxNameSize = 8192;
        char symBuffer[sizeof(SYMBOL_INFO)+MaxNameSize];
        SYMBOL_INFO* symInfo = (SYMBOL_INFO*)symBuffer;
        symInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        symInfo->MaxNameLen   = MaxNameSize;
        HANDLE proc = GetCurrentProcess();

#define PL_CRASH_STACKTRACE_DUMP_INFO_(itemNbrStr, colorItem, colorFunc, colorNeutral) \
        if(isFuncValid || isLineValid) {                                \
            snprintf(tmpStr, sizeof(tmpStr), "(%u)", isLineValid? line.LineNumber : 0); \
            snprintf(msgStr, PL_DYN_STRING_MAX_SIZE, "%s %s%s : %s", itemNbrStr, \
                     isLineValid? strrchr(line.FileName, '\\')+1 : "<unknown>", isLineValid? tmpStr : "", \
                     isFuncValid? symInfo->Name : "<unknown>");         \
            snprintf(localMsgStr, sizeof(localMsgStr), "   " colorItem "%s " colorNeutral "%s%s : " colorFunc "%s" colorNeutral "\n", itemNbrStr, \
                     isLineValid? strrchr(line.FileName, '\\')+1 : "<unknown>", isLineValid? tmpStr : "", \
                     isFuncValid? symInfo->Name : "<unknown>");         \
        } else {                                                        \
            snprintf(msgStr, PL_DYN_STRING_MAX_SIZE, "%s 0x%" PRIX64, itemNbrStr, ptr); \
            snprintf(localMsgStr, sizeof(localMsgStr), "   " colorItem "%s" colorFunc " 0x%" PRIX64 colorNeutral "\n", itemNbrStr, ptr); \
        }                                                               \
        plData("CRASH", msgStr);                                        \
        PL_IMPL_PRINT_STDERR(localMsgStr, true, false)

        const int skipDepthQty = 3; // No need to display the bottom machinery
        for(int depth=skipDepthQty; depth<foundStackDepth; ++depth) {
            uint64_t ptr = ((uint64_t)stacktrace[depth]) - 1; // -1 because the captured PC is already pointing on the next code line at snapshot time

            // Get the nested inline function calls, if any
            DWORD frameIdx, curContext = 0;
            int inlineQty = SymAddrIncludeInlineTrace( proc, ptr );
            if(inlineQty>0 && SymQueryInlineTrace(proc, ptr, 0, ptr, ptr, &curContext, &frameIdx)) {
                for(int i=0; i<inlineQty; ++i) {
                    bool isFuncValid = (SymFromInlineContext(proc, ptr, curContext, 0, symInfo)!=0);
                    bool isLineValid = (SymGetLineFromInlineContext(proc, ptr, curContext, 0, &displacement, &line)!=0);
                    ++curContext;
#if PL_IMPL_CONSOLE_COLOR==1
                    PL_CRASH_STACKTRACE_DUMP_INFO_("inl", "\033[93m", "\033[36m", "\033[0m");
#else
                    PL_CRASH_STACKTRACE_DUMP_INFO_("inl", "", "", "");
#endif
                }
            }

            // Get the function call for this depth
            bool isFuncValid = (SymFromAddr         (proc, ptr, 0, symInfo)!=0);
            bool isLineValid = (SymGetLineFromAddr64(proc, ptr-1, &displacement, &line)!=0);
            snprintf(depthStr, sizeof(depthStr), "#%-2d", depth-skipDepthQty);
#if PL_IMPL_CONSOLE_COLOR==1
            PL_CRASH_STACKTRACE_DUMP_INFO_(depthStr, "\033[93m", "\033[36m", "\033[0m");
#else
            PL_CRASH_STACKTRACE_DUMP_INFO_(depthStr, "", "", "");
#endif
        } // End of loop on stack depth
    }
#endif //if defined(_WIN32) && PL_IMPL_STACKTRACE==1



#if PL_NOCONTROL==0 || PL_NOEVENT==0

    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION]  Platform abstraction layer for communication
    // =======================================================================================================

#if PL_IMPL_CUSTOM_COM_LAYER==0

    static bool
    palComSend(uint8_t* buffer, int size) {
        int   qty = 0;
        if     (implCtx.mode==PL_MODE_STORE_IN_FILE) qty = (int)fwrite((void*)buffer, 1, size, implCtx.fileHandle);
        else if(implCtx.mode==PL_MODE_CONNECTED) {
#ifdef _WIN32
            qty = (int)send(implCtx.serverSocket, (const char*)buffer, size, 0);
#else
            qty = (int)send(implCtx.serverSocket, (const char*)buffer, size, MSG_NOSIGNAL);  // MSG_NOSIGNAL to prevent sending SIGPIPE
#endif
        }
        implCtx.stats.sentBufferQty += 1;
        implCtx.stats.sentByteQty   += size;
        return (qty==size);
    }

#if PL_NOCONTROL==0
    // Returned value is the received byte quantity (no reception in file mode)
    //  Special values are: 0 = disconnection from server   -1 = timeout (empty reception)
    static int
    palComReceive(uint8_t* buffer, int maxBuffersize) {
        plAssert(implCtx.serverSocket!=PL_PRIV_SOCKET_ERROR);
        int byteQty = recv(implCtx.serverSocket, (char*)buffer, maxBuffersize, 0);
#ifdef _WIN32
        if((WSAGetLastError()==EAGAIN || WSAGetLastError()==EWOULDBLOCK) && byteQty<0) return -1; // Timeout on reception (empty)
#else
        if((errno==EAGAIN || errno==EWOULDBLOCK) && byteQty<0) return -1; // Timeout on reception (empty)
#endif
        else if(byteQty<1) return 0; // Client is disconnected
        return byteQty;
    }
#endif // PL_NOCONTROL==0

    static void
    palComInit(int serverConnectionTimeoutMsec)
    {
        // Initialize the socket connection
        if(implCtx.mode==PL_MODE_CONNECTED) {
#if defined(_WIN32) && PL_IMPL_MANAGE_WINDOWS_SOCKET==1
            // Windows special case: initialize the socket library
            WSADATA wsaData;
            int wsaInitStatus = WSAStartup(MAKEWORD(2,2), &wsaData);
            plAssert(wsaInitStatus==0, "Unable to initialize winsock", wsaInitStatus);
            implCtx.wsaInitialized = true;
#endif

            // Create socket
            implCtx.serverSocket = (socket_t)socket(AF_INET , SOCK_STREAM , 0);
            plAssert(PL_PRIV_IS_SOCKET_VALID(implCtx.serverSocket), "Unable to create the socket");

            // Set the timeout on reception
#ifdef __unix__
            const struct timeval socketTimeout = { .tv_sec=0, .tv_usec=10000 };
#endif
#ifdef _WIN32
            DWORD socketTimeout = 10*1000;
#endif
            setsockopt(implCtx.serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&socketTimeout, sizeof(socketTimeout));

            // Get the hostname
            plAssert(implCtx.serverPort>0 && implCtx.serverPort<65536, implCtx.serverPort);
            sockaddr_in m_server;
            m_server.sin_family = AF_INET;
            m_server.sin_port   = htons((uint16_t)implCtx.serverPort);
            int inetSuccess     = inet_pton(AF_INET, implCtx.serverAddr, &m_server.sin_addr);
            plAssert(inetSuccess, "Unable to parse the address", implCtx.serverAddr);

            // Wait for connection
            bool hasWarned = false;
            while(connect(implCtx.serverSocket, (struct sockaddr*)&m_server, sizeof(m_server))==PL_PRIV_SOCKET_ERROR) {
                if(serverConnectionTimeoutMsec==0) { // If the timeout expired, switch to inactive mode
#if _WIN32
                    closesocket(implCtx.serverSocket);
#else
                    close(implCtx.serverSocket);
#endif
                    implCtx.serverSocket = (socket_t)PL_PRIV_SOCKET_ERROR;
                    implCtx.mode         = PL_MODE_INACTIVE;
                    PL_IMPL_PRINT_STDERR("Socket connection to server failed, skipping Palanteer remote service.\n", false, false);
                    return;
                }
                if(!hasWarned) {
                    hasWarned = true;
                    PL_IMPL_PRINT_STDERR("Waiting for server\n", false, false);
                }
                // Next round. Negative timeout means unlimited
                if(serverConnectionTimeoutMsec>0) {
                    serverConnectionTimeoutMsec = (serverConnectionTimeoutMsec>=100)? serverConnectionTimeoutMsec-100 : 0;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        if(implCtx.mode==PL_MODE_STORE_IN_FILE) {
            implCtx.fileHandle = fopen(implCtx.filename, "wb");
            plAssert(implCtx.fileHandle, "Unable to open the event file for writing");
        }
    }

    static void
    palComUninit(void)
    {
        if(implCtx.mode==PL_MODE_CONNECTED) {
            // Send a FIN packet to the server and wait for closing to ensure that all previously sent data are properly received.
            // Without this process, the reception on server side may be truncated (because a RST packet is sent at socket closing time)
#if _WIN32
            int socketStatus = shutdown(implCtx.serverSocket, SD_SEND);
#else
            int socketStatus = shutdown(implCtx.serverSocket, SHUT_WR);
#endif
            if(socketStatus==0) {
                char tmp[512];
                for(int i=0; i<10; ++i) { // 1 iteration should be enough in practice.
                    int readByteQty = recv(implCtx.serverSocket, (char*)tmp, sizeof(tmp), 0); // The timeout is still applied
#ifdef _WIN32
                    if(!(WSAGetLastError()==EAGAIN || WSAGetLastError()==EWOULDBLOCK) || readByteQty==0) break;  // End of connection from server side?
#else
                    if(!(errno==EAGAIN || errno==EWOULDBLOCK) || readByteQty==0) break;  // End of connection from server side?
#endif
                }
            }

            // Close sockets
#if defined(_WIN32)
            // Windows case
            closesocket(implCtx.serverSocket);
#if PL_IMPL_MANAGE_WINDOWS_SOCKET==1
            if(implCtx.wsaInitialized) { // Windows special case: cleanup the socket library
                WSACleanup();
                implCtx.wsaInitialized = false;
            }
#endif
#else
            // Linux case
            close(implCtx.serverSocket);
#endif
            implCtx.serverSocket = (socket_t)PL_PRIV_SOCKET_ERROR;
        }

        if(implCtx.mode==PL_MODE_STORE_IN_FILE) {
            fclose(implCtx.fileHandle);
            implCtx.fileHandle = 0;
        }
    }

#endif // if PL_IMPL_CUSTOM_COM_LAYER==0



    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] Reception management
    // =======================================================================================================

#if PL_NOCONTROL==0

    void
    registerCli(plCliHandler_t handler, const char* name, const char* specParams, const char* description,
                hashStr_t nameHash, hashStr_t specParamsHash, hashStr_t descriptionHash)
    {
        implCtx.cliManager.registerCli(handler, name, specParams, description,
                                       nameHash, specParamsHash, descriptionHash);
    }

    static uint8_t*
    helperFillResponseBufferHeader(RemoteCommandType commandType, int commandByteSize, uint8_t* br)
    {
        br[0] = 'P';
        br[1] = 'L';
        br[2] = ((int)PL_DATA_TYPE_CONTROL>>8)&0xFF;
        br[3] = ((int)PL_DATA_TYPE_CONTROL>>0)&0xFF;
        commandByteSize += 2;               // Size of the command type
        br[4] = (commandByteSize>>24)&0xFF; // Command byte quantity (after the 8 bytes remote data type header)
        br[5] = (commandByteSize>>16)&0xFF;
        br[6] = (commandByteSize>> 8)&0xFF;
        br[7] = (commandByteSize>> 0)&0xFF;
        br[8] = ((int)commandType>>8)&0xFF;
        br[9] = ((int)commandType>>0)&0xFF;
        return br;
    }

    static void
    helperFinishResponseBuffer(int txBufferSize)
    {
        // Mark it for sending (in the tx thread)
        std::lock_guard<std::mutex> lk(implCtx.txThreadSyncMx);
        implCtx.rspBufferSize.store(txBufferSize);
        // Will be sent at the next tx thread loop, that we force now
        implCtx.txThreadSyncCv.notify_one();
    }

    void
    receiveFromServer(void)
    {
        auto& ic = implCtx;
        plgDeclareThread(PL_VERBOSE, "Palanteer/Reception");

        while(!ic.threadServerFlagStop.load()) {

            int recByteQty = palComReceive(ic.reqBuffer, PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY);
            if     (recByteQty <0) continue; // Timeout on reception (empty)
            else if(recByteQty==0) break;    // Client is disconnected

            // Parse the received content, expected block structure is:
            //   [block]            <2B: synchro magic>: 'P' 'L'
            //   [block]            <2B: bloc type>
            //   [remote data type] <4B: command byte qty>
            //   [remote data type] <2B: remote command type>
            //   (following payloads depends on the command type)
            // Sanity
            uint8_t* b = (uint8_t*)ic.reqBuffer;
            plAssert(recByteQty>=10);
            plAssert(b[0]=='P' && b[1]=='L', "Magic not present: connection is desynchronized");
            DataType dt = (DataType)((b[2]<<8) | b[3]);
            plAssert(dt==PL_DATA_TYPE_CONTROL, "Wrong block data type received through socket despite synchronization: connection is buggy.");
            int commandByteQty = (b[4]<<24) | (b[5]<<16) | (b[6]<<8) | b[7];
            plAssert(8+commandByteQty<=PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY, "Too big remote command received. Limit is:",
                     PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY, 8+commandByteQty);

            // Wait the end of the request reception
            while(recByteQty<8+commandByteQty && !ic.threadServerFlagStop.load()) {
                int nextByteQty = palComReceive(ic.reqBuffer+recByteQty, PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY-recByteQty);
                if     (nextByteQty <0) continue; // Timeout on reception (empty)
                else if(nextByteQty==0) break;    // Client is disconnected
                recByteQty += nextByteQty;
            }
            if(recByteQty<8+commandByteQty || ic.threadServerFlagStop.load()) continue; // Exit or connection break case
            if(ic.rspBufferSize.load()>0) continue; // The response buffer shall be free, if the sender behaves as expected

            // Process the command
            RemoteCommandType ct = (RemoteCommandType)((b[8]<<8) | b[9]);
            int payloadByteQty = commandByteQty-2; // 2 = command type

            if(ct==PL_CMD_SET_FREEZE_MODE) {
                { // Scope so that the bootstrap of the thread is not included
                    plgScope(PL_VERBOSE, "Request: set freeze mode");
                    plAssert(payloadByteQty==1);

                    // Update the state
                    bool isFreezeEnabled = (b[10]!=0);
                    plgData(PL_VERBOSE, "State", isFreezeEnabled);
                    {
                        std::unique_lock<std::mutex> lk(ic.frozenThreadMx);
                        ic.frozenThreadEnabled.store(isFreezeEnabled? 1:0);
                    }

                    // Free the threads if disabled
                    if(!isFreezeEnabled) {
                        uint64_t bitmap = ic.frozenThreadBitmap.load();
                        int tId = 0;
                        while(bitmap) {
                            if(bitmap&1) ic.frozenThreadCv[tId].notify_one();
                            bitmap >>= 1; ++tId;
                        }
                    }

                    // Build and send the response
                    uint8_t* br = helperFillResponseBufferHeader(PL_CMD_SET_FREEZE_MODE, 2, ic.rspBuffer);
                    br[10] = (((int)PL_OK)>>8)&0xFF;
                    br[11] = (((int)PL_OK)>>0)&0xFF;
                    helperFinishResponseBuffer(12);
                }

                // Notify the initialization thread that server and reception thread is ready
                // This let also the chance to safely activate the freeze mode before starting the program
                if(!ic.rxIsStarted) {
                    std::lock_guard<std::mutex> lk(ic.threadInitMx);
                    ic.rxIsStarted = true;
                    ic.threadInitCvTx.notify_one();
                }
            }

            else if(ct==PL_CMD_STEP_CONTINUE) {
                plgScope(PL_VERBOSE, "Request: resume thread execution");
                plAssert(payloadByteQty==8);
                // Unmask the selected threads
                uint64_t bitmap = ((uint64_t)b[10]<<56) | ((uint64_t)b[11]<<48) | ((uint64_t)b[12]<<40) | ((uint64_t)b[13]<<32) |
                    ((uint64_t)b[14]<<24) | ((uint64_t)b[15]<<16) | ((uint64_t)b[16]<<8) | ((uint64_t)b[17]<<0);
                plgData(PL_VERBOSE, "Thread bitmap##hexa", bitmap);
                {
                    std::unique_lock<std::mutex> lk(ic.frozenThreadMx);
                    ic.frozenThreadBitmap.fetch_and(~bitmap);
                }
                // Wake up the selected threads
                int tId = 0;
                while(bitmap) {
                    if(bitmap&1) ic.frozenThreadCv[tId].notify_one();
                    bitmap >>= 1; ++tId;
                }
                // Build and send the response
                uint8_t* br = helperFillResponseBufferHeader(PL_CMD_STEP_CONTINUE, 2, ic.rspBuffer);
                br[10] = (((int)PL_OK)>>8)&0xFF;
                br[11] = (((int)PL_OK)>>0)&0xFF;
                helperFinishResponseBuffer(12);
            }

            else if(ct==PL_CMD_SET_MAX_LATENCY) {
                plgScope(PL_VERBOSE, "Request: set max latency");
                plAssert(payloadByteQty==2);
                // Update the internal state
                int maxLatencyMs = (int)(b[10]<<8) | (int)b[11];
                plgData(PL_VERBOSE, "Max latency##ms", maxLatencyMs);
                ic.maxSendingLatencyNs = 1e6*maxLatencyMs;

                // Build and send the response
                uint8_t* br = helperFillResponseBufferHeader(PL_CMD_SET_MAX_LATENCY, 2, ic.rspBuffer);
                br[10] = (((int)PL_OK)>>8)&0xFF;
                br[11] = (((int)PL_OK)>>0)&0xFF;
                helperFinishResponseBuffer(12);
            }

            else if(ct==PL_CMD_KILL_PROGRAM) {
                plgScope(PL_VERBOSE, "Request: kill program");
                // We use quick_exit (introduced in C++11) instead of exit (or abort) because:
                //  - 'quick_exit' is design for multi-threaded application which are hard or costly in plumbing to stop in a clean manner
                //  - 'quick_exit' does not "clean" the process before leaving but allows manual cleaning through 'at_quick_exit' registration
                //  - 'exit' has good odds to block or crash if the program is not specifically designed for it
                //  - 'abort' is "violent", the signal does not allow any cleaning and may lead to a core dump or a popup window (on windows)
                std::quick_exit(0);
                // No need to bother for any response, as connection will be down very soon...
            }

            else if(ct==PL_CMD_CALL_CLI) {
                plgScope(PL_VERBOSE, "Request: CLI");
                // Sanity
                plAssert(payloadByteQty>2);
                b[8+commandByteQty-1] = 0; // Force the zero terminated string at the end of the reception buffer, just in case

                // Get the CLI request qty and prepare the response buffer
                int cliRequestQty = (b[10]<<8) | b[11];
                int reqOffset     = 12; // Current position in request buffer
                int rspOffset     = 12; // Command response header, partially filled at the end
                uint8_t* br       = helperFillResponseBufferHeader(PL_CMD_CALL_CLI, 0, ic.rspBuffer); // 0 bytes size is a dummy value, it will be overwritten later
                // br[ 4-> 7] = 4 bytes for the command response byte qty (filled later)
                // br[10->11] = 2 bytes for the CLI response qty (filled later)
                plgData(PL_VERBOSE, "CLI request quantity", cliRequestQty);

                // Loop on requests and fill the response buffer
                constexpr int bufferFullMessageLength = 28; //strlen("CLI response buffer is full")+1; // strlen is not constexpr in MSVC2019
                int cliRequestNbr = 0;
                while(cliRequestNbr<cliRequestQty) {
                    // Call
                    plRemoteStatus cliStatus;
                    plgScope(PL_VERBOSE, "Call");
                    const char* cliResponse = ic.cliManager.execute((char*)&b[reqOffset], cliStatus);
                    int responseLength = (int)strlen(cliResponse)+1;
                    plgVar(PL_VERBOSE, cliRequestNbr, cliStatus, responseLength);

                    // Check if we can store the response in the buffer
                    if(rspOffset+2+responseLength>PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY -
                       ((cliRequestNbr==cliRequestQty-1)? 0 : 2+bufferFullMessageLength)) { // minimum size to store a truncated response, if not last command
                        // Not enough space in response buffer
                        plAssert(rspOffset+2+bufferFullMessageLength<=PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY); // It should by design
                        plgLogError(PL_VERBOSE, "error", "Not enough space in the response buffer");
                        br[rspOffset+0] = (((int)PL_ERROR)>>8)&0xFF;
                        br[rspOffset+1] = (((int)PL_ERROR)>>0)&0xFF;
                        snprintf((char*)&br[rspOffset+2], bufferFullMessageLength+1, "CLI response buffer is full");
                        rspOffset += 2+bufferFullMessageLength;
                        while(b[reqOffset]!=0) ++reqOffset;
                        ++reqOffset; // Skip the zero termination of the string request
                        ++cliRequestNbr;
                        break;
                    }

                    // Store the answer
                    br[rspOffset+0] = (((int)cliStatus)>>8)&0xFF;
                    br[rspOffset+1] = (((int)cliStatus)>>0)&0xFF;
                    memcpy(br+rspOffset+2, cliResponse, responseLength); // Copy the response string with the zero termination
                    rspOffset += 2+responseLength;

                    // Next request
                    while(b[reqOffset]!=0) ++reqOffset;
                    ++reqOffset; // Skip the zero termination of the string request
                    ++cliRequestNbr;

                    // if(cliStatus!=PL_OK) break;  // Another possible behavior is stop at first failure (not by default)
                } // End of loop on the CLI requests
                // Either not all requests are processed, either we read all the request bytes
                plAssert(cliRequestNbr<cliRequestQty || reqOffset==8+commandByteQty,
                         cliRequestNbr, cliRequestQty, reqOffset, 8+commandByteQty);

                // Finalize the response and send it
                br[4] = ((rspOffset-8)>>24)&0xFF; // Command byte quantity (after the 8 bytes remote data type header)
                br[5] = ((rspOffset-8)>>16)&0xFF;
                br[6] = ((rspOffset-8)>> 8)&0xFF;
                br[7] = ((rspOffset-8)>> 0)&0xFF;
                br[10] = (cliRequestNbr>>8)&0xFF; // CLI answer quantity (less than or equal to the request quantity)
                br[11] = (cliRequestNbr>>0)&0xFF;
                helperFinishResponseBuffer(rspOffset);
            } // if(ct==PL_CMD_CALL_CLI)

        } // End of reception loop

        // In case of server connection failure, the program shall be started anyway
        if(!ic.rxIsStarted) {
            std::lock_guard<std::mutex> lk(ic.threadInitMx);
            ic.rxIsStarted = true;
            ic.threadInitCvTx.notify_one();
        }

        plgLogInfo(PL_VERBOSE, "threading", "End of Palanteer reception loop");
    }

#endif // if PL_NOCONTROL==0



    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] Collection and transmission task
    // =======================================================================================================

    // Trick to be able to push the few events from this function even in case of saturated buffer
    //   It is safe because buffer have dimensioned margins for it (only for this specific internal case)
#undef  PL_STORE_COLLECT_CASE_
#define PL_STORE_COLLECT_CASE_ 1

    static void
    sendStrings(int stringQty)
    {
        // Strings
        auto& ic = implCtx;
        auto& sBuf = ic.strBuffer; // Content filled before this call, with pre-allocated 8-bytes header
        // Initialize the pre-allocated header
        sBuf[0] = 'P'; // For desynchronization/problem detection
        sBuf[1] = 'L';
        sBuf[2] = ((int)PL_DATA_TYPE_STRING>>8)&0xFF; // Data block type
        sBuf[3] = ((int)PL_DATA_TYPE_STRING>>0)&0xFF;
        sBuf[4] = (stringQty>>24)&0xFF; // String qty
        sBuf[5] = (stringQty>>16)&0xFF;
        sBuf[6] = (stringQty>> 8)&0xFF;
        sBuf[7] = (stringQty>> 0)&0xFF;
        palComSend(&sBuf[0], sBuf.size());
        ic.stats.sentStringQty += stringQty;
        plgData(PL_VERBOSE, "sent strings", stringQty);
    }

#define PL_PRIV_PROCESS_STRING(h,s,d)                                   \
    {                                                                   \
        uint32_t stringIndex;                                           \
        if(!ic.lkupStringToIndex.find(h, stringIndex)) {                \
            int l = 1 + ((s)?(int)strlen(s):0);                         \
            if(8+l>sBuf.free_space()) {                                 \
                sendStrings(stringQty);                                 \
                stringQty = 0;                                          \
                sBuf.resize(8); /* Space for header */                  \
                plAssert(8+l<sBuf.free_space(), "PL_IMPL_STRING_BUFFER_BYTE_QTY is too small to contain the string", s, PL_IMPL_STRING_BUFFER_BYTE_QTY); \
            }                                                           \
            int sOffset = sBuf.size();                                  \
            sBuf.resize(sBuf.size()+8+l);                               \
            sBuf[sOffset+0] = (((uint64_t)(h))>>56)&0xFF;               \
            sBuf[sOffset+1] = (((uint64_t)(h))>>48)&0xFF;               \
            sBuf[sOffset+2] = (((uint64_t)(h))>>40)&0xFF;               \
            sBuf[sOffset+3] = (((uint64_t)(h))>>32)&0xFF;               \
            sBuf[sOffset+4] = (((uint64_t)(h))>>24)&0xFF;               \
            sBuf[sOffset+5] = (((uint64_t)(h))>>16)&0xFF;               \
            sBuf[sOffset+6] = (((uint64_t)(h))>> 8)&0xFF;               \
            sBuf[sOffset+7] = (((uint64_t)(h))>> 0)&0xFF;               \
            if(s) memcpy(&sBuf[sOffset+8], s, l);                       \
            else  sBuf[sOffset+8] = 0;                                  \
            ic.lkupStringToIndex.insert(h, ic.stringUniqueId);          \
            (d) = ic.stringUniqueId;                                    \
            ++ic.stringUniqueId;                                        \
            ++stringQty;                                                \
        }                                                               \
        else { (d) = stringIndex; }                                     \
    }

#if PL_NOCONTROL==0

    static void
    collectResponse(void)
    {
        auto& ic = implCtx;

        // Send a response, if any is pending
        int rspBufferSize = ic.rspBufferSize.load();
        if(rspBufferSize>0) {
            // Send
            plgBegin (PL_VERBOSE, "Response: sending buffer");
            plgData(PL_VERBOSE, "size", rspBufferSize);
            memcpy(ic.sendBuffer, ic.rspBuffer, rspBufferSize); // Copy into the "send" buffer to avoid the race condition (at "send" moment) with the command reception
            ic.rspBufferSize.store(0);
            palComSend(ic.sendBuffer, rspBufferSize);
            plgEnd(PL_VERBOSE, "Response: sending buffer");
        }

        // Check if frozen threads bitmap changed
        // This is done before newly registered CLI sending so that frozen thread synchronization covers their state.
        // Freeze synchronization shall be fully reliable, it is a critical feature for scripting.
        uint64_t bitmapChange=0, bitmapLast=0;
        if(ic.frozenThreadBitmapChange.load()) {
            // @#TODO Shall be atomic
            bitmapChange = ic.frozenThreadBitmapChange.load();
            ic.frozenThreadBitmapChange.store(0);
            bitmapLast   = ic.frozenLastThreadBitmap;
            ic.frozenLastThreadBitmap = ic.frozenThreadBitmap.load();
        }

        // Check if new CLIs have been registered
        int registeredCliQty = ic.cliManager.getCliQty();
        if(ic.lastSentCliQty<registeredCliQty) {
            plgBegin (PL_VERBOSE, "Notification: sending new declared CLIs");
            auto& sBuf = ic.strBuffer;
            sBuf.resize(8); // Base header (2B synchro + 2B data type) + 4B string qty
            int cliQty = registeredCliQty-ic.lastSentCliQty;
            plAssert(8+2+2*3*cliQty<PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY,
                     "The CLI qty exceeds the capacity of the response buffer to declare them on server side",
                     PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY, 10/*header*/ + 6/*bytes per CLI*/*cliQty);
            uint8_t* br = helperFillResponseBufferHeader(PL_NTF_DECLARE_CLI, 2+2*3*cliQty, ic.sendBuffer);
            br[10] = (cliQty>>8)&0xFF;
            br[11] = (cliQty>>0)&0xFF;

            // Parse the new non-sent CLIs
            int offset = 12, stringQty = 0;
            for(int i=ic.lastSentCliQty; i<registeredCliQty; ++i) {
                const CliManager::CliStrings& cs = ic.cliManager.getCliStrings(i);
                hashStr_t strHash;
                int       strIdx;
                // CLI name
                for(int j=0; j<3; ++j) {
                    const char* str = (j==0)? cs.name : ((j==1)? cs.specParams : cs.description);
                    strHash = (j==0)? cs.nameHash : ((j==1)? cs.specParamsHash : cs.descriptionHash);
                    PL_PRIV_PROCESS_STRING(strHash, str, strIdx);
                    br[offset++] = (strIdx>>8)&0xFF;
                    br[offset++] = (strIdx>>0)&0xFF;
                }
            }

            // Send infos
            if(stringQty) sendStrings(stringQty);
            palComSend(br, offset);
            plgData(PL_VERBOSE, "cli qty", registeredCliQty-ic.lastSentCliQty);
            ic.lastSentCliQty = registeredCliQty;
            plgEnd(PL_VERBOSE, "Notification: sending new declared CLIs");
        }

        // Send frozen threads bitmap changes
        if(bitmapChange) {
            // Build the notification from the changes
            uint64_t newBitmap = bitmapLast ^ bitmapChange;
            uint8_t* br = helperFillResponseBufferHeader(PL_NTF_FROZEN_THREAD, 8, ic.sendBuffer);
            br[10] = (newBitmap>>56)&0xFF;
            br[11] = (newBitmap>>48)&0xFF;
            br[12] = (newBitmap>>40)&0xFF;
            br[13] = (newBitmap>>32)&0xFF;
            br[14] = (newBitmap>>24)&0xFF;
            br[15] = (newBitmap>>16)&0xFF;
            br[16] = (newBitmap>> 8)&0xFF;
            br[17] = (newBitmap>> 0)&0xFF;
            plgBegin(PL_VERBOSE, "Notification: sending new frozen thread bitmap from change");
            plgVar(PL_VERBOSE, newBitmap);
            palComSend(br, 18);
            plgEnd(PL_VERBOSE, "Notification: sending new frozen thread bitmap from change");

            // Send the notification from the last bitmap, if different from the "change" version
            // This 2-step scheme is solving the ABA problem on server side (ABABA is equivalent to ABA)
            if(newBitmap!=ic.frozenLastThreadBitmap) {
                br = helperFillResponseBufferHeader(PL_NTF_FROZEN_THREAD, 8, ic.sendBuffer);
                br[10] = (ic.frozenLastThreadBitmap>>56)&0xFF;
                br[11] = (ic.frozenLastThreadBitmap>>48)&0xFF;
                br[12] = (ic.frozenLastThreadBitmap>>40)&0xFF;
                br[13] = (ic.frozenLastThreadBitmap>>32)&0xFF;
                br[14] = (ic.frozenLastThreadBitmap>>24)&0xFF;
                br[15] = (ic.frozenLastThreadBitmap>>16)&0xFF;
                br[16] = (ic.frozenLastThreadBitmap>> 8)&0xFF;
                br[17] = (ic.frozenLastThreadBitmap>> 0)&0xFF;
                plgBegin(PL_VERBOSE, "Notification: sending new frozen thread bitmap");
                plgVar(PL_VERBOSE, ic.frozenLastThreadBitmap);
                palComSend(br, 18);
                plgEnd(PL_VERBOSE, "Notification: sending new frozen thread bitmap");
            }
        }
    }
#endif //if PL_NOCONTROL==0


#if PL_NOEVENT==0

    static void
    sendEvents(int eventQty, uint8_t* eventBuffer, DataType dataType, uint64_t preDateTick)
    {
        // Initialize the pre-allocated header
        eventBuffer[0] = 'P'; // For desynchronization/problem detection
        eventBuffer[1] = 'L';
        eventBuffer[2] = ((int)dataType>>8)&0xFF; // Auxiliary data block type
        eventBuffer[3] = ((int)dataType>>0)&0xFF;
        eventBuffer[4] = (eventQty>>24)&0xFF; // Event qty
        eventBuffer[5] = (eventQty>>16)&0xFF;
        eventBuffer[6] = (eventQty>> 8)&0xFF;
        eventBuffer[7] = (eventQty>> 0)&0xFF;

        // Timestamp
        eventBuffer[ 8] = (preDateTick>>56)&0xFF;
        eventBuffer[ 9] = (preDateTick>>48)&0xFF;
        eventBuffer[10] = (preDateTick>>40)&0xFF;
        eventBuffer[11] = (preDateTick>>32)&0xFF;
        eventBuffer[12] = (preDateTick>>24)&0xFF;
        eventBuffer[13] = (preDateTick>>16)&0xFF;
        eventBuffer[14] = (preDateTick>> 8)&0xFF;
        eventBuffer[15] = (preDateTick>> 0)&0xFF;

        palComSend(eventBuffer, 16+eventQty*sizeof(EventExt));
        implCtx.stats.sentEventQty += eventQty;
        if(eventQty) plgData(PL_VERBOSE, "sent events",  eventQty);
    }


    static inline void
    updateDateWrap(clockType_t dateTick)
    {
        // Useful only with short date. With long dates, wrap is always zero and server ignores the info anyway
        if(dateTick<implCtx.lastWrapCheckDateTick) ++implCtx.lastDateWrapQty;
        implCtx.lastWrapCheckDateTick = dateTick;
    }


    static bool
    collectEvents(bool doForce)
    {
        // Scope does not work in our special case with disabled event buffer bound checks
        plgBegin(PL_VERBOSE, "collectEvents");
        if(globalCtx.dynStringPool.getUsed()) plgData(PL_VERBOSE, "dyn strings in use", globalCtx.dynStringPool.getUsed());

        auto& ic = implCtx;
        clockType_t dateTick =  PL_GET_CLOCK_TICK_FUNC();
        updateDateWrap(dateTick);

        // Rate limit the sending calls (only if the induced latency is tolerated and
        //  1/8 filling of the current buffer is not reached and less than 1/8 of the dynamic
        //  string pool is used)
        if(!doForce &&
           ic.tickToNs*(clockType_t)(dateTick-ic.lastSentEventBufferTick)<ic.maxSendingLatencyNs &&
           (int)(globalCtx.bankAndIndex.load()&EVTBUFFER_MASK_INDEX)<globalCtx.collectBufferMaxEventQty/8 &&
           globalCtx.dynStringPool.getUsed()<globalCtx.dynStringPool.getSize()/8) {
            plgEnd(PL_VERBOSE, "collectEvents");
            return false; // No need to recollect another time with short loop
        }
        ic.lastSentEventBufferTick  = dateTick;

        // Get the buffers to process
        // The destination buffer is the same as the source buffer, shifted by 1 input event. This ok because the collection buffers
        //   are shifted by 1 event vs the allocation, in order to allow this. This is memory efficient, cache friendly and safe as
        //   the output event is smaller than the input one
        uint32_t eventQty         = implCtx.prevBankAndIndex&EVTBUFFER_MASK_INDEX;
        uint8_t* srcBuffer        = (uint8_t*)(globalCtx.collectBuffers[(implCtx.prevBankAndIndex>>31)&1]);
        uint8_t* dstBuffer        = implCtx.sendBuffer;
        uint32_t srcByteToCopyQty = eventQty*sizeof(EventInt);
        uint32_t stringQty        = 0;
        if(srcByteToCopyQty>ic.stats.collectBufferMaxUsageByteQty) {
            ic.stats.collectBufferMaxUsageByteQty = srcByteToCopyQty;
        }
        if(globalCtx.dynStringPool.getUsed()>=(int)ic.stats.collectDynStringMaxUsageQty) {
            ic.stats.collectDynStringMaxUsageQty = globalCtx.dynStringPool.getUsed();
        }

        // Collect the new strings
        if(eventQty) plgBegin(PL_VERBOSE, "parsing");
        auto& sBuf = ic.strBuffer;
        sBuf.resize(8); // Base header (2B synchro + 2B data type) + 4B string qty

        for(uint32_t evtIdx=0; evtIdx<eventQty; ++evtIdx) {
            // We convert in-place the EventInt into EventExt, which is cache friendly.
            // There is no overlap thanks to the src/dst buffer start shift and smaller size of EventExt
            EventInt& src = ((EventInt*)srcBuffer)[evtIdx];
            EventExt& dst = ((EventExt*)(dstBuffer+16))[evtIdx]; // 16B header offset, the header will be filled before sending

            // Check the write acknowledgement byte, to ensure that the event is fully written
            if(src.writeAck==0) {
                volatile const EventInt& waitSrc = ((EventInt*)srcBuffer)[evtIdx];
                while(waitSrc.writeAck==0) std::this_thread::yield();
            }
            src.writeAck = 0; // Clean the write acknowledgement, for the next cycle

            // Copy the remaining values
            dst.threadId = src.threadId;
            dst.flags    = src.flags;
            dst.lineNbr  = src.lineNbr;

            // Log params case (special because the layout is an exception to the structure)
            if(src.flags==PL_FLAG_TYPE_LOG_PARAM) {
                // Raw copy of the payload, starting from the 4th byte. It contains up to 4 packed values
                memcpy(((uint8_t*)&dst)+4, ((uint8_t*)&src)+4, sizeof(EventExt)-4);
                // Update the string data: replace the pointer with the index
                int dataOffset = 0;
                for(int paramTypeShift=0; paramTypeShift<=12; paramTypeShift+=3) {
                    int paramType = (dst.lineNbr>>paramTypeShift)&0x7;
                    if(paramType==PL_FLAG_TYPE_DATA_NONE) break;
                    if(paramType==PL_FLAG_TYPE_DATA_STRING) {
                        uint8_t* payload = ((uint8_t*)&dst)+4+dataOffset;
                        const char* name = *(const char**)payload;
                        hashStr_t strNameHash  = hashString(name); // Runtime hash (as the string is dynamic, no choice)
                        PL_PRIV_PROCESS_STRING(strNameHash, name, *(uint32_t*)payload);
                        globalCtx.dynStringPool.release((DynString_t*)name);
                    }
                    dataOffset += (paramType>=PL_FLAG_TYPE_DATA_S64)? 8 : 4;
                }
                continue;
            }
            // Memory case (special because many infos to fit)
            if(src.flags==PL_FLAG_TYPE_ALLOC_PART || src.flags==PL_FLAG_TYPE_DEALLOC_PART) {
                dst.memSize = src.extra;
            }
            // Context switch case (windows specific path)
            else if(src.flags==PL_FLAG_TYPE_CSWITCH) {
                // Idle            : threadId =NONE and sysThreadId=0
                // External process: threadId =NONE and sysThreadId=N strictly positif
                // Internal process: threadId!=NONE and sysThreadID=N/A
                dst.prevCoreId  = (src.lineNbr>>8)&0xFF; // Stored in the line field...
                dst.newCoreId   = (src.lineNbr   )&0xFF;
                if     (src.threadId!=PL_CSWITCH_CORE_NONE) dst.nameIdx = (nameData_t)0xFFFFFFFF; // Internal thread
                else if(src.extra==0)                       dst.nameIdx = (nameData_t)0xFFFFFFFE; // Idle
                else {                                                                // External thread
                    // @#TODO Retrieve the name of the associated process
                    hashStr_t strNameHash = hashString("External"); // Runtime hash (as the string is dynamic, no choice)
                    PL_PRIV_PROCESS_STRING(strNameHash, "External", dst.nameIdx);
                }
            }
            // Generic info case
            else {
                { // Filename processing
                    hashStr_t strHash = src.filenameHash;
                    bool  isDynString = (strHash==0);
                    if(isDynString) strHash = hashString(src.filename); // Runtime hash (as the string is dynamic, no choice)
                    PL_PRIV_PROCESS_STRING(strHash, src.filename, dst.filenameIdx);
                    if(isDynString) globalCtx.dynStringPool.release((DynString_t*)src.filename);
                }
                { // Event name processing
                    hashStr_t strHash = src.nameHash;
                    bool  isDynString = (strHash==0);
                    if(isDynString) strHash = hashString(src.name); // Runtime hash (as the string is dynamic, no choice)
                    PL_PRIV_PROCESS_STRING(strHash, src.name, dst.nameIdx);
                    if(isDynString) globalCtx.dynStringPool.release((DynString_t*)src.name);
                }
            }

            // Copy data fields
            dst.PL_PRIV_RAW_FIELD = src.PL_PRIV_RAW_FIELD; // Enough to copy all types except strings (endianness will be handled on server side)
            if(src.flags==PL_FLAG_TYPE_DATA_STRING) {
                hashStr_t strHash = src.vString.hash;
                bool  isDynString = (strHash==0);
                if(isDynString) strHash = hashString(src.vString.value); // Runtime hash (as the string is dynamic, no choice)
                PL_PRIV_PROCESS_STRING(strHash, src.vString.value, dst.vStringIdx);
                if(isDynString) globalCtx.dynStringPool.release((DynString_t*)src.vString.value);
            }
        }

        if(eventQty) plgEnd(PL_VERBOSE, "parsing");

        // Swap the banks: Toggle the bank bit and reset the index
        std::atomic<uint32_t>& bi = globalCtx.bankAndIndex;
        uint32_t  newBankAndIndex = (bi.load()^EVTBUFFER_MASK_BANK)&EVTBUFFER_MASK_BANK;
        int bankNbr = (newBankAndIndex&EVTBUFFER_MASK_BANK)? 1 : 0;

        // Get the date before any event from this batch
        uint64_t preDateTick = implCtx.bankPreDateTick[bankNbr];
#if PL_SHORT_DATE==1
        preDateTick |= ((uint64_t)implCtx.bankPreDateWrapQty[bankNbr])<<32;
#endif

        // Store the date before the bank starts being filled
        implCtx.bankPreDateWrapQty[bankNbr] = implCtx.lastDateWrapQty;
        implCtx.bankPreDateTick   [bankNbr] = implCtx.lastWrapCheckDateTick;

        // Swap!
        implCtx.prevBankAndIndex = bi.exchange(newBankAndIndex);

        // Some saturation are detected?
        int isSaturated = globalCtx.isBufferSaturated.exchange(0);
        if(isSaturated) plLogError("SATURATION", "EVENT BUFFER IS FULL. PLEASE INCREASE ITS SIZE FOR VALID MEASUREMENTS");
        isSaturated = globalCtx.isDynStringPoolEmpty.exchange(0);
        if(isSaturated) plLogError("SATURATION", "DYNAMIC STRING POOL IS EMPTY. PLEASE INCREASE ITS SIZE FOR VALID MEASUREMENTS");

        // Write (file case) or send (socket case) the buffer
        if(eventQty || stringQty) plgBegin(PL_VERBOSE, "sending scopes");
        if(stringQty) sendStrings(stringQty);
        // Event buffer is sent even without events. No event is an information by itself ("a collection loop was done")
        sendEvents (eventQty, dstBuffer, PL_DATA_TYPE_EVENT, preDateTick);
        if(eventQty || stringQty) plgEnd(PL_VERBOSE, "sending scopes");
        plgEnd(PL_VERBOSE, "collectEvents");

        return (eventQty ||
                globalCtx.dynStringPool.getUsed()>=globalCtx.dynStringPool.getSize()/8); // Recollection is needed if some dynamic strings are used
    }
#endif // if PL_NOEVENT==0


#if defined(__unix__) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    static bool
    collectCtxSwitch(bool doForce)
    {
        plgBegin(PL_VERBOSE, "collectCtxSwitch");
        bool wasWorkedDone = false;
        auto& ic = implCtx;
        int threadQty = globalCtx.nextThreadId;
        char pidName1[32], pidName2[32];
        int maxLoopCount = doForce? 200:30; // Sanity to avoid infinite loops if forced polling

        // On Linux, context switch events are directly collected and sent to the server.
        // Indeed, as we are in the collection thread, it would be suboptimal to go through the collection buffer.
        while(maxLoopCount>0 && (doForce || !ic.threadServerFlagStop.load()) && poll(&ic.cswitchPollFd, 1, 0)>0) {
            --maxLoopCount;

            plgBegin(PL_VERBOSE, "read pipe");
            ssize_t readSize = read(ic.cswitchPollFd.fd, ic.cswitchPollBuffer, SWITCH_CTX_BUFFER_SIZE);
            plgEnd(PL_VERBOSE, "read pipe");
            if(readSize<=0) break;
            wasWorkedDone = true;
            auto& sBuf = ic.strBuffer;
            sBuf.resize(8); // Base header (2B synchro + 2B data type) + 4B string qty
            uint32_t stringQty = 0;

            plgBegin(PL_VERBOSE, "parse lines");
            EventExt* dstBuffer   = (EventExt*)(ic.cswitchPollBuffer+16); // In-place, with 16B for the header
            int       dstEventQty = 0;
            char*       line = ic.cswitchPollBuffer;
            const char* end  = ic.cswitchPollBuffer + readSize;
            *(line+readSize) = 0;

            while(1) {
                // Get the line
                char* next = line;
                while(next<end && *next!='\n') ++next;
                if(next==end) break;
                plAssert(*next == '\n'); // @#TODO Probably an assertion is too brutal in case the context switch line is partial...
                next++;

#define PL_SEARCH_TEXT(cond, offset) while(cond) { ++line; } line += offset
                // Parse the line. Typical example:
                //   sched:
                //           <...>-1193  [001]  61144.372379: sched_switch: prev_comm=xfce4-terminal prev_pid=1193 prev_prio=120 prev_state=R+ ==> next_comm=kworker/u4:1 next_pid=147703 next_prio=120
                //   softirq:
                //           <idle>-0     [000]  1541931773185958: softirq_entry: vec=7 [action=SCHED]

                line += 15;
                PL_SEARCH_TEXT(line<next && *line!='-', 1);
                uint32_t curPid = parseNumber(line); line += 1; // skip ' '
                PL_SEARCH_TEXT(line<next && *line!='[', 1);
                uint8_t coreId = (uint8_t)parseNumber(line); line += 1; // skip ']'
                PL_SEARCH_TEXT(line<next && *line==' ', 0);
                uint64_t timeValueNs = parseNumber(line); line += 2;  // skip ': '

                // Sched event?
                if(memcmp(line, "sched_switch", 12)==0) {
                    line += 14;
                    PL_SEARCH_TEXT(line<next-9 && memcmp(line, "prev_comm", 9)!=0, 10);
                    parseString(line, &pidName1[0], 32);
                    PL_SEARCH_TEXT(line<next-8 && memcmp(line, "prev_pid", 8)!=0, 9);
                    uint32_t oldSysThreadId = parseNumber(line); ++line;
                    PL_SEARCH_TEXT(line<next-9 && memcmp(line, "next_comm", 9)!=0, 10);
                    parseString(line, &pidName2[0], 32);
                    PL_SEARCH_TEXT(line<next-8 && memcmp(line, "next_pid", 8)!=0, 9);
                    uint32_t newSysThreadId = parseNumber(line);

                    // Convert POSIX PID into our thread IDs
                    int oldThreadId = PL_CSWITCH_CORE_NONE, newThreadId = PL_CSWITCH_CORE_NONE;
                    for(int threadId=0; threadId<threadQty; ++threadId) {
                        uint32_t tid = globalCtx.threadInfos[threadId].pid;
                        if(oldSysThreadId==tid) { oldThreadId = threadId; if(newThreadId!=PL_CSWITCH_CORE_NONE) break; }
                        if(newSysThreadId==tid) { newThreadId = threadId; if(oldThreadId!=PL_CSWITCH_CORE_NONE) break; }
                    }

                    // Store the external process strings
                    uint32_t oldNameIdx = (oldSysThreadId==0)? 0xFFFFFFFE : 0xFFFFFFFF;
                    if(oldNameIdx==0xFFFFFFFF && oldThreadId==PL_CSWITCH_CORE_NONE) { // Not idle & not a thread of us
                        hashStr_t strHash = hashString(&pidName1[0]);
                        PL_PRIV_PROCESS_STRING(strHash, &pidName1[0], oldNameIdx);
                    }
                    uint32_t newNameIdx = (newSysThreadId==0)? 0xFFFFFFFE : 0xFFFFFFFF;
                    if(newNameIdx==0xFFFFFFFF && newThreadId==PL_CSWITCH_CORE_NONE) {
                        hashStr_t strHash = hashString(&pidName2[0]);
                        PL_PRIV_PROCESS_STRING(strHash, &pidName2[0], newNameIdx);
                    }

                    // Store the data in place (2 times 18 bytes stored, and a line is more than 36 bytes in any cases)
                    EventExt& dst1  = dstBuffer[dstEventQty++];
                    dst1.threadId   = oldThreadId;
                    dst1.flags      = PL_FLAG_TYPE_CSWITCH;
                    dst1.lineNbr    = 0;
                    dst1.prevCoreId = coreId;
                    dst1.newCoreId  = PL_CSWITCH_CORE_NONE;
                    dst1.nameIdx    = oldNameIdx;
                    dst1.PL_PRIV_RAW_FIELD = (bigRawData_t)timeValueNs;

                    EventExt& dst2 = dstBuffer[dstEventQty++];
                    dst2.threadId   = newThreadId;
                    dst2.flags      = PL_FLAG_TYPE_CSWITCH;
                    dst2.lineNbr    = 0;
                    dst2.prevCoreId = PL_CSWITCH_CORE_NONE;
                    dst2.newCoreId  = coreId;
                    dst2.nameIdx    = newNameIdx;
                    dst2.PL_PRIV_RAW_FIELD = (bigRawData_t)timeValueNs;
                }

                else if(memcmp(line, "softirq_e", 9)==0) {
                    bool isEntry = (line[9]=='n'); // Either 'n' or 'x'...

                    // Convert POSIX PID into our thread IDs
                    int threadId = 0;
                    while(threadId<threadQty && curPid!=globalCtx.threadInfos[threadId].pid) ++threadId;

                    if(threadId<threadQty) {
                        // Get the action (line is like:   <idle>-0     [000]  1541931773185958: softirq_entry: vec=7 [action=SCHED] )
                        line += 21;
                        PL_SEARCH_TEXT(line<next-7 && memcmp(line, "action=", 7)!=0, 0);
                        char* dstStr  = &pidName1[0];
                        while(*line!='\n' && *line!=']' && dstStr-&pidName1[0]<(int)sizeof(pidName1)-1) *dstStr++ = *line++;
                        *dstStr = 0;

                        // Store the action  strings
                        uint32_t  actionNameIdx = 0xFFFFFFFF;
                        hashStr_t strHash = hashString(&pidName1[0]);
                        PL_PRIV_PROCESS_STRING(strHash, &pidName1[0], actionNameIdx);

                        // Store the data in place (18 bytes stored, and a line is more than that in any cases)
                        EventExt& dst1  = dstBuffer[dstEventQty++];
                        dst1.threadId   = threadId;
                        dst1.flags      = PL_FLAG_TYPE_SOFTIRQ | (isEntry? PL_FLAG_SCOPE_BEGIN : PL_FLAG_SCOPE_END);
                        dst1.lineNbr    = 0;
                        dst1.prevCoreId = coreId;
                        dst1.newCoreId  = coreId;
                        dst1.nameIdx    = actionNameIdx;
                        dst1.PL_PRIV_RAW_FIELD = (bigRawData_t)timeValueNs;
                    }
                }

                // Next line
                line = next;
            } // Loop on lines
            plgEnd(PL_VERBOSE, "parse lines");

            // Write (file case) or send (socket case) the buffer
            plgBegin(PL_VERBOSE, "sending ctx switches");
            ic.stats.sentEventQty += dstEventQty;
            if(stringQty)   sendStrings(stringQty);
            if(dstEventQty) sendEvents (dstEventQty, (uint8_t*)(ic.cswitchPollBuffer), PL_DATA_TYPE_EVENT_AUX, 0);
            plgEnd(PL_VERBOSE, "sending ctx switches");

            if(!doForce && dstEventQty<16) break; // No need to iterate if very few events collected
        } // Loop on data buffers

        plgEnd(PL_VERBOSE, "collectCtxSwitch");
        return wasWorkedDone;
    }
#endif // if defined(__unix__) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1

#if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    // On windows, the context switch events are logged in a dedicated thread. This is its handler
    static void
    collectCtxSwitch(void)
    {
        plgDeclareThread(PL_VERBOSE_CS_CBK, "Palanteer/winTraceLogger");
        auto& ic = implCtx;
        // Increase its priority so that we do not lose context switch events (also processing time is small)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
        // Blocking call (windows API) until tracing is stopped
        ProcessTrace(&ic.cswitchConsumerHandle, 1, 0, 0);
        // Stop properly the tracing
        ControlTrace(0, KERNEL_LOGGER_NAME, ic.cswitchProperties, EVENT_TRACE_CONTROL_STOP);
        free(ic.cswitchProperties);
    }


    // Definition copied from https://docs.microsoft.com/en-us/windows/win32/etw/cswitch
    struct EventCSwitch
    {
        uint32_t newThreadId;
        uint32_t oldThreadId;
        int8_t   newThreadPriority;
        int8_t   oldThreadPriority;
        uint8_t  previousCState;
        int8_t   spareByte;
        int8_t   oldThreadWaitReason;
        int8_t   oldThreadWaitMode;
        int8_t   oldThreadState;
        int8_t   oldThreadWaitIdealProcessor;
        uint32_t newThreadWaitTime;
        uint32_t reserved;
    };

    // Not in the Palanteer reception thread: check shall be activated
#undef PL_STORE_COLLECT_CASE_
#define PL_STORE_COLLECT_CASE_ 0

    static void
    WINAPI
    eventRecordCallback(EVENT_RECORD* record)
    {
        // Filtering on event with provided: Thread Guid("{3d6fa8d1-fe05-11d0-9dda-00c04fd7ba7c}")
        //  and EventType{36}, EventTypeName{"CSwitch"}
        if(!PL_IS_ENABLED_()) return;
        EVENT_HEADER& h = record->EventHeader;
        if(h.ProviderId.Data1!=0x3d6fa8d1 || h.EventDescriptor.Opcode!=36) return;
        plgBegin(PL_VERBOSE_CS_CBK, "eventRecordCallback");

        // 1 Hz clock re-synchronization
#if PL_SHORT_DATE==1
        // Works as long as the checked period (=0.5s) is less than the wrap period
        int64_t deltaTicks = (int64_t)plPriv::getClockTick()-(int64_t)implCtx.rdtscRef;
        if(deltaTicks<=0) deltaTicks += (1LL<<32);
#else
        int64_t deltaTicks = plPriv::getClockTick()-implCtx.rdtscRef;
#endif
        if(implCtx.tickToNs*deltaTicks>5e8) {
            LARGE_INTEGER qpc;
            QueryPerformanceCounter(&qpc);
            implCtx.rdtscRef = plPriv::getClockTick();
            implCtx.qpcRef   = qpc.QuadPart;
        }

        // Extract useful fields
        uint64_t evtTime = (uint64_t)((int64_t)(implCtx.qpcToRdtsc*((int64_t)(h.TimeStamp.QuadPart-implCtx.qpcRef)))+implCtx.rdtscRef);
        int      coreId  = record->BufferContext.ProcessorNumber;
        uint32_t oldSysThreadId = ((EventCSwitch*)record->UserData)->oldThreadId;
        uint32_t newSysThreadId = ((EventCSwitch*)record->UserData)->newThreadId;

        // Convert system thread IDs into our thread IDs
        int threadQty   = globalCtx.nextThreadId;
        int oldThreadId = PL_CSWITCH_CORE_NONE, newThreadId = PL_CSWITCH_CORE_NONE;
        for(int threadId=0; threadId<threadQty; ++threadId) {
            uint32_t tid = globalCtx.threadInfos[threadId].pid;
            if(oldSysThreadId==tid) { oldThreadId = threadId; if(newThreadId!=PL_CSWITCH_CORE_NONE) break; }
            if(newSysThreadId==tid) { newThreadId = threadId; if(oldThreadId!=PL_CSWITCH_CORE_NONE) break; }
        }

        // Idle            : threadId =PL_CSWITCH_CORE_NONE and sysThreadId=0
        // External process: threadId =PL_CSWITCH_CORE_NONE and sysThreadId=N strictly positif
        // Internal process: threadId!=PL_CSWITCH_CORE_NONE and sysThreadID=N/A
        eventLogCSwitch(oldThreadId, oldSysThreadId, coreId, PL_CSWITCH_CORE_NONE, (clockType_t)evtTime);
        eventLogCSwitch(newThreadId, newSysThreadId, PL_CSWITCH_CORE_NONE, coreId, (clockType_t)evtTime);
        plgEnd(PL_VERBOSE_CS_CBK, "eventRecordCallback");
    }

    // Deactivate back the checks
#undef PL_STORE_COLLECT_CASE_
#define PL_STORE_COLLECT_CASE_ 1

#endif // if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1


    static void
    transmitToServer(void)
    {
        auto& ic = implCtx;
#if PL_NOCONTROL==0
        if(ic.mode!=PL_MODE_STORE_IN_FILE) {
            // Create the reception thread only if remote control is enabled and not storage on file
            plAssert(PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY >=64, "A minimum buffer size is required", PL_IMPL_REMOTE_REQUEST_BUFFER_BYTE_QTY);
            plAssert(PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY>=64, "A minimum buffer size is required", PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY);
            ic.rxIsStarted = false;
            ic.threadServerRx = new std::thread(plPriv::receiveFromServer);

            // Wait that the process is unfrozen by the server
            std::unique_lock<std::mutex> lk(ic.threadInitMx);
            while(!ic.threadInitCvTx.wait_for(lk, std::chrono::milliseconds(50), [&] { return ic.rxIsStarted; })) {
#if PL_NOEVENT==0
                updateDateWrap(PL_GET_CLOCK_TICK_FUNC());
#endif
                collectResponse();
            }
        }
#endif

        // Start the collection
#if PL_NOEVENT==0
        updateDateWrap(PL_GET_CLOCK_TICK_FUNC());
        implCtx.bankPreDateTick   [1] = implCtx.lastWrapCheckDateTick;  // Before any dated event
        implCtx.bankPreDateWrapQty[1] = implCtx.lastDateWrapQty;
#endif
        globalCtx.enabled        = true;
        globalCtx.collectEnabled = true;
        plgDeclareThread(PL_VERBOSE, "Palanteer/Transmission");
        plgLogInfo(PL_VERBOSE, "threading", "Start of Palanteer transmission loop");

        ic.lastSentEventBufferTick = PL_GET_CLOCK_TICK_FUNC();
        ic.txThreadId              = PL_GET_SYS_THREAD_ID();  // So that we can identify this thread if it crashes

        {
#if PL_NOEVENT==0
            // Send the already known threads names, either because set before the start of the service, either from a previous service activation.
            int tId = 0;
            while(tId<PL_MAX_THREAD_QTY && globalCtx.threadInfos[tId].nameHash!=0) {
                plPriv::ThreadInfo_t& ti = plPriv::globalCtx.threadInfos[tId];
                uint32_t bi = globalCtx.bankAndIndex.fetch_add(1);
                EventInt& e = globalCtx.collectBuffers[bi>>31][bi&EVTBUFFER_MASK_INDEX];
                e.threadId     = (uint8_t)tId;  // We are obliged to expand the event building due to this field...
                e.flags        = PL_FLAG_TYPE_THREADNAME;
                e.lineNbr      = 0;
                e.filenameHash = PL_STRINGHASH("");
                e.nameHash     = ti.nameHash;
                e.filename     = PL_EXTERNAL_STRINGS?0:"";
                e.name         = ti.name[0]? ti.name : 0;
                e.PL_PRIV_RAW_FIELD = 0;
                e.writeAck     = 1;
                ++tId;
            }
#endif

            // Notify the initialization thread that transmission thread is ready
            std::lock_guard<std::mutex> lk(ic.threadInitMx);
            ic.txIsStarted = true;
            ic.threadInitCv.notify_one();
        }

#if PL_NOEVENT==0 && defined(__unix__) && PL_IMPL_CONTEXT_SWITCH==1
        uint32_t count = 0; // Used for context switch sub-period
#endif

        // Collection loop
        // ===============
        while(!ic.threadServerFlagStop.load()) {
            bool workWasDone = false;

#if PL_NOCONTROL==0
            // Collect remote command response
            collectResponse();
#endif

#if PL_NOEVENT==0
            // Collect events
            if(collectEvents(false)) workWasDone = true;

            // Collect context switches on linux
            // Note: on windows, they are collected in a dedicated thread (due to windows API) and injected as standard events
#if defined(__unix__) && PL_IMPL_CONTEXT_SWITCH==1
            if(ic.cswitchPollEnabled && collectCtxSwitch(false))  workWasDone = true;
            ++count;
#endif // if defined(__unix__) && PL_IMPL_CONTEXT_SWITCH==1
#endif // if PL_NOEVENT==0

            // Sleep only if no work was done
            if(!workWasDone) {
                std::unique_lock<std::mutex> lk(ic.txThreadSyncMx);
                ic.txThreadSyncCv.wait_for(lk, std::chrono::milliseconds(5),
                                           [&] { return ic.threadServerFlagStop.load() || ic.rspBufferSize.load()>0; });
            }
        } // End of collection loop

#if PL_NOEVENT==0
        // Finish the collection on the two generic event banks and the context switch
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
#if defined(_WIN32) && PL_IMPL_CONTEXT_SWITCH==1
        if(ic.cswitchPollEnabled) {
            // Closing the trace should theoretically stop the logging thread (not always, alas)
            CloseTrace(ic.cswitchSessionHandle);
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Need to wait a bit so that all is processed (else thread is blocked...)
            CloseTrace(ic.cswitchConsumerHandle);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
#endif // if defined(_WIN32) && PL_IMPL_CONTEXT_SWITCH==1
        plgLogInfo(PL_VERBOSE, "threading", "End of Palanteer transmission loop");
        collectEvents(true); // Flush the previous bank
        collectEvents(true); // Flush the current bank
        collectEvents(true); // Flush the last collect thread round infos

#if defined(__unix__) && PL_IMPL_CONTEXT_SWITCH==1
#define PL_WRITE_TRACE_(path, valueStr, isCritical)                     \
        if(ic.cswitchPollEnabled) {                                     \
            snprintf(tmpStr, sizeof(tmpStr), "/sys/kernel/debug/tracing/%s", path); \
            tracerFd = open(tmpStr, O_WRONLY);                          \
            if(tracerFd>=0) {                                           \
                if(write(tracerFd, valueStr, strlen(valueStr))!=strlen(valueStr)) { \
                    if(isCritical) ic.cswitchPollEnabled = false;       \
                }                                                       \
                close(tracerFd);                                        \
            }                                                           \
            else if(isCritical) ic.cswitchPollEnabled = false;          \
        }
        // Cleaning context switches
        if(ic.cswitchPollEnabled) {
            // Collect the last context switches including the previous event collection
            collectCtxSwitch(true);

            // Disable the tracing
            close(ic.cswitchPollFd.fd);
            delete[] ic.cswitchPollBuffer; ic.cswitchPollBuffer = 0;
            char tmpStr[64];
            int  tracerFd;
            PL_WRITE_TRACE_("events/enable", "0", false); // Disable all kernel events
            PL_WRITE_TRACE_("tracing_on",    "0", true);
        }
#endif // defined(__unix__) && PL_IMPL_CONTEXT_SWITCH==1
        ic.cswitchPollEnabled = false;
#endif // if PL_NOEVENT==0

        // Cleaning communication
        palComUninit();
    }

    // End of trick to collect events even with saturated buffers
#undef PL_STORE_COLLECT_CASE_
#define PL_STORE_COLLECT_CASE_ 0

#endif // if PL_NOCONTROL==0 || PL_NOEVENT==0



    // =======================================================================================================
    // [PRIVATE IMPLEMENTATION] Signals and exception handlers
    // =======================================================================================================

    static void
    signalHandler(int signalId)
    {
        const char* sigDescr = "*Unknown*";
        switch(signalId) {
        case SIGABRT: sigDescr = "Abort"; break;
        case SIGFPE:  sigDescr = "Floating point exception"; break;
        case SIGILL:  sigDescr = "Illegal instruction"; break;
        case SIGSEGV: sigDescr = "Segmentation fault"; break;
        case SIGINT:  sigDescr = "Interrupt"; break;
        case SIGTERM: sigDescr = "Termination"; break;
#if defined(__unix__)
        case SIGPIPE: sigDescr = "Broken pipe"; break;
#endif
        default: break;
        }
        char infoStr[256];
        snprintf(infoStr, sizeof(infoStr), "[PALANTEER] signal %d '%s' received", signalId, sigDescr);
        plCrash(infoStr);
    }

#if _WIN32
    // Specific to windows, on top of the signal handler
    static LONG WINAPI
    exceptionHandler(struct _EXCEPTION_POINTERS* pExcept)
    {
        char infoStr[256];
        int  tmp;
        unsigned int code = pExcept->ExceptionRecord->ExceptionCode;
#define PL_LOG_EXCEPTION_(str)                                          \
        snprintf(infoStr, sizeof(infoStr), "[PALANTEER] exception '%s' received.", str); \
        plCrash(infoStr)

        switch(code) {
        case EXCEPTION_ACCESS_VIOLATION:
            tmp = (int)pExcept->ExceptionRecord->ExceptionInformation[0];
            snprintf(infoStr, sizeof(infoStr), "[PALANTEER] exception 'ACCESS_VIOLATION' (%s) received.",
                     (tmp==0)? "read":((tmp==1)? "write":"user-mode data execution prevention (DEP) violation"));
            plCrash(infoStr);
            break;
        case EXCEPTION_BREAKPOINT:            break; // Let this one go through the handler
        case EXCEPTION_SINGLE_STEP:           break; // Let this one go through the handler
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: PL_LOG_EXCEPTION_("ARRAY_BOUNDS_EXCEEDED"); break;
        case EXCEPTION_DATATYPE_MISALIGNMENT: PL_LOG_EXCEPTION_("DATATYPE_MISALIGNMENT"); break;
        case EXCEPTION_FLT_DENORMAL_OPERAND:  PL_LOG_EXCEPTION_("FLT_DENORMAL_OPERAND"); break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:    PL_LOG_EXCEPTION_("FLT_DIVIDE_BY_ZERO"); break;
        case EXCEPTION_FLT_INEXACT_RESULT:    PL_LOG_EXCEPTION_("FLT_INEXACT_RESULT"); break;
        case EXCEPTION_FLT_INVALID_OPERATION: PL_LOG_EXCEPTION_("FLT_INVALID_OPERATION"); break;
        case EXCEPTION_FLT_OVERFLOW:          PL_LOG_EXCEPTION_("FLT_OVERFLOW"); break;
        case EXCEPTION_FLT_STACK_CHECK:       PL_LOG_EXCEPTION_("FLT_STACK_CHECK"); break;
        case EXCEPTION_FLT_UNDERFLOW:         PL_LOG_EXCEPTION_("FLT_UNDERFLOW"); break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:   PL_LOG_EXCEPTION_("ILLEGAL_INSTRUCTION"); break;
        case EXCEPTION_IN_PAGE_ERROR:         PL_LOG_EXCEPTION_("IN_PAGE_ERROR"); break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:    PL_LOG_EXCEPTION_("INT_DIVIDE_BY_ZERO"); break;
        case EXCEPTION_INT_OVERFLOW:          PL_LOG_EXCEPTION_("INT_OVERFLOW"); break;
        case EXCEPTION_INVALID_DISPOSITION:   PL_LOG_EXCEPTION_("INVALID_DISPOSITION"); break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: PL_LOG_EXCEPTION_("NONCONTINUABLE_EXCEPTION"); break;
        case EXCEPTION_PRIV_INSTRUCTION:      PL_LOG_EXCEPTION_("PRIV_INSTRUCTION"); break;
        case EXCEPTION_STACK_OVERFLOW:        PL_LOG_EXCEPTION_("STACK_OVERFLOW"); break;
        default: PL_LOG_EXCEPTION_("UNKNOWN"); break;
        }
        // Go to the next handler
        return EXCEPTION_CONTINUE_SEARCH;
    }
#endif // if _WIN32

} // namespace plPriv



// Not private so that it is easier to break into it under debugger
void
plCrash(const char* message)
{
#if PL_NOCONTROL==0 || PL_NOEVENT==0
    // Do not log if the crash is located inside the Palanteer transmisison thread
    if(plPriv::implCtx.threadServerTx && (int)PL_GET_SYS_THREAD_ID()==plPriv::implCtx.txThreadId) {
        plPriv::globalCtx.enabled = false;
        plPriv::globalCtx.collectEnabled = false;
    }
#endif

    // Log and display the crash message
    plLogError("CRASH", "%s", message);
#if PL_IMPL_CONSOLE_COLOR==1
    PL_IMPL_PRINT_STDERR("\033[91m", true, false);  // Red
#endif
    PL_IMPL_PRINT_STDERR(message, true, false);
#if PL_IMPL_CONSOLE_COLOR==1
    PL_IMPL_PRINT_STDERR("\033[0m", true, false); // Standard
#endif
    PL_IMPL_PRINT_STDERR("\n", true, false);

    // Log and display the call stack
    PL_IMPL_STACKTRACE_FUNC();
    PL_IMPL_PRINT_STDERR("\n", true, true); // End of full crash display

    // Properly stop any recording, but without cleaning
    plPriv::implCtx.doNotUninit = true;
    plStopAndUninit();

    // Stop the process
    PL_IMPL_CRASH_EXIT_FUNC();
}


// =======================================================================================================
// [PUBLIC IMPLEMENTATION] Allocator overload
// =======================================================================================================

#if PL_NOEVENT==0 && PL_IMPL_OVERLOAD_NEW_DELETE==1

#if defined(__clang__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define PL_BUG_CLANG_ASAN_NEW_OVERLOAD // See clang bug https://bugs.llvm.org/show_bug.cgi?id=19660
#endif
#endif

#if !defined(PL_BUG_CLANG_ASAN_NEW_OVERLOAD)

#define PL_NEW_(ptr, size) malloc(size); if(PL_IS_ENABLED_()) { plPriv::eventLogAlloc(ptr, (uint32_t)(size)); }
#define PL_DELETE_(ptr)    if(PL_IS_ENABLED_()) { plPriv::eventLogDealloc(ptr); } free(ptr)

// @#LATER Handle the alignments stuff
void* operator new  (std::size_t size) noexcept(false)                 { void* ptr = PL_NEW_(ptr, size); return(ptr); }
void* operator new[](std::size_t size) noexcept(false)                 { void* ptr = PL_NEW_(ptr, size); return(ptr); }
void* operator new  (std::size_t size, const std::nothrow_t &) throw() { void* ptr = PL_NEW_(ptr, size); return(ptr); }
void* operator new[](std::size_t size, const std::nothrow_t &) throw() { void* ptr = PL_NEW_(ptr, size); return(ptr); }

void operator delete  (void* ptr) noexcept                        { PL_DELETE_(ptr); }
void operator delete[](void* ptr) noexcept                        { PL_DELETE_(ptr); }
void operator delete  (void* ptr, std::size_t size) noexcept      { PL_DELETE_(ptr); PL_UNUSED(size); }
void operator delete[](void* ptr, std::size_t size) noexcept      { PL_DELETE_(ptr); PL_UNUSED(size); }
void operator delete  (void *ptr, const std::nothrow_t&) noexcept { PL_DELETE_(ptr); }
void operator delete[](void *ptr, const std::nothrow_t&) noexcept { PL_DELETE_(ptr); }

#endif // #if !defined(PL_BUG_CLANG_ASAN_NEW_OVERLOAD)
#endif // if PL_NOEVENT==0 && PL_IMPL_OVERLOAD_NEW_DELETE==1


// =======================================================================================================
// [PUBLIC IMPLEMENTATION] Automatic function instrumentation hooks
// =======================================================================================================

#if PL_NOEVENT==0 && PL_IMPL_AUTO_INSTRUMENT==1

extern "C"
void PL_NOINSTRUMENT
__cyg_profile_func_enter (void* funcAddress, void* caller) {
    (void)caller;
    if(PL_IS_ENABLED_() && plPriv::implCtx.baseVirtualAddress!=(char*)-1) {
        plPriv::hashStr_t baseHash = (plPriv::hashStr_t)((char*)funcAddress-plPriv::implCtx.baseVirtualAddress+PL_HASH_SALT);
        // baseHash is XORed with the hashed appName so that multiple auto-instrumented streams are directly supported
        plPriv::eventLogRaw((baseHash+1)^plPriv::implCtx.hashedAppName, baseHash^plPriv::implCtx.hashedAppName,
                            0, 0, 0, PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC());
    }
}


extern "C"
void PL_NOINSTRUMENT
__cyg_profile_func_exit (void* funcAddress, void* caller)  {
    (void)caller;
    if(PL_IS_ENABLED_() && plPriv::implCtx.baseVirtualAddress!=(char*)-1) {
        plPriv::hashStr_t baseHash = (plPriv::hashStr_t)((char*)funcAddress-plPriv::implCtx.baseVirtualAddress+PL_HASH_SALT);
        // baseHash is XORed with the hashed appName so that multiple auto-instrumented streams are directly supported
        plPriv::eventLogRaw((baseHash+1)^plPriv::implCtx.hashedAppName, baseHash^plPriv::implCtx.hashedAppName,
                            0, 0, 0, PL_STORE_COLLECT_CASE_, PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC());
    }
}

#endif // if PL_NOEVENT==0 && PL_IMPL_AUTO_INSTRUMENT==1


// =======================================================================================================
// [PUBLIC IMPLEMENTATION] Public API implementation for event logging
// =======================================================================================================

#if PL_NOCONTROL==0
void
plFreezePoint(void)
{
    // Is freeze mode enabled?
    auto& ic = plPriv::implCtx;
    if(!ic.frozenThreadEnabled.load()) return;

    // Mark the thread as frozen
    int tId = plPriv::getThreadId();
    if(tId>=PL_MAX_THREAD_QTY) return; // No freeze feature above the maximum thread quantity
    uint64_t mask = 1ULL<<tId;
    ic.frozenThreadBitmap.fetch_or(mask);
    ic.frozenThreadBitmapChange.fetch_or(mask);

    // Wait for unfreeze
    std::unique_lock<std::mutex> lk(ic.frozenThreadMx);
    ic.frozenThreadCv[tId].wait(lk, [&] { return !(ic.frozenThreadEnabled.load() && (ic.frozenThreadBitmap.load()&mask)); });
    // Unfrozen, force the thread bit to zero in case the freeze feature has been disabled
    ic.frozenThreadBitmap.fetch_and(~mask);
    ic.frozenThreadBitmapChange.fetch_or(mask);
}
#endif // if PL_NOCONTROL==0

void
plSetFilename(const char* filename)
{
    snprintf(plPriv::implCtx.filename, sizeof(plPriv::implCtx.filename), "%s", filename); // Safe and null terminated
}


void
plSetServer(const char* serverAddr, int serverPort)
{
    snprintf(plPriv::implCtx.serverAddr, sizeof(plPriv::implCtx.serverAddr), "%s", serverAddr);
    plPriv::implCtx.serverPort = serverPort;
}


void
plSetLogLevelRecord(plLogLevel level)
{
    plAssert(level>=PL_LOG_LEVEL_ALL && level<=PL_LOG_LEVEL_NONE, level);
    plPriv::globalCtx.minLogLevelRecord = level;
}


void
plSetLogLevelConsole(plLogLevel level)
{
    plAssert(level>=PL_LOG_LEVEL_ALL && level<=PL_LOG_LEVEL_NONE, level);
    plPriv::globalCtx.minLogLevelConsole = level;
}


void
plInitAndStart(const char* appName, plMode mode, const char* buildName, int serverConnectionTimeoutMsec)
{
    auto& ic = plPriv::implCtx;
    ic.mode = mode;
    (void)buildName;

    // Sanity
    static_assert(PL_MAX_THREAD_QTY<=254, "Maximum supported thread quantity reached (limitation on exchange structure side)");
    static_assert(PL_IMPL_COLLECTION_BUFFER_BYTE_QTY>(int)2*sizeof(plPriv::EventInt), "Too small collection buffer"); // Much more expected anyway...
    static_assert(PL_IMPL_DYN_STRING_QTY>=32, "Invalid configuration");  // Stack trace requires dynamic strings
#if PL_NOCONTROL==0 || PL_NOEVENT==0
#if PL_COMPACT_MODEL==1
    static_assert(sizeof(plPriv::EventExt)==12, "Bad size of compact exchange event structure");
#else
    static_assert(sizeof(plPriv::EventExt)==24, "Bad size of full exchange event structure");
#endif
    plAssert(!ic.allocCollectBuffer, "Double call to 'plInitAndStart' detected");
#if PL_IMPL_AUTO_INSTRUMENT==1 && (!defined(__GNUC__) || defined(__clang__))
    // Clang misses the file exclusion feature (-finstrument-functions-exclude-file-list) which is mandatory to avoid
    // "Larsen effect" by excluding palanteer.h and all functions from the standard library used when logging (atomic, thread...).
    #error "Sorry, auto instrumentation is supported only for GCC"
#endif
#endif

    // Register POSIX signals
    memset(ic.signalsOldHandlers, 0, sizeof(ic.signalsOldHandlers));
#if PL_IMPL_CATCH_SIGNALS==1
    ic.signalsOldHandlers[0] = std::signal(SIGABRT, plPriv::signalHandler);
    ic.signalsOldHandlers[1] = std::signal(SIGFPE,  plPriv::signalHandler);
    ic.signalsOldHandlers[2] = std::signal(SIGILL,  plPriv::signalHandler);
    ic.signalsOldHandlers[3] = std::signal(SIGSEGV, plPriv::signalHandler);
    ic.signalsOldHandlers[4] = std::signal(SIGINT,  plPriv::signalHandler);
    ic.signalsOldHandlers[5] = std::signal(SIGTERM, plPriv::signalHandler);
#if defined(__unix__)
    ic.signalsOldHandlers[6] = std::signal(SIGPIPE, plPriv::signalHandler);
#endif
    ic.signalHandlersSaved = true;
#if defined(_WIN32)
    // Register the exception handler
    ic.exceptionHandler = AddVectoredExceptionHandler(1, plPriv::exceptionHandler);
#endif // if defined(_WIN32)
#endif // if PL_IMPL_CATCH_SIGNALS==1

#if defined(_WIN32) && PL_IMPL_STACKTRACE==1
    // Initialize the symbol reading for the stacktrace (in case of crash)
    SymInitialize(GetCurrentProcess(), 0, true);
    SymSetOptions(SYMOPT_LOAD_LINES);
    ic.rtlWalkFrameChain = (plPriv::rtlWalkFrameChain_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlWalkFrameChain");
    plAssert(ic.rtlWalkFrameChain);
#endif // if defined(_WIN32) && PL_IMPL_STACKTRACE==1

    // In inactive mode, nothing else to do
    if(mode==PL_MODE_INACTIVE) {
        return;
    }

    // Remove some warning for some configurations
    PL_UNUSED(appName);
    PL_UNUSED(serverConnectionTimeoutMsec);

#if PL_NOEVENT==0 && PL_VIRTUAL_THREADS==1
    for(int i=0; i<PL_MAX_THREAD_QTY; ++i) {
        // Other fields are not reset to have persistent thread names
        plPriv::globalCtx.threadInfos[i].isSuspended = false;
        plPriv::globalCtx.threadInfos[i].isBeginSent = false;
    }
#endif

#if PL_IMPL_AUTO_INSTRUMENT==1 && defined(__unix__) && (PL_NOCONTROL==0 || PL_NOEVENT==0)
    // Initialize the automatic instrumentation mode (gcc only) with -finstrument-functions
    ic.hashedAppName = plPriv::hashString(appName);
#if PL_IMPL_AUTO_INSTRUMENT_PIC==1
    Dl_info info;
    if(dladdr((void*)&plInitAndStart, &info)) {
        ic.baseVirtualAddress = (char*)info.dli_fbase;
    }
#else
    ic.baseVirtualAddress = 0;  // Absolute addresses
#endif  // PL_IMPL_AUTO_INSTRUMENT_PIC==1
#endif  // PL_IMPL_AUTO_INSTRUMENT==1 && defined(__unix__) && (PL_NOCONTROL==0 || PL_NOEVENT==0)

#if PL_NOCONTROL==0 || PL_NOEVENT==0
    // Measure the event's high performance clock frequency with the standard nanosecond clock
    int64_t highPerfT0 = PL_GET_CLOCK_TICK_FUNC();
    const auto stdT0 = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int64_t highPerfT1 = PL_GET_CLOCK_TICK_FUNC();
    const auto stdT1 = std::chrono::high_resolution_clock::now();
    // This coefficient will be sent to the server
#if PL_SHORT_DATE==1
    if(highPerfT1<=highPerfT0) highPerfT1 += (1LL<<32);
#endif
    ic.tickToNs = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(stdT1-stdT0).count()/(double)(highPerfT1-highPerfT0);

#if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    // Windows: compute the clock conversion information for the context switch
    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    ic.rdtscRef = plPriv::getClockTick();
    ic.qpcRef   = qpc.QuadPart;
    QueryPerformanceFrequency(&qpc);
    ic.qpcToRdtsc = 1e9/(qpc.QuadPart*ic.tickToNs);
#endif

    // Allocate the 2 collection banks (in one chunk, with a slight shift for a more efficient collectEvents())
    //   aligned on 64 bytes to match most cache lines (the internal event representation has also a size of 64 bytes)
    plPriv::globalCtx.collectBufferMaxEventQty = PL_IMPL_COLLECTION_BUFFER_BYTE_QTY/sizeof(plPriv::EventInt);
#if PL_NOEVENT==0
    plAssert((uint32_t)plPriv::globalCtx.collectBufferMaxEventQty<plPriv::EVTBUFFER_MASK_INDEX, "The collection buffer is too large");
#endif
    const int realBufferEventQty = plPriv::globalCtx.collectBufferMaxEventQty + (1+PL_MAX_THREAD_QTY)+64; // 64=margin for the collection thread
    int sendBufferSize = sizeof(plPriv::EventExt)*realBufferEventQty;
    if(sendBufferSize<PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY) sendBufferSize = PL_IMPL_REMOTE_RESPONSE_BUFFER_BYTE_QTY;
    ic.sendBuffer = new uint8_t[sendBufferSize+64];  // 64 = sent header margin
    ic.allocCollectBuffer = new uint8_t[sizeof(plPriv::EventInt)*2*realBufferEventQty+64];
    memset(ic.allocCollectBuffer, 0, sizeof(plPriv::EventInt)*2*realBufferEventQty+64);
    uint8_t* alignedAllocCollectBuffer = (uint8_t*)((((uintptr_t)ic.allocCollectBuffer)+64)&(uintptr_t)(~0x3F));
    plPriv::globalCtx.collectBuffers[0] = (plPriv::EventInt*)alignedAllocCollectBuffer;
    plPriv::globalCtx.collectBuffers[1] = plPriv::globalCtx.collectBuffers[0] + realBufferEventQty;

    // Initialize some fields
    memset(&ic.stats, 0, sizeof(plStats));
    ic.stats.collectBufferSizeByteQty = PL_IMPL_COLLECTION_BUFFER_BYTE_QTY;
    ic.stats.collectDynStringQty      = PL_IMPL_DYN_STRING_QTY;
    plPriv::globalCtx.bankAndIndex.store(0);
    ic.prevBankAndIndex = 1UL<<31;

    plPriv::palComInit(serverConnectionTimeoutMsec);
    if(ic.mode==PL_MODE_INACTIVE) return;


#if defined(__unix__) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    {
        ic.cswitchPollEnabled = true;
        // Configure the tracing
        char tmpStr[64];
        int  tracerFd;
        PL_WRITE_TRACE_("tracing_on",     "0",                   true); // Disables tracing while configuring
        PL_WRITE_TRACE_("current_tracer", "nop",                 true); // Removes all function tracers
        PL_WRITE_TRACE_("trace_options",  "noirq-info",         false); // No need for irq information
        PL_WRITE_TRACE_("trace_options",  "noannotate",         false); // No need for extra "annotate" infos
        PL_WRITE_TRACE_("trace_options",  "norecord-cmd",       false); // No need for extra thread info (PID are enough for our usage)
        PL_WRITE_TRACE_("trace_options",  "norecord-tgid",      false); // No need for the Thread Group ID
#if defined(__x86_64__)
        PL_WRITE_TRACE_("trace_clock",    "x86-tsc",             true); // Same clock than the default RDTSC one for Linux
#else
        PL_WRITE_TRACE_("trace_clock",    "mono",                true); // Usually (depending on arch) same as std::chrono::steady_clock but lower precision than RDTSC
#endif
        PL_WRITE_TRACE_("events/enable",  "0",                  false); // Disable all kernel events
        PL_WRITE_TRACE_("events/sched/sched_switch/enable", "1", true); // Enable the events we want
        PL_WRITE_TRACE_("events/irq/softirq_entry/enable",  "1", true);
        PL_WRITE_TRACE_("events/irq/softirq_exit/enable",   "1", true);
        PL_WRITE_TRACE_("buffer_size_kb", "512",                 true); // Reserve 512KB for exchanges
        PL_WRITE_TRACE_("tracing_on",     "1",                   true); // Enable tracing

        // Open the exchange pipe
        if(ic.cswitchPollEnabled && (ic.cswitchPollFd.fd=open("/sys/kernel/debug/tracing/trace_pipe", O_RDONLY))>=0) {
            ic.cswitchPollFd.events = POLLIN | POLLERR;
            ic.cswitchPollBuffer    = new char[plPriv::SWITCH_CTX_BUFFER_SIZE];
        } else ic.cswitchPollEnabled = false;
    }
#endif //if defined(__unix__) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1

#if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    // See https://caseymuratori.com/blog_0025: "The Worst API Ever Made"

    // Allocate the tracer "property" structure as intended by the API
    ULONG propertySize = sizeof(EVENT_TRACE_PROPERTIES)+sizeof(KERNEL_LOGGER_NAME);
    ic.cswitchProperties = (EVENT_TRACE_PROPERTIES*)malloc(propertySize);
    memset(ic.cswitchProperties, 0, propertySize);
    // Fill the properties
    ic.cswitchProperties->EnableFlags      = EVENT_TRACE_FLAG_CSWITCH;   // That is what we want, context switches
    ic.cswitchProperties->LogFileMode      = EVENT_TRACE_REAL_TIME_MODE; // No file, retrieval through a callback
    ic.cswitchProperties->Wnode.BufferSize = propertySize;
    ic.cswitchProperties->Wnode.Flags      = WNODE_FLAG_TRACED_GUID;
    ic.cswitchProperties->Wnode.Guid       = SystemTraceControlGuid;
    ic.cswitchProperties->BufferSize       = 8; // In KB. Buffers are flushed when full, so small to be flushed often (else it is 1 Hz)
    ic.cswitchProperties->MinimumBuffers   = 1*PL_MAX_THREAD_QTY;
    ic.cswitchProperties->MaximumBuffers   = 4*PL_MAX_THREAD_QTY;
    ic.cswitchProperties->Wnode.ClientContext = 1;                 // 1 means rdtsc timer
    ic.cswitchProperties->LoggerNameOffset    = sizeof(EVENT_TRACE_PROPERTIES);
    memcpy(((char*)ic.cswitchProperties)+sizeof(EVENT_TRACE_PROPERTIES), KERNEL_LOGGER_NAME, sizeof(KERNEL_LOGGER_NAME));
    ic.cswitchPollEnabled = true; // Let's be optimistic

    // Stop the previous tracing (persistent across processes...) if not stopped properly by the last process using it.
    //  Of course, it modifies the property structure when the call really stops the previous tracing, hence the save/restore
    EVENT_TRACE_PROPERTIES save = *ic.cswitchProperties;
    ControlTrace(0, KERNEL_LOGGER_NAME, ic.cswitchProperties, EVENT_TRACE_CONTROL_STOP);
    *ic.cswitchProperties = save;

    // Start the tracing
    // Note: we will fail here if the executable is not run as administrator (not enough privileges)
    if(ic.cswitchPollEnabled && StartTrace(&ic.cswitchSessionHandle, KERNEL_LOGGER_NAME, ic.cswitchProperties)!=ERROR_SUCCESS) {
        ic.cswitchPollEnabled = false;
    }

    // Configure the logging. Indeed, tracing just activates the event collection so logging is required to retrieve the events...
    EVENT_TRACE_LOGFILE LogFile = {0};
#if defined(_UNICODE) || defined(UNICODE)
    LogFile.LoggerName          = (LPWSTR)KERNEL_LOGGER_NAME;
#else
    LogFile.LoggerName          = (LPSTR)KERNEL_LOGGER_NAME;
#endif
    LogFile.ProcessTraceMode    = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP;
    LogFile.EventRecordCallback = plPriv::eventRecordCallback;

    if(ic.cswitchPollEnabled) {
        ic.cswitchConsumerHandle = OpenTrace(&LogFile);
        if(ic.cswitchConsumerHandle==(TRACEHANDLE)INVALID_HANDLE_VALUE) {
            CloseTrace(ic.cswitchSessionHandle);
            ic.cswitchPollEnabled = false;
        }
    }

    if(ic.cswitchPollEnabled) { // Successful activation
        ic.cswitchTraceLoggerThread = new std::thread(plPriv::collectCtxSwitch);
    } else free(ic.cswitchProperties); // Fail to activate the context switch logging
#endif // if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1


    // Build the data exchange header
    int appNameLength   = (int)strlen(appName)+1;
    int buildNameLength = (buildName && buildName[0]!=0)? (int)strlen(buildName)+1 : 0;
#ifndef PL_PRIV_IMPL_LANGUAGE
#define PL_PRIV_IMPL_LANGUAGE "C++"
#endif
    int langNameLength  = strlen(PL_PRIV_IMPL_LANGUAGE)? (int)strlen(PL_PRIV_IMPL_LANGUAGE)+1 : 0;
    int tlvTotalSize    = 6/*Protocol TLV*/ + 20/*Time unit and origin*/ + 4+appNameLength/* Application name TLV */;
    if(buildNameLength) tlvTotalSize += 4+buildNameLength; // Optional build name TLV
    if(langNameLength)  tlvTotalSize += 4+langNameLength;  // Optional language name TLV
#if PL_EXTERNAL_STRINGS==1
    tlvTotalSize += 4;
#endif
#if PL_SHORT_STRING_HASH==1
    tlvTotalSize += 4;
#endif
#if PL_NOCONTROL==1
    tlvTotalSize += 4;
#endif
#if PL_SHORT_DATE==1
    tlvTotalSize += 12;
#endif
#if PL_COMPACT_MODEL==1
    tlvTotalSize += 4;
#endif
#if PL_IMPL_AUTO_INSTRUMENT==1
    ic.hasAutoInstrument = true;
#endif
    if(ic.hasAutoInstrument) {
        tlvTotalSize += 4;
    }
    if(ic.cswitchPollEnabled) {
        tlvTotalSize += 4;
    }
#if PL_HASH_SALT!=0
    tlvTotalSize += 8;
#endif
    int      headerSize = 16+tlvTotalSize;
    uint8_t* header = (uint8_t*)alloca(headerSize*sizeof(uint8_t));
    memset(header, 0, headerSize*sizeof(uint8_t)); // For the padding
    // Write the magic string to discriminate the connection type
    for(int i=0; i<8; ++i) header[i] = ("PL-MAGIC"[i]);
    // Write the endianess detection (provision, as little endian is supposed at the moment)
    *(uint32_t*)&header[8]  = 0x12345678;
    // Write the size of TLV block
    header[12] = (tlvTotalSize>>24)&0xFF;
    header[13] = (tlvTotalSize>>16)&0xFF;
    header[14] = (tlvTotalSize>> 8)&0xFF;
    header[15] = (tlvTotalSize    )&0xFF;
    // Write TLVs in big endian, T=2 bytes L=2 bytes
    int offset = 16;
    // TLV Protocol
    header[offset+0] = PL_TLV_PROTOCOL>>8; header[offset+1] = PL_TLV_PROTOCOL&0xFF;
    header[offset+2] = 0; header[offset+3] = 2; // 2 bytes payload
    header[offset+4] = PALANTEER_CLIENT_PROTOCOL_VERSION>>8; header[offset+5] = PALANTEER_CLIENT_PROTOCOL_VERSION&0xFF;
    offset += 6;
    // TLV Clock info
    header[offset+0] = PL_TLV_CLOCK_INFO>>8; header[offset+1] = PL_TLV_CLOCK_INFO&0xFF;
    header[offset+2] = 0; header[offset+3] = 16; // 16 bytes payload
    uint64_t clockOriginTick = PL_GET_CLOCK_TICK_FUNC();
    header[offset+ 4] = (clockOriginTick>>56)&0xFF; header[offset+ 5] = (clockOriginTick>>48)&0xFF;
    header[offset+ 6] = (clockOriginTick>>40)&0xFF; header[offset+ 7] = (clockOriginTick>>32)&0xFF;
    header[offset+ 8] = (clockOriginTick>>24)&0xFF; header[offset+ 9] = (clockOriginTick>>16)&0xFF;
    header[offset+10] = (clockOriginTick>> 8)&0xFF; header[offset+11] = (clockOriginTick    )&0xFF;
    char* tmp1 = (char*)&ic.tickToNs; uint64_t tmp = *(uint64_t*)tmp1;      // Avoids warning about strict aliasing
    header[offset+12] = (tmp>>56)&0xFF; header[offset+13] = (tmp>>48)&0xFF; // Standard IEEE 754 format, big endian
    header[offset+14] = (tmp>>40)&0xFF; header[offset+15] = (tmp>>32)&0xFF;
    header[offset+16] = (tmp>>24)&0xFF; header[offset+17] = (tmp>>16)&0xFF;
    header[offset+18] = (tmp>> 8)&0xFF; header[offset+19] = (tmp    )&0xFF;
    offset += 20;
    // TLV App name
    header[offset+0] = PL_TLV_APP_NAME>>8; header[offset+1] = PL_TLV_APP_NAME&0xFF;
    header[offset+2] = (appNameLength>>8)&0xFF; header[offset+3] = appNameLength&0xFF;
    memcpy(&header[offset+4], appName, appNameLength);
    offset += 4+appNameLength;
    // TLV build name
    if(buildNameLength>0) {
        header[offset+0] = PL_TLV_HAS_BUILD_NAME>>8; header[offset+1] = PL_TLV_HAS_BUILD_NAME&0xFF;
        header[offset+2] = (buildNameLength>>8)&0xFF; header[offset+3] = buildNameLength&0xFF;
        memcpy(&header[offset+4], buildName, buildNameLength);
        offset += 4+buildNameLength;
    }
    // TLV language name
    if(langNameLength>0) {
        header[offset+0] = PL_TLV_HAS_LANG_NAME>>8; header[offset+1] = PL_TLV_HAS_LANG_NAME&0xFF;
        header[offset+2] = (langNameLength>>8)&0xFF; header[offset+3] = langNameLength&0xFF;
        memcpy(&header[offset+4], PL_PRIV_IMPL_LANGUAGE, langNameLength);
        offset += 4+langNameLength;
    }

#define ADD_TLV_FLAG(flag)                                              \
    header[offset+0] = (flag)>>8; header[offset+1] = (flag)&0xFF;       \
    header[offset+2] = 0; header[offset+3] = 0; /* 0 bytes payload */   \
    offset += 4
#if PL_EXTERNAL_STRINGS==1
    ADD_TLV_FLAG(PL_TLV_HAS_EXTERNAL_STRING);
#endif
#if PL_SHORT_STRING_HASH==1
    ADD_TLV_FLAG(PL_TLV_HAS_SHORT_STRING_HASH);
#endif
#if PL_NOCONTROL==1
    ADD_TLV_FLAG(PL_TLV_HAS_NO_CONTROL);
#endif
#if PL_COMPACT_MODEL==1
    ADD_TLV_FLAG(PL_TLV_HAS_COMPACT_MODEL);
#endif
    if(ic.hasAutoInstrument) {
        ADD_TLV_FLAG(PL_TLV_HAS_AUTO_INSTRUMENT);
    }
    if(ic.cswitchPollEnabled) {
        ADD_TLV_FLAG(PL_TLV_HAS_CSWITCH_INFO);
    }
    plPriv::globalCtx.originNs = PL_GET_SYSTEM_CLOCK_NS();  // The global system date
#if PL_SHORT_DATE==1
    header[offset+ 0] = PL_TLV_HAS_SHORT_DATE>>8; header[offset+1] = PL_TLV_HAS_SHORT_DATE&0xFF;
    header[offset+ 2] = 0; header[offset+3] = 8; // 8 bytes payload
    tmp = plPriv::globalCtx.originNs;
    header[offset+ 4] = (tmp>>56)&0xFF; header[offset+ 5] = (tmp>>48)&0xFF;
    header[offset+ 6] = (tmp>>40)&0xFF; header[offset+ 7] = (tmp>>32)&0xFF;
    header[offset+ 8] = (tmp>>24)&0xFF; header[offset+ 9] = (tmp>>16)&0xFF;
    header[offset+10] = (tmp>> 8)&0xFF; header[offset+11] = (tmp    )&0xFF;
    ic.lastWrapCheckDateTick = (uint32_t)clockOriginTick;  // The associated event timestamp in tick
    ic.lastDateWrapQty       = 0;
    offset += 12;
#endif
#if PL_HASH_SALT!=0
    header[offset+0] = PL_TLV_HAS_HASH_SALT>>8; header[offset+1] = PL_TLV_HAS_HASH_SALT&0xFF;
    header[offset+2] = 0; header[offset+3] = 4; // 4 bytes payload
    header[offset+4] = (PL_HASH_SALT>>24)&0xFF; header[offset+5] = (PL_HASH_SALT>>16)&0xFF;
    header[offset+5] = (PL_HASH_SALT>> 8)&0xFF; header[offset+7] = (PL_HASH_SALT    )&0xFF;
    offset += 8;
#endif
    plAssert(offset==headerSize);

    // Write/send the built header
    bool headerSendingStatus = plPriv::palComSend(&header[0], headerSize);
    plAssert(headerSendingStatus, "Unable to send the session header");

    {
        // Create the transmission thread and wait for its readiness
        plAssert(PL_IMPL_STRING_BUFFER_BYTE_QTY>=128, "A minimum buffer size is required", PL_IMPL_STRING_BUFFER_BYTE_QTY);
        ic.txIsStarted = false;
        ic.threadServerTx = new std::thread(plPriv::transmitToServer);
        std::unique_lock<std::mutex> lk(ic.threadInitMx);
        ic.threadInitCv.wait(lk, [&] { return ic.txIsStarted; });
    }

#endif // if PL_NOCONTROL==0 || PL_NOEVENT==0
}


void
plStopAndUninit(void)
{
    auto& ic = plPriv::implCtx; PL_UNUSED(ic);

    // Unregister signals
#if PL_IMPL_CATCH_SIGNALS==1
    if(ic.signalHandlersSaved) {
        std::signal(SIGABRT, ic.signalsOldHandlers[0]);
        std::signal(SIGFPE,  ic.signalsOldHandlers[1]);
        std::signal(SIGILL,  ic.signalsOldHandlers[2]);
        std::signal(SIGSEGV, ic.signalsOldHandlers[3]);
        std::signal(SIGINT,  ic.signalsOldHandlers[4]);
        std::signal(SIGTERM, ic.signalsOldHandlers[5]);
#if defined(__unix__)
        std::signal(SIGPIPE, ic.signalsOldHandlers[6]);
#endif
#if defined(_WIN32)
        RemoveVectoredExceptionHandler(ic.exceptionHandler);
#endif // if defined(_WIN32)
    }
#endif // if PL_IMPL_CATCH_SIGNALS==1

#if PL_NOCONTROL==0 || PL_NOEVENT==0
    // Stop the data collection thread
    plPriv::globalCtx.enabled = false;
    {
        // Notify end of collection thread and wake it up
        std::lock_guard<std::mutex> lk(ic.txThreadSyncMx);
        ic.threadServerFlagStop.store(1);
        ic.txThreadSyncCv.notify_one();
    }
    if(ic.doNotUninit) {
        // Wait for the TX thread to send the last data sending (unless it is the crashing thread)
        if(ic.threadServerTx && ic.threadServerTx->joinable() && (int)PL_GET_SYS_THREAD_ID()!=ic.txThreadId) ic.threadServerTx->join();
        // No cleaning, so stop here
        return;
    }
    if(ic.threadServerTx && ic.threadServerTx->joinable()) ic.threadServerTx->join();
    if(ic.threadServerRx && ic.threadServerRx->joinable()) ic.threadServerRx->join();
#if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    if(ic.cswitchTraceLoggerThread && ic.cswitchTraceLoggerThread->joinable()) {
        ic.cswitchTraceLoggerThread->join();
    }
#endif
    plPriv::globalCtx.collectEnabled = false;
    delete ic.threadServerTx; ic.threadServerTx = 0;
    delete ic.threadServerRx; ic.threadServerRx = 0;
#if defined(_WIN32) && PL_NOEVENT==0 && PL_IMPL_CONTEXT_SWITCH==1
    delete ic.cswitchTraceLoggerThread; ic.cswitchTraceLoggerThread = 0;
#endif

    // Restore the initial global state
    ic.threadServerFlagStop.store(0);
    delete[] ic.allocCollectBuffer; ic.allocCollectBuffer = 0;
    delete[] ic.sendBuffer; ic.sendBuffer = 0;
    plPriv::globalCtx.collectBuffers[0] = 0;
    plPriv::globalCtx.collectBuffers[1] = 0;
    plPriv::globalCtx.bankAndIndex.store(0);
    ic.prevBankAndIndex = 1UL<<31;
    ic.lkupStringToIndex.clear();
    ic.strBuffer.clear();
    ic.stringUniqueId = 0;
    ic.rxIsStarted = false;
    ic.txIsStarted = false;
    ic.frozenLastThreadBitmap = 0;
    ic.frozenThreadBitmap.store(0);
    ic.frozenThreadBitmapChange.store(0);
    ic.frozenThreadEnabled.store(0);

#endif // if PL_NOCONTROL==0 || PL_NOEVENT==0
}


plStats
plGetStats(void) { return plPriv::implCtx.stats; }


void
plDeclareVirtualThread(uint32_t externalVirtualThreadId, const char* format, ...)
{
    PL_UNUSED(externalVirtualThreadId); PL_UNUSED(format);
#if PL_NOEVENT==0
#if PL_VIRTUAL_THREADS==1

    // Build the name
    char name[PL_DYN_STRING_MAX_SIZE];
    va_list args;
    va_start(args, format);
    (void)vsnprintf(name, sizeof(name), format, args);
    va_end(args);

    // Ensure that the OS thread owns an internal Id (for context switches at least)
    plPriv::ThreadContext_t* tCtx = &plPriv::threadCtx;
    if(tCtx->id==0xFFFFFFFF) plPriv::getThreadId();

    // Get the internal thread Id
    plPriv::hashStr_t hash = (PL_FNV_HASH_OFFSET_^externalVirtualThreadId)*PL_FNV_HASH_PRIME_;
    uint32_t   newThreadId = 0;
    if(plPriv::implCtx.vThreadLkupExtToCtx.find(hash, newThreadId)) return; // Nothing to do as the vThread is already declared and only the first call matters

    // Create this new internal thread and virtual thread context
    newThreadId = plPriv::globalCtx.nextThreadId.fetch_add(1);
    plPriv::implCtx.vThreadLkupExtToCtx.insert(hash, newThreadId);

    if(newThreadId<PL_MAX_THREAD_QTY) {
        // Store the thread infos
        plPriv::ThreadInfo_t& ti = plPriv::globalCtx.threadInfos[newThreadId];
        ti.pid = 0xFFFFFFFF; // Prevent OS thread matching for context switches
        ti.nameHash = plPriv::hashString(name);
        int nameLength = (int)strlen(name);
        if(nameLength) memcpy(ti.name, name, nameLength+1);
        ti.name[PL_DYN_STRING_MAX_SIZE-1] = 0;

        // Log the thread declaration event with this new thread ID (not the standard call, as we do not want to override the real OS thread data)
        if(PL_IS_ENABLED_()) {
            uint32_t prevInternalThreadId = tCtx->id;
            tCtx->id = newThreadId;
            plPriv::eventLogRawDynName(PL_STRINGHASH(PL_BASEFILENAME), PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, name, 0, 0, PL_FLAG_TYPE_THREADNAME, 0);
            tCtx->id = prevInternalThreadId; // Restore previous thread Id
        }
    }

#else
    plAssert(0, "plDeclareVirtualThread is a specific API of the 'virtual threads' feature which is not enabled. Add -DPL_VIRTUAL_THREADS=1 in your compilation options.");
#endif // if PL_VIRTUAL_THREADS==1
#endif // if PL_NOEVENT==0
}


void
plDetachVirtualThread(bool isSuspended)
{
    PL_UNUSED(isSuspended);
#if PL_NOEVENT==0
#if PL_VIRTUAL_THREADS==1
    plPriv::ThreadContext_t* tCtx = &plPriv::threadCtx;

    // Ensure that the OS thread Id owns an internal Id (for context switches at least)
    if(tCtx->id==0xFFFFFFFF) plPriv::getThreadId();

    if(tCtx->id==tCtx->realId) return; // Nothing to do
    int vThreadId = tCtx->id;

    if(vThreadId<PL_MAX_THREAD_QTY) {
        // Save the suspend state
        plPriv::globalCtx.threadInfos[vThreadId].isSuspended = isSuspended;

        // Suspension is represented as an interrupting IRQ
        if(isSuspended && PL_IS_ENABLED_()) {
            plPriv::eventLogRaw(PL_STRINGHASH(""), PL_STRINGHASH("Suspended"), PL_EXTERNAL_STRINGS?0:"", "Suspended", 0, 0,
                                PL_FLAG_TYPE_SOFTIRQ | PL_FLAG_SCOPE_BEGIN, PL_GET_CLOCK_TICK_FUNC());
        }

        // The worker thread is no more used (logged on the virtual threadId)
        if(tCtx->realRscNameHash!=0 && PL_IS_ENABLED_()) {
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), tCtx->realRscNameHash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, 0, 0, 0,
                                PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC());
        }
    }

    // Switch to OS thread Id
    tCtx->id = tCtx->realId;

    if(vThreadId<PL_MAX_THREAD_QTY && PL_IS_ENABLED_()) {
        plPriv::ThreadInfo_t& ti = plPriv::globalCtx.threadInfos[vThreadId];
        if(ti.nameHash!=0 && ti.isBeginSent) {
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), ti.nameHash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, 0, 0, 0,
                                PL_FLAG_SCOPE_END | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC());
            ti.isBeginSent = false;
        }
    }

#else  // if PL_VIRTUAL_THREADS==1
    plAssert(0, "plDetachVirtualThread is a specific API of the 'virtual thread' feature which is not enabled. Add -DPL_VIRTUAL_THREADS=1 in your compilation options.");
#endif // if PL_VIRTUAL_THREADS==1
#endif // if PL_NOEVENT==0
}


bool
plAttachVirtualThread(uint32_t externalVirtualThreadId)
{
    PL_UNUSED(externalVirtualThreadId);
    bool isNewVirtualThread = false;
#if PL_NOEVENT==0
#if PL_VIRTUAL_THREADS==1
    plPriv::ThreadContext_t* tCtx = &plPriv::threadCtx;

    // Ensure that the OS thread Id owns an internal Id (for context switches at least)
    if(tCtx->id==0xFFFFFFFF) plPriv::getThreadId();

    // Get the new virtual thread Id
    plPriv::hashStr_t hash = (PL_FNV_HASH_OFFSET_^externalVirtualThreadId)*PL_FNV_HASH_PRIME_;
    uint32_t vThreadId   = 0;
    if(!plPriv::implCtx.vThreadLkupExtToCtx.find(hash, vThreadId)) {
        // Create this new internal thread and virtual thread context
        vThreadId = plPriv::globalCtx.nextThreadId.fetch_add(1);
        plPriv::implCtx.vThreadLkupExtToCtx.insert(hash, vThreadId);
        isNewVirtualThread = true;
        if(vThreadId<PL_MAX_THREAD_QTY) {
            plPriv::globalCtx.threadInfos[vThreadId].pid = 0xFFFFFFFF; // Prevent OS thread matching for context switches
        }
    }
    if(tCtx->id==vThreadId) return isNewVirtualThread; // Nothing to do

    if(PL_IS_ENABLED_() && tCtx->realRscNameHash!=0 && tCtx->id!=tCtx->realId) {
        // The worker thread is switched to idle
        plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), tCtx->realRscNameHash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, 0, 0, 0,
                            PL_FLAG_TYPE_LOCK_RELEASED, PL_GET_CLOCK_TICK_FUNC());
    }

    if(vThreadId<PL_MAX_THREAD_QTY && PL_IS_ENABLED_()) {
        plPriv::ThreadInfo_t& ti = plPriv::globalCtx.threadInfos[vThreadId];
        if(ti.nameHash!=0 && !ti.isBeginSent) {
            tCtx->id = tCtx->realId;
            plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), ti.nameHash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, 0, 0, 0,
                                PL_FLAG_SCOPE_BEGIN | PL_FLAG_TYPE_DATA_TIMESTAMP, PL_GET_CLOCK_TICK_FUNC());
            ti.isBeginSent = true;
        }
    }

    // Overwrite the OS thread Id
    tCtx->id = vThreadId;

    // The worker thread is now dedicated to this virtual thread (logged on the worker threadId)
    if(vThreadId<PL_MAX_THREAD_QTY && plPriv::globalCtx.threadInfos[vThreadId].isSuspended && PL_IS_ENABLED_()) {
        plPriv::eventLogRaw(PL_STRINGHASH(""), PL_STRINGHASH("Suspended"), PL_EXTERNAL_STRINGS?0:"", "Suspended", 0, 0,
                            PL_FLAG_TYPE_SOFTIRQ | PL_FLAG_SCOPE_END, PL_GET_CLOCK_TICK_FUNC());
    }

    if(tCtx->realRscNameHash!=0 && PL_IS_ENABLED_()) {
        plPriv::eventLogRaw(PL_STRINGHASH(PL_BASEFILENAME), tCtx->realRscNameHash, PL_EXTERNAL_STRINGS?0:PL_BASEFILENAME, 0, 0, 0,
                            PL_FLAG_TYPE_LOCK_ACQUIRED, PL_GET_CLOCK_TICK_FUNC());
    }

#else  // if PL_VIRTUAL_THREADS==1
    plAssert(0, "plAttachVirtualThread is a specific API of the 'virtual thread' feature which is not enabled. Add -DPL_VIRTUAL_THREADS=1 in your compilation options.");
#endif // if PL_VIRTUAL_THREADS==1
#endif // if PL_NOEVENT==0
    return isNewVirtualThread;
}

#endif // if USE_PL==1 && PL_IMPLEMENTATION==1
