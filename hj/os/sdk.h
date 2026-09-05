/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SDK_H
#define SDK_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__cplusplus)
#include <type_traits>
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__)
#define SDK_EXPORT __declspec(dllexport)
#define SDK_IMPORT __declspec(dllimport)
#define SDK_LOCAL
#else
#if __GNUC__ >= 4 || defined(__clang__)
#define SDK_EXPORT __attribute__((visibility("default")))
#define SDK_IMPORT __attribute__((visibility("default")))
#define SDK_LOCAL __attribute__((visibility("hidden")))
#else
#define SDK_EXPORT
#define SDK_IMPORT
#define SDK_LOCAL
#endif
#endif

#ifdef SDK_BUILD
#define SDK_API SDK_EXPORT
#else
#define SDK_API SDK_IMPORT
#endif

#ifdef __cplusplus
#define SDK_C_API extern "C" SDK_API
#else
#define SDK_C_API SDK_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SDK_CONTEXT_ABI_VERSION_1 1U
#define SDK_ABI_MAGIC 0x484A5344 // 'HJSD' in ASCII

/**
 * @brief Common ABI prefix for all user-provided parameter structures.
 *        MUST be placed as the FIRST member of any binary-extensible struct!
 *
 * @abi_contract PLATFORM-NATIVE ABI SPECIFICATION
 *               - This header and its associated structures define an in-memory,
 *                 platform-native C ABI.
 *               - Binary layout compatibility (offsets, sizes, padding) is guaranteed ONLY
 *                 within the SAME target architecture, word size (e.g., x64 vs x86), 
 *                 and compiler alignment model.
 *               - This is NOT a universal/cross-platform wire format or serialization ABI.
 *               - Both caller and implementation MUST be compiled for the same platform target.
 * *
 * @abi_type_safety STRICT FIXED-WIDTH TYPE MANDATE
 *               - Non-fixed width types (`size_t`, `long`, `int`, `bool`, `enum`) are 
 *                 STRICTLY FORBIDDEN inside SDK ABI structures.
 *               - DANGERS OF NON-FIXED WIDTH TYPES:
 *                 1) `size_t` / `uintptr_t`: Varies between 32-bit (4 bytes) and 64-bit (8 bytes),
 *                    causing instant struct layout mismatch across x86/x64 boundaries.
 *                 2) `long`: Suffers from data model fragmentation — 4 bytes under Windows x64 (LLP64),
 *                    but 8 bytes under Linux/macOS x64 (LP64).
 *                 3) `int`: Platform/Architecture dependent (16-bit vs 32-bit).
 *                 4) `bool`: Width is unstandardized by C/C++ specs (varies between 1, 2, or 4 bytes).
 *                 5) `enum`: Size can change based on compiler flags (e.g., `-fshort-enums`).
 *               - REMEDY: ALWAYS use explicit fixed-width types (`uint32_t`, `int32_t`, `uint64_t`).
 */
#define SDK_ABI_HEADER                                                         \
    uint32_t abi_magic;                                                        \
    uint32_t abi_size;                                                         \
    uint32_t abi_version;

typedef struct sdk_abi_header
{
    SDK_ABI_HEADER
} sdk_abi_header_t;

/**
 * @brief SDK callback function signature.
 *
 * @param user_data Opaque borrowed pointer passed back to the caller.
 *
 * @thread_context     Callback MAY be invoked from internal SDK worker, network, or I/O threads.
 *                     The execution thread is NOT guaranteed to be the calling thread.
 *
 * @thread_safety      Callback implementations MUST be thread-safe, reentrant, and non-blocking.
 *                     Long-running or blocking operations inside the callback WILL starve 
 *                     SDK internal worker threads.
 *
 * @reentrancy         Callback implementations MUST NOT synchronously invoke any SDK APIs 
 *                     using the current context unless explicitly documented as thread-safe,
 *                     to avoid potential internal recursive deadlocks.
 *
 * @concurrency        For a given context instance, callback invocations are NOT guaranteed 
 *                     to be serialized. Multiple callbacks MAY run concurrently on different threads.
 *
 * @ordering           Callbacks MAY be delivered out-of-order across thread boundaries.
 *
 * @exception_safety   STRICT ABI RULE: Callbacks MUST NOT allow C++ exceptions to propagate 
 *                     across the C ABI boundary. Any C++ exception MUST be caught and handled 
 *                     internally within the callback (e.g., using `try { ... } catch (...) {}`).
 *                     Uncaught exceptions crossing the ABI boundary result in UNDEFINED BEHAVIOR 
 *                     or immediate process termination via `std::terminate()`.
 *
 * @lifetime           1) The memory hosting the callback function code MUST remain loaded 
 *                        in executable memory until task completion / SDK shutdown.
 *                     2) Unloading dynamic libraries (e.g., `FreeLibrary` / `dlclose`) that contain 
 *                        active callback code will cause immediate crashes.
 *
 * @cancellation       Callbacks CANNOT be canceled mid-execution. Once enqueued, the callback 
 *                     WILL be executed unless the SDK is explicitly shut down prior to queuing.
 *
 * @post_shutdown      NO callbacks will be dispatched after SDK shutdown / destruction completes.
 */
