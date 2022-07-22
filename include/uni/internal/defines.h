/* Unicode Algorithms Implementation by Marl Gigical.
 * License: Public Domain or MIT - sign whatever you want.
 * See LICENSE.txt */

// Warning! This file must never be used by a wrapper.

// Internal defines, must be used together with impl_undefs.h

// The internal defines are lowercased because they make too much noise in the code if uppercased.

// FIX: delete this
#define uaix_always_inline inline

// Must not be used without performance testing it can degrade performance in many cases
#if defined(__GNUC__) || defined(__clang__)
    #define uaix_likely(x) __builtin_expect(!!(x), 1)
    #define uaix_unlikely(x) __builtin_expect(!!(x), 0)
#else
    #define uaix_likely(x) (!!(x))
    #define uaix_unlikely(x) (!!(x))
#endif

// FIX: delete below
#define uaix_static
#define uaix_inline inline
#define uaix_const inline constexpr

// The define can be used to switch Unicode data to static (constexpr in C++) mode
// uaix_const_data is only used by generated Unicode data files
#ifdef UNI_ALGO_STATIC_DATA
    #define uaix_const_data uaix_const
#else
    #define uaix_const_data const
#endif

// FIX: delete below
#ifdef __cplusplus
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wsign-conversion"
        #pragma GCC diagnostic ignored "-Wold-style-cast"
    #endif
#endif
