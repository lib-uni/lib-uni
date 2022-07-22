/* Unicode Algorithms Implementation by Marl Gigical.
 * License: Public Domain or MIT - sign whatever you want.
 * See LICENSE.txt */

// Must be included to define and check internal types in C wrapper.
// Must never be included by a C++ wrapper.

/* This file is not used in C++ wrapper but it's here for a reference.
 * In C++ the types must be defined by a wrapper.
 * Note that all the types must be unsigned in C++ because the implementation
 * knows nothing about iterator types and signed casts might alter values.
 * Note that in C++ the types (except type_codept) are used only to
 * supress warnings by casts but the casts still can alter values.
 * See: test/test_extra.h -> test_alter_value()
 * This means the types must be checked with static asserts like this: */
#if 0
    #ifdef __cplusplus
namespace uni { namespace detail {
using type_codept = char32_t;
using type_char8  = unsigned char;
using type_char16 = char16_t;
using type_char32 = char32_t;
const size_t impl_npos = static_cast<size_t>(-1); // the same as const size_t impl_npos = std::numeric_limits<size_t>::max();
const std::nullptr_t impl_nullptr = nullptr;
static_assert(std::is_unsigned<type_codept>::value && sizeof(type_codept) >= sizeof(char32_t));
static_assert(std::is_unsigned<type_char8>::value);
static_assert(std::is_unsigned<type_char16>::value && sizeof(type_char16) >= sizeof(char16_t));
static_assert(std::is_unsigned<type_char32>::value && sizeof(type_char32) >= sizeof(char32_t));
// This is almost the same as this in C:
//typedef unsigned int type_codept
//typedef unsigned char type_char8
//typedef unsigned short type_char16
//typedef unsigned int type_char32
//static const size_t impl_npos = (size_t)-1;
//static void* impl_nullptr = (void*)0;
//typedef int static_assert_char8[(type_char8)-1 > 0 ? 1 : -1];
//typedef int static_assert_char16[sizeof(type_char16) >= sizeof(short) && (type_codept)-1 > 0 ? 1 : -1];
//typedef int static_assert_char32[sizeof(type_char32) >= sizeof(short) * 2 && (type_codept)-1 > 0 ? 1 : -1];
//typedef int static_assert_codept[sizeof(type_codept) >= sizeof(short) * 2 && (type_codept)-1 > 0 ? 1 : -1];
}} // namespace uni::detail
    #endif
#endif  // 0
