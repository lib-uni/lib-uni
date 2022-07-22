/* Unicode Algorithms Implementation by Marl Gigical.
 * License: Public Domain or MIT - sign whatever you want.
 * See notice at the end of this file. */

#pragma once

#include <uni/impl/iterator.h>
#include <uni/internal/defines.h>
#include <uni/internal/stages.h>

#ifndef UNI_ALGO_STATIC_DATA
    #include <uni/impl/break_grapheme_data_extern.h>
#endif

namespace uni::detail {

enum class prop_gb : type_codept {
    prepend = 1,
    cr,
    lf,
    control,
    extend,
    regional_indicator,
    spacing_mark,
    l,
    v,
    t,
    lv,
    lvt,
    zwj,
    extended_pictographic,
};

bool operator==(const prop_gb& lhs, const type_codept& rhs) {
    return static_cast<type_codept>(lhs) == rhs;
}
bool operator==(const type_codept& lhs, const prop_gb& rhs) {
    return lhs == static_cast<type_codept>(rhs);
}

enum class break_grapheme_state {
    begin,
    process,
    ep,
    ep_zwj,
    ri,
    ri_ri,
};

inline type_codept stages_break_grapheme_prop(type_codept c) {
    return stages(c, stage1_break_grapheme, stage2_break_grapheme);
}

struct impl_break_grapheme_state {
    type_codept prev_cp = 0;
    type_codept prev_cp_prop = 0;
    break_grapheme_state state = break_grapheme_state::begin;
};

inline void
impl_break_grapheme_state_reset(struct impl_break_grapheme_state* state) {
    state->prev_cp = 0;
    state->prev_cp_prop = 0;
    state->state = break_grapheme_state::begin;
}
/*
// TODO: see TODO below.
// Extend_ExtCccZwj and ZWJ_ExtCccZwj should not be used.
// ZWJ must be the same as Extend.
const bool break_table_grapheme[15][15] =
{
//   Oth CR LF Con Ext RI Pre SpM L  V  T  LV LVT EP ZWJ
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 1, 1, 1,  1, 0}, // Other
    {1,  1, 0, 1,  1,  1, 1,  1,  1, 1, 1, 1, 1,  1, 1}, // CR
    {1,  1, 1, 1,  1,  1, 1,  1,  1, 1, 1, 1, 1,  1, 1}, // LF
    {1,  1, 1, 1,  1,  1, 1,  1,  1, 1, 1, 1, 1,  1, 1}, // Control
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 1, 1, 1,  1, 0}, // Extend
    {1,  1, 1, 1,  0,  0, 1,  0,  1, 1, 1, 1, 1,  1, 0}, // RI
    {0,  1, 1, 1,  0,  0, 0,  0,  0, 0, 0, 0, 0,  0, 0}, // Prepend
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 1, 1, 1,  1, 0}, // SpacingMark
    {1,  1, 1, 1,  0,  1, 1,  0,  0, 0, 1, 0, 0,  1, 0}, // L
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 0, 0, 1, 1,  1, 0}, // V
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 0, 1, 1,  1, 0}, // T
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 0, 0, 1, 1,  1, 0}, // LV
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 0, 1, 1,  1, 0}, // LVT
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 1, 1, 1,  1, 0}, // ExtPict
    {1,  1, 1, 1,  0,  1, 1,  0,  1, 1, 1, 1, 1,  1, 0}, // ZWJ
};
*/
inline bool break_grapheme(struct impl_break_grapheme_state* state,
                           type_codept c) {
    // TODO: https://unicode.org/reports/tr29/#State_Machines
    // ftp://ftp.unicode.org/Public/UNIDATA/auxiliary/GraphemeBreakTest.html
    // See state table above.
    // Compared the performance with ICU it's already much faster so it can wait.

    type_codept c_prop = stages_break_grapheme_prop(c);
    type_codept p_prop = state->prev_cp_prop;

    bool result = false;

    // https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundary_Rules
    // Unicode 11.0 - 14.0 rules

    if (state->state == break_grapheme_state::begin)
        state->state = break_grapheme_state::process;
    else if (p_prop == prop_gb::cr && c_prop == prop_gb::lf)  // GB3
        result = false;  // NOLINT
    else if (p_prop == prop_gb::control || p_prop == prop_gb::cr
             || p_prop == prop_gb::lf)  // GB4
        result = true;  // NOLINT
    else if (c_prop == prop_gb::control || c_prop == prop_gb::cr
             || c_prop == prop_gb::lf)  // GB5
        result = true;  // NOLINT
    else if (p_prop == prop_gb::l
             && (c_prop == prop_gb::l || c_prop == prop_gb::v
                 || c_prop == prop_gb::lv || c_prop == prop_gb::lvt))  // GB6
        result = false;  // NOLINT
    else if ((p_prop == prop_gb::lv || p_prop == prop_gb::v)
             && (c_prop == prop_gb::v || c_prop == prop_gb::t))  // GB7
        result = false;  // NOLINT
    else if ((p_prop == prop_gb::lvt || p_prop == prop_gb::t)
             && c_prop == prop_gb::t)  // GB8
        result = false;  // NOLINT
    else if (c_prop == prop_gb::extend || c_prop == prop_gb::zwj)  // GB9
        result = false;  // NOLINT
    else if (c_prop == prop_gb::spacing_mark)  // GB9a
        result = false;  // NOLINT
    else if (p_prop == prop_gb::prepend)  // GB9b
        result = false;  // NOLINT
    else if (state->state == break_grapheme_state::ep_zwj
             && c_prop == prop_gb::extended_pictographic)  // GB11
        result = false;  // NOLINT
    else if (state->state == break_grapheme_state::ri
             && c_prop == prop_gb::regional_indicator)  // GB12/GB13
        result = false;  // NOLINT
    else  // GB999
        result = true;  // NOLINT

    // GB12/GB13
    if (c_prop == prop_gb::regional_indicator) {
        if (state->state == break_grapheme_state::ri)
            state->state = break_grapheme_state::ri_ri;
        else
            state->state = break_grapheme_state::ri;
    }
    // GB11
    else if (c_prop == prop_gb::extended_pictographic)
        state->state = break_grapheme_state::ep;  // NOLINT
    else if (state->state == break_grapheme_state::ep
             && c_prop == prop_gb::extend)
        state->state = break_grapheme_state::ep;  // NOLINT
    else if (state->state == break_grapheme_state::ep && c_prop == prop_gb::zwj)
        state->state = break_grapheme_state::ep_zwj;
    else
        state->state = break_grapheme_state::process;

    state->prev_cp = c;
    state->prev_cp_prop = c_prop;

    return result;
}

#ifdef __cplusplus
template<
    typename =
        void>  // TODO: What is this? Why uaix_inline is not used here instead of this crap?
#endif
bool impl_break_grapheme(struct impl_break_grapheme_state* state,
                         type_codept c) {
    return break_grapheme(state, c);
}

inline bool inline_break_grapheme(struct impl_break_grapheme_state* state,
                                  type_codept c) {
    return break_grapheme(state, c);
}

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
