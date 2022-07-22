/* Unicode Algorithms Implementation by Marl Gigical.
 * License: Public Domain or MIT - sign whatever you want.
 * See notice at the end of this file. */

#pragma once

#include <uni/impl/iterator.h>
#include <uni/internal/defines.h>
#include <uni/internal/stages.h>

#ifndef UNI_ALGO_STATIC_DATA
    #include <uni/impl/break_word_data_extern.h>
#endif

namespace uni::detail {

enum prop_wb : type_codept {
    double_quote = 1,
    single_quote,
    hebrew_letter,
    cr,
    lf,
    newline,
    extend,
    regional_indicator,
    format,
    katakana,
    a_letter,
    mid_letter,
    mid_num,
    mid_num_let,
    numeric,
    extend_num_let,
    zwj,
    w_seg_space,
    extended_pictographic,
    remaining_alphabetic,
};

inline bool operator==(const prop_wb& lhs, const type_codept& rhs) {
    return static_cast<type_codept>(lhs) == rhs;
}
inline bool operator==(const type_codept& lhs, const prop_wb& rhs) {
    return lhs == static_cast<type_codept>(rhs);
}

enum class break_word_state {
    begin,
    process,
    ep,
    ep_zwj,
    ri,
    ri_ri,
};

inline type_codept stages_break_word_prop(type_codept c) {
    return stages(c, stage1_break_word, stage2_break_word);
}

struct impl_break_word_state {
    type_codept prev_cp = 0;
    type_codept prev_cp_prop = 0;
    break_word_state state = break_word_state::begin;

    type_codept prev_cp1 = 0;
    type_codept prev_cp1_prop = 0;

