/* Unicode Algorithms Implementation by Marl Gigical.
 * License: Public Domain or MIT - sign whatever you want.
 * See LICENSE.txt */

// Warning! This file must never be used by a wrapper.

// Internal defines, must be used together with impl_undefs.h

// The internal defines are lowercased because they make too much noise in the code if uppercased.

// Must not be used without performance testing it can degrade performance in many cases
#if defined(__GNUC__) || defined(__clang__)
    #define uaix_likely(x) __builtin_expect(!!(x), 1)
    #define uaix_unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define uaix_likely(x) (!!(x))
    #define uaix_unlikely(x) (!!(x))
#endif


// FIX: delete below
#ifdef __cplusplus
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wsign-conversion"
        #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
#endif
