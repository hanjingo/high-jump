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

#include <stdint.h>

#if defined(_MSC_VER)
#define SDK_EXPORT __declspec(dllexport)
#define SDK_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define SDK_EXPORT __attribute__((visibility("default")))
#define SDK_IMPORT
#else
#define SDK_EXPORT
#define SDK_IMPORT
#endif

#ifdef __cplusplus
#define SDK_C_STYLE_EXPORT extern "C" SDK_EXPORT
#define SDK_C_STYLE_IMPORT extern "C" SDK_EXPORT
#else
#define SDK_C_STYLE_EXPORT SDK_EXPORT
#define SDK_C_STYLE_IMPORT SDK_EXPORT
#endif

#ifdef SDK_BUILD
#define SDK_API SDK_C_STYLE_EXPORT
#else
#define SDK_API SDK_C_STYLE_IMPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Current ABI version for `sdk_context_t`.
 */
#define SDK_CONTEXT_VERSION_1 1U

/**
 * @brief SDK callback function signature.
 *
 * @param user_data Opaque borrowed pointer passed back to the caller.
 *
 * @thread_safety Callback MAY be invoked from internal SDK worker/I-O threads.
 *                The implementation MUST be thread-safe and non-blocking.
 * @reentrancy    Callback implementations MUST NOT invoke SDK APIs synchronously 
 *                unless explicitly stated otherwise to avoid potential deadlocks.
 */
typedef void (*sdk_callback_t)(void *user_data);

/**
 * @brief SDK operational context structure.
 */
typedef struct sdk_context
{
    /**
     * @brief Size of this structure in bytes for memory boundary validation.
     * @contract The caller MUST set this field to `sizeof(sdk_context_t)` 
     *           prior to passing the structure to any SDK functions.
     */
    uint32_t size;

    /**
     * @brief ABI version indicator for logical layout and semantic validation.
     * @contract The caller MUST set this field to the appropriate version macro 
     *           (e.g., `SDK_CONTEXT_VERSION_1`). The SDK uses this to branch logic 
     *           for legacy vs modern API execution.
     */
    uint32_t version;

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

#ifdef __cplusplus
}
#endif

#endif // SDK_H