    type_codept prev_cp2 = 0;
    type_codept prev_cp2_prop = 0;
};

inline void impl_break_word_state_reset(struct impl_break_word_state* state) {
    state->prev_cp = 0;
    state->prev_cp_prop = 0;
    state->state = break_word_state::begin;

    state->prev_cp1 = 0;
    state->prev_cp1_prop = 0;

    state->prev_cp2 = 0;
    state->prev_cp2_prop = 0;
}

inline bool impl_break_is_word(type_codept word_prop) {
    switch (word_prop) {
        case prop_wb::a_letter:
        case prop_wb::hebrew_letter:
        case prop_wb::numeric:
        case prop_wb::katakana:
        case prop_wb::remaining_alphabetic:
            return true;
        default:
            return false;
    }
}

inline bool break_word_skip(type_codept prop) {
    switch (prop) {
        case prop_wb::zwj:
        case prop_wb::extend:
        case prop_wb::format:
            return true;
        default:
            return false;
    }
}

template<typename it_in_utf8, typename it_end_utf8>
type_codept utf8_break_word_skip(it_in_utf8 first, it_end_utf8 last) {
    /* C++ Note: this function makes the word segmentation algorithm incompatible with input iterators.
     * This means Word Boundary Rules are not compatible with streams at all.
     * It can even look like it works but it never pass tests because we must go back after this function.
     * The test line where it fails: "\x0061\x003A\x0001" - AHLetter + MidLetter + Other.
     * Must be 3 words but if we don't go back it will be 2. */

    // Make it compile only for contiguous or random access iterator for now
    it_in_utf8 src = first + 0;
    type_codept c = 0;

    while (src != last) {
        src = utf8_iter(src, last, &c, iter_replacement);

        type_codept prop = stages_break_word_prop(c);

        if (break_word_skip(prop))
            continue;

        return prop;
    }
    return 0;
}

template<typename it_in_utf8, typename it_end_utf8>
inline bool utf8_break_word(struct impl_break_word_state* state, type_codept c,
                            type_codept* word_prop, it_in_utf8 first,
                            it_end_utf8 last) {
    // word_prop property must be used only with impl_break_is_word

    // TODO: https://unicode.org/reports/tr29/#State_Machines
    // ftp://ftp.unicode.org/Public/UNIDATA/auxiliary/WordBreakTest.html
    // Note that only WB5, WB7a, WB8, WB9, WB10, WB13 rules can be handled
    // with the state table so it's not even worth it probably.
    // Compared the performance with ICU it's already much faster so it can wait.

    type_codept c_prop = stages_break_word_prop(c);
    type_codept p_prop = state->prev_cp_prop;

    // Previous values of code points with WB4 rules
    type_codept p1_prop = state->prev_cp1_prop;
    type_codept p2_prop = state->prev_cp2_prop;

    type_codept t_prop = 0;

    bool result = false;

    // https://www.unicode.org/reports/tr29/#Word_Boundary_Rules
    // Unicode 11.0 - 14.0 rules

    if (state->state == break_word_state::begin) {
        state->state = break_word_state::process;
        *word_prop = c_prop;
        // TODO: fix word_prop
        //*word_prop = 0;
    } else if (p_prop == prop_wb::cr && c_prop == prop_wb::lf)  // WB3
        result = false;  // NOLINT
    else if (p_prop == prop_wb::newline || p_prop == prop_wb::cr
             || p_prop == prop_wb::lf)  // WB3a
        result = true;  // NOLINT
    else if (c_prop == prop_wb::newline || c_prop == prop_wb::cr
             || c_prop == prop_wb::lf)  // WB3b
        result = true;  // NOLINT
    else if (p_prop == prop_wb::zwj
             && c_prop == prop_wb::extended_pictographic)  // WB3c
        result = false;  // NOLINT
    else if (p_prop == prop_wb::w_seg_space
             && c_prop == prop_wb::w_seg_space)  // WB3d
        result = false;  // NOLINT
    else if (break_word_skip(c_prop))  // WB4
    {
        // Just skip forward without touching previous values of code points with WB4 rules
        state->prev_cp = c;
        state->prev_cp_prop = c_prop;
        return false;
    }

    // p and p_prop must not be used anymore because WB4 takes effect below this line

    else if ((p1_prop == prop_wb::a_letter || p1_prop == prop_wb::hebrew_letter)
             && (c_prop == prop_wb::a_letter
                 || c_prop == prop_wb::hebrew_letter))  // WB5
        result = false;  // NOLINT
    else if ((p1_prop == prop_wb::a_letter || p1_prop == prop_wb::hebrew_letter)
             && (c_prop == prop_wb::mid_letter || c_prop == prop_wb::mid_num_let
                 || c_prop == prop_wb::single_quote)
             && ((t_prop = utf8_break_word_skip(first, last)) != 0
                 && (t_prop == prop_wb::a_letter
                     || t_prop == prop_wb::hebrew_letter)))  // WB6
        result = false;  // NOLINT
    else if ((p2_prop == prop_wb::a_letter || p2_prop == prop_wb::hebrew_letter)
             && (p1_prop == prop_wb::mid_letter
                 || p1_prop == prop_wb::mid_num_let
                 || p1_prop == prop_wb::single_quote)
             && (c_prop == prop_wb::a_letter
                 || c_prop == prop_wb::hebrew_letter))  // WB7
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::hebrew_letter
             && c_prop == prop_wb::single_quote)  // WB7a
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::hebrew_letter
             && c_prop == prop_wb::double_quote
             && utf8_break_word_skip(first, last)
                 == prop_wb::hebrew_letter)  // WB7b
        result = false;  // NOLINT
    else if (p2_prop == prop_wb::hebrew_letter
             && p1_prop == prop_wb::double_quote
             && c_prop == prop_wb::hebrew_letter)  // WB7c
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::numeric && c_prop == prop_wb::numeric)  // WB8
        result = false;  // NOLINT
    else if ((p1_prop == prop_wb::a_letter || p1_prop == prop_wb::hebrew_letter)
             && c_prop == prop_wb::numeric)  // WB9
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::numeric
             && (c_prop == prop_wb::a_letter
                 || c_prop == prop_wb::hebrew_letter))  // WB10
        result = false;  // NOLINT
    else if (p2_prop == prop_wb::numeric
             && (p1_prop == prop_wb::mid_num || p1_prop == prop_wb::mid_num_let
                 || p1_prop == prop_wb::single_quote)
             && c_prop == prop_wb::numeric)  // WB11
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::numeric
             && (c_prop == prop_wb::mid_num || c_prop == prop_wb::mid_num_let
                 || c_prop == prop_wb::single_quote)
             && utf8_break_word_skip(first, last) == prop_wb::numeric)  // WB12
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::katakana
             && c_prop == prop_wb::katakana)  // WB13
        result = false;  // NOLINT
    else if (((p1_prop == prop_wb::a_letter
               || p1_prop == prop_wb::hebrew_letter)
              || p1_prop == prop_wb::numeric || p1_prop == prop_wb::katakana
              || p1_prop == prop_wb::extend_num_let)
             && c_prop == prop_wb::extend_num_let)  // WB13a
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::extend_num_let
             && ((c_prop == prop_wb::a_letter
                  || c_prop == prop_wb::hebrew_letter)
                 || c_prop == prop_wb::numeric
                 || c_prop == prop_wb::katakana))  // WB13b
        result = false;  // NOLINT
    else if (state->state == break_word_state::ri
             && c_prop == prop_wb::regional_indicator)  // WB15/WB16
        result = false;  // NOLINT
    else  // WB999
    {
        result = true;
        *word_prop = c_prop;
        // TODO: fix word_prop
        //*word_prop = 0;
    }

    // WB15/WB16
    if (c_prop == prop_wb::regional_indicator) {
        if (state->state == break_word_state::ri)
            state->state = break_word_state::ri_ri;
        else
            state->state = break_word_state::ri;
    } else
        state->state = break_word_state::process;

    // Set previous values of codepoints with WB4 rules
    state->prev_cp2 = state->prev_cp1;
    state->prev_cp2_prop = state->prev_cp1_prop;
    state->prev_cp1 = c;
    state->prev_cp1_prop = c_prop;
    state->prev_cp = c;
    state->prev_cp_prop = c_prop;

    // TODO: The logic of word_prop is incorrect right now, to make it work properly rearrage all word properties to:
    // all punctuations > prop_wb::numeric > prop_WB_Hebrew_Letter > prop_wb::a_letter > prop_wb::katakana > prop_wb::remaining_alphabetic
    // then document why such arrangement is required and and uncomment all todos: fix word_prop
    // TODO: fix word_prop
    //if (c_proc > *word_prop)
    //	*word_prop = c_prop;

    return result;
}

