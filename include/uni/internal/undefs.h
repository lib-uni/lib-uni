/* Unicode Algorithms Implementation by Marl Gigical.
 * License: Public Domain or MIT - sign whatever you want.
 * See LICENSE.txt */

// Warning! This file must never be used by a wrapper.

// Internal undefs, must be used together with impl_defines.h

#undef uaix_likely
#undef uaix_unlikely

#ifdef __cplusplus
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic pop
    #endif
#endif