typedef void (*sdk_callback_t)(void *user_data);

/**
 * @brief SDK operational context structure.
 */
typedef struct sdk_context
{
    SDK_ABI_HEADER

    /**
     * @brief User-provided context pointer forwarded to callback functions.
     *
     * @ownership Borrowed pointer.
     *            - SDK NEVER assumes ownership of `user_data`.
     *            - SDK NEVER allocates nor frees `user_data`.
     *            - Caller retains full ownership and responsibility for its memory lifetime.
     *
     * @lifetime  The object referenced by `user_data` MUST outlive all asynchronous execution 
     *            and remaining callback invocations.
     *
     * @warning   DO NOT pass pointers to automatic/stack-allocated variables if the SDK API 
     *            executes asynchronously, as returning from the calling scope will cause 
     *            a Use-After-Free (UAF) during callback execution.
     */
    void *user_data;

    /**
     * @brief Callback function pointer.
     * @lifetime The callback function code MUST remain loaded in executable memory 
     *           until processing is complete.
     */
    sdk_callback_t callback;
} sdk_context_t;

/**
 * @brief Generic function pointer definition for SDK API invocations.
 *
 * @note  This is a low-level ABI type definition used internally for function dispatching,
 *        dynamic symbol resolution (e.g., via `dlsym` / `GetProcAddress`), or virtual API tables.
 *        It is NOT a concrete exported library function.
 *
 * @details Concrete C-style exported SDK functions are defined explicitly across library headers.
 *          Example concrete API signature:
 *          @code
 *          SDK_C_STYLE_EXPORT void lic_init(sdk_context_t *ctx);
 *          @endcode
 */
typedef void (*sdk_api_t)(sdk_context_t *);

#if defined(__cplusplus) && __cplusplus >= 201103L
#define SDK_ASSERT_NOT_SIZE_T(type, field)                                     \
    static_assert(                                                             \
        !std::is_same<decltype(((type *) 0)->field), size_t>::value,           \
        "ABI Violation: 'size_t' is forbidden in SDK ABI structures due to "   \
        "x86/x64 layout mismatch. Use 'uint32_t' or 'uint64_t'!")
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define SDK_ASSERT_NOT_SIZE_T(type, field)                                     \
    _Static_assert(!_Generic((((type *) 0)->field), size_t: 1, default: 0),    \
                   "ABI Violation: 'size_t' is forbidden in SDK ABI "          \
                   "structures! Use 'uint32_t' or 'uint64_t'!")
#else
#define SDK_ASSERT_NOT_SIZE_T(type, field) ((void) 0)
#endif

#define SDK_CONTEXT(ctx)                                                       \
    sdk_context_t ctx;                                                         \
    SDK_INIT_ABI_HEADER(&(ctx), sdk_context_t, SDK_CONTEXT_ABI_VERSION_1)

#define SDK_INIT_ABI_HEADER(var_ptr, type_name, ver)                           \
    do                                                                         \
    {                                                                          \
        memset((var_ptr), 0, sizeof(type_name));                               \
        ((sdk_abi_header_t *) (var_ptr))->abi_magic = SDK_ABI_MAGIC;           \
        ((sdk_abi_header_t *) (var_ptr))->abi_size =                           \
            (uint32_t) sizeof(type_name);                                      \
        ((sdk_abi_header_t *) (var_ptr))->abi_version = (ver);                 \
    } while(0)

#define SDK_HAS_ABI_HEADER(ptr)                                                \
    ((ptr) != NULL                                                             \
     && (((const sdk_abi_header_t *) (ptr))->abi_magic == SDK_ABI_MAGIC)       \
     && (((const sdk_abi_header_t *) (ptr))->abi_size                          \
         >= sizeof(sdk_abi_header_t)))

#define SDK_HAS_FIELD(ptr, type, field)                                        \
    (SDK_ASSERT_NOT_SIZE_T(type, field),                                       \
     SDK_HAS_ABI_HEADER(ptr)                                                   \
         && (((const type *) (ptr))->abi_size <= sizeof(type))                 \
         && (((const type *) (ptr))->abi_size                                  \
             >= (offsetof(type, field) + sizeof(((type *) 0)->field))))

#define SDK_VALIDATE_ABI(ptr, type, max_ver)                                   \
    (SDK_HAS_ABI_HEADER(ptr)                                                   \
     && (((const type *) (ptr))->abi_size >= sizeof(sdk_abi_header_t))         \
     && (((const type *) (ptr))->abi_size <= sizeof(type))                     \
     && (((const type *) (ptr))->abi_version <= (max_ver)))

#ifdef __cplusplus
}
#endif

#endif // SDK_H