template<typename it_in_utf8, typename it_end_utf8>
bool impl_utf8_break_word(struct impl_break_word_state* state, type_codept c,
                          type_codept* word_prop, it_in_utf8 first,
                          it_end_utf8 last) {
    return utf8_break_word(state, c, word_prop, first, last);
}

template<typename it_in_utf8, typename it_end_utf8>
inline bool inline_utf8_break_word(struct impl_break_word_state* state,
                                   type_codept c, type_codept* word_prop,
                                   it_in_utf8 first, it_end_utf8 last) {
    return utf8_break_word(state, c, word_prop, first, last);
}

// BEGIN: GENERATED UTF-16 FUNCTIONS
#ifndef UNI_ALGO_DOC_GENERATED_UTF16

template<typename it_in_utf16, typename it_end_utf16>
type_codept utf16_break_word_skip(it_in_utf16 first, it_end_utf16 last) {
    it_in_utf16 src = first;
    type_codept c = 0;

    while (src != last) {
        src = utf16_iter(src, last, &c, iter_replacement);

        type_codept prop = stages_break_word_prop(c);

        if (break_word_skip(prop))
            continue;

        return prop;
    }
    return 0;
}

template<typename it_in_utf16, typename it_end_utf16>
inline bool utf16_break_word(struct impl_break_word_state* state, type_codept c,
                             type_codept* word_prop, it_in_utf16 first,
                             it_end_utf16 last) {
    // word_prop property must be used only with impl_break_is_word

    // TODO: https://unicode.org/reports/tr29/#State_Machines
    // ftp://ftp.unicode.org/Public/UNIDATA/auxiliary/WordBreakTest.html
    // I compared the performance with ICU it's already much faster so it can wait.

    type_codept c_prop = stages_break_word_prop(c);
    type_codept p_prop = state->prev_cp_prop;

    // Previous values of code points with WB4 rules
    type_codept p1_prop = state->prev_cp1_prop;
    type_codept p2_prop = state->prev_cp2_prop;

    type_codept t_prop = 0;

    bool result = false;

    // https://www.unicode.org/reports/tr29/#Word_Boundary_Rules
    // Unicode 11.0 - 14.0 rules

    if (state->state == break_word_state::begin) {
        state->state = break_word_state::process;
        *word_prop = c_prop;
        // TODO: fix word_prop
        //*word_prop = 0;
    } else if (p_prop == prop_wb::cr && c_prop == prop_wb::lf)  // WB3
        result = false;  // NOLINT
    else if (p_prop == prop_wb::newline || p_prop == prop_wb::cr
             || p_prop == prop_wb::lf)  // WB3a
        result = true;  // NOLINT
    else if (c_prop == prop_wb::newline || c_prop == prop_wb::cr
             || c_prop == prop_wb::lf)  // WB3b
        result = true;  // NOLINT
    else if (p_prop == prop_wb::zwj
             && c_prop == prop_wb::extended_pictographic)  // WB3c
        result = false;  // NOLINT
    else if (p_prop == prop_wb::w_seg_space
             && c_prop == prop_wb::w_seg_space)  // WB3d
        result = false;  // NOLINT
    else if (break_word_skip(c_prop))  // WB4
    {
        // Just skip forward without touching previous values of code points with WB4 rules
        state->prev_cp = c;
        state->prev_cp_prop = c_prop;
        return false;
    }

    // p and p_prop must not be used anymore because WB4 takes effect below this line

    else if ((p1_prop == prop_wb::a_letter || p1_prop == prop_wb::hebrew_letter)
             && (c_prop == prop_wb::a_letter
                 || c_prop == prop_wb::hebrew_letter))  // WB5
        result = false;  // NOLINT
    else if ((p1_prop == prop_wb::a_letter || p1_prop == prop_wb::hebrew_letter)
             && (c_prop == prop_wb::mid_letter || c_prop == prop_wb::mid_num_let
                 || c_prop == prop_wb::single_quote)
             && ((t_prop = utf16_break_word_skip(first, last)) != 0
                 && (t_prop == prop_wb::a_letter
                     || t_prop == prop_wb::hebrew_letter)))  // WB6
        result = false;  // NOLINT
    else if ((p2_prop == prop_wb::a_letter || p2_prop == prop_wb::hebrew_letter)
             && (p1_prop == prop_wb::mid_letter
                 || p1_prop == prop_wb::mid_num_let
                 || p1_prop == prop_wb::single_quote)
             && (c_prop == prop_wb::a_letter
                 || c_prop == prop_wb::hebrew_letter))  // WB7
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::hebrew_letter
             && c_prop == prop_wb::single_quote)  // WB7a
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::hebrew_letter
             && c_prop == prop_wb::double_quote
             && utf16_break_word_skip(first, last)
                 == prop_wb::hebrew_letter)  // WB7b
        result = false;  // NOLINT
    else if (p2_prop == prop_wb::hebrew_letter
             && p1_prop == prop_wb::double_quote
             && c_prop == prop_wb::hebrew_letter)  // WB7c
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::numeric && c_prop == prop_wb::numeric)  // WB8
        result = false;  // NOLINT
    else if ((p1_prop == prop_wb::a_letter || p1_prop == prop_wb::hebrew_letter)
             && c_prop == prop_wb::numeric)  // WB9
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::numeric
             && (c_prop == prop_wb::a_letter
                 || c_prop == prop_wb::hebrew_letter))  // WB10
        result = false;  // NOLINT
    else if (p2_prop == prop_wb::numeric
             && (p1_prop == prop_wb::mid_num || p1_prop == prop_wb::mid_num_let
                 || p1_prop == prop_wb::single_quote)
             && c_prop == prop_wb::numeric)  // WB11
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::numeric
             && (c_prop == prop_wb::mid_num || c_prop == prop_wb::mid_num_let
                 || c_prop == prop_wb::single_quote)
             && utf16_break_word_skip(first, last) == prop_wb::numeric)  // WB12
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::katakana
             && c_prop == prop_wb::katakana)  // WB13
        result = false;  // NOLINT
    else if (((p1_prop == prop_wb::a_letter
               || p1_prop == prop_wb::hebrew_letter)
              || p1_prop == prop_wb::numeric || p1_prop == prop_wb::katakana
              || p1_prop == prop_wb::extend_num_let)
             && c_prop == prop_wb::extend_num_let)  // WB13a
        result = false;  // NOLINT
    else if (p1_prop == prop_wb::extend_num_let
             && ((c_prop == prop_wb::a_letter
                  || c_prop == prop_wb::hebrew_letter)
                 || c_prop == prop_wb::numeric
                 || c_prop == prop_wb::katakana))  // WB13b
        result = false;  // NOLINT
    else if (state->state == break_word_state::ri
             && c_prop == prop_wb::regional_indicator)  // WB15/WB16
        result = false;  // NOLINT
    else  // WB999
    {
        result = true;
        *word_prop = c_prop;
        // TODO: fix word_prop
        //*word_prop = 0;
    }

    // WB15/WB16
    if (c_prop == prop_wb::regional_indicator) {
        if (state->state == break_word_state::ri)
            state->state = break_word_state::ri_ri;
        else
            state->state = break_word_state::ri;
    } else
        state->state = break_word_state::process;

    // Set previous values of codepoints with WB4 rules
    state->prev_cp2 = state->prev_cp1;
    state->prev_cp2_prop = state->prev_cp1_prop;
    state->prev_cp1 = c;
    state->prev_cp1_prop = c_prop;
    state->prev_cp = c;
    state->prev_cp_prop = c_prop;

    // TODO: fix word_prop
    //if (c_proc > *word_prop)
    //	*word_prop = c_prop;

    return result;
}

template<typename it_in_utf16, typename it_end_utf16>
bool impl_utf16_break_word(struct impl_break_word_state* state, type_codept c,
                           type_codept* word_prop, it_in_utf16 first,
                           it_end_utf16 last) {
    return utf16_break_word(state, c, word_prop, first, last);
}

template<typename it_in_utf16, typename it_end_utf16>
inline bool inline_utf16_break_word(struct impl_break_word_state* state,
                                    type_codept c, type_codept* word_prop,
                                    it_in_utf16 first, it_end_utf16 last) {
    return utf16_break_word(state, c, word_prop, first, last);
}

#endif  // UNI_ALGO_DOC_GENERATED_UTF16
// END: GENERATED UTF-16 FUNCTIONS

}  // namespace uni::detail

#include <uni/internal/undefs.h>

/* Public Domain Contract
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
 * software, either in source code form or as a compiled binary, for any purpose,
 * commercial or non-commercial, and by any means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors of this
 * software dedicate any and all copyright interest in the software to the public
 * domain. We make this dedication for the benefit of the public at large and to
 * the detriment of our heirs and successors. We intend this dedication to be an
 * overt act of relinquishment in perpetuity of all present and future rights to
 * this software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE  SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * MIT Contract
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO  THE WARRANTIES OF  MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  TORT OR OTHERWISE, ARISING FROM,
 * OUT OF  OR  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
