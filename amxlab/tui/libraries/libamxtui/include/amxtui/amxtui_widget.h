/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2023 SoftAtHome
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above copyright notice,
** this list of conditions and the following disclaimer in the documentation
** and/or other materials provided with the distribution.
**
** Subject to the terms and conditions of this license, each copyright holder
** and contributor hereby grants to those receiving rights under this license
** a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
** (except for failure to satisfy the conditions of this license) patent license
** to make, have made, use, offer to sell, sell, import, and otherwise transfer
** this software, where such license applies only to those patent claims, already
** acquired or hereafter acquired, licensable by such copyright holder or contributor
** that are necessarily infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright holders and
** non-copyrightable additions of contributors, in source or binary form) alone;
** or
**
** (b) combination of their Contribution(s) with the work of authorship to which
** such Contribution(s) was added by such copyright holder or contributor, if,
** at the time the Contribution is added, such addition causes such combination
** to be necessarily infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any copyright
** holder or contributor is granted under this license, whether expressly, by
** implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
** USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#if !defined(__AMXTUI_WIDGET_H__)
#define __AMXTUI_WIDGET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <amxtui/amxtui_types.h>

#define AMXTUI_COLOR_GREEN_BLACK    1
#define AMXTUI_COLOR_WHITE_BLACK    2
#define AMXTUI_COLOR_BLUE_BLACK     3
#define AMXTUI_COLOR_RED_BLACK      4
#define AMXTUI_COLOR_YELLOW_BLACK   5
#define AMXTUI_COLOR_BLACK_WHITE    6
#define AMXTUI_COLOR_BLACK_CYAN     7
#define AMXTUI_COLOR_RED_CYAN       8
#define AMXTUI_COLOR_GREEN_CYAN     9
#define AMXTUI_COLOR_BLUE_CYAN      10
#define AMXTUI_COLOR_MAGENTA_CYAN   11
#define AMXTUI_COLOR_BLUE_WHITE     12

int amxtui_widget_new(amxtui_widget_t** widget,
                      const char* ctrl_type,
                      int32_t width,
                      amxtui_size_pos_type_t width_type,
                      int32_t height,
                      amxtui_size_pos_type_t height_type);

int amxtui_widget_delete(amxtui_widget_t** widget);

int amxtui_widget_init(amxtui_widget_t* widget,
                       const char* ctrl_type,
                       int32_t width,
                       amxtui_size_pos_type_t width_type,
                       int32_t height,
                       amxtui_size_pos_type_t height_type);

int amxtui_widget_clean(amxtui_widget_t* widget);

int amxtui_widget_add_widget(amxtui_widget_t* parent,
                             amxtui_widget_t* child);

int amxtui_widget_redraw(const amxtui_widget_t* widget);

int amxtui_widget_set_pos(amxtui_widget_t* widget,
                          int32_t x,
                          amxtui_size_pos_type_t x_type,
                          int32_t y,
                          amxtui_size_pos_type_t y_type);

int amxtui_widget_set_width(amxtui_widget_t* widget,
                            int32_t width,
                            amxtui_size_pos_type_t width_type);

int amxtui_widget_set_height(amxtui_widget_t* widget,
                             int32_t height,
                             amxtui_size_pos_type_t height_type);

int amxtui_widget_set_ctrl(amxtui_widget_t* widget,
                           amxtui_ctrl_t* ctrl);

amxtui_ctrl_t* amxtui_widget_get_ctrl(const amxtui_widget_t* widget);

int amxtui_widget_set_title(amxtui_widget_t* widget,
                            const char* title);

int amxtui_widget_set_box(amxtui_widget_t* widget,
                          bool has_box);

int amxtui_widget_set_color(amxtui_widget_t* widget,
                            amxtui_print_type_t type,
                            int32_t color_code);

int amxtui_widget_set_modal(amxtui_widget_t* widget,
                            bool modal);

int amxtui_widget_add_button(amxtui_widget_t* widget,
                             const char* button_txt);

int amxtui_widget_remove_button(amxtui_widget_t* widget,
                                const char* button_txt);

int amxtui_widget_print_at(const amxtui_widget_t* widget,
                           int line,
                           int col,
                           amxtui_print_type_t type,
                           const amxc_string_t* txt,
                           size_t str_offset);

int amxtui_widget_print(const amxtui_widget_t* widget,
                        int line,
                        amxtui_alligment_t allign,
                        amxtui_print_type_t type,
                        const amxc_string_t* txt,
                        size_t str_offset);

int amxtui_widget_show_cursor(const amxtui_widget_t* widget,
                              int x,
                              int y);

bool amxtui_widget_is_child(const amxtui_widget_t* widget);

bool amxtui_widget_is_child_of(const amxtui_widget_t* widget,
                               const amxtui_widget_t* parent);

bool amxtui_widget_has_focus(const amxtui_widget_t* widget);

size_t amxtui_widget_width(const amxtui_widget_t* widget);

size_t amxtui_widget_height(const amxtui_widget_t* widget);

int amxtui_widget_screen_pos_x(const amxtui_widget_t* widget);

int amxtui_widget_screen_pos_y(const amxtui_widget_t* widget);

int amxtui_widget_connect(const amxtui_widget_t* source_widget,
                          amxtui_widget_t* dest_widget,
                          const char* signal,
                          amxp_slot_fn_t fn);

amxc_var_t* amxtui_widget_ctrl_data(const amxtui_widget_t* widget);

#ifdef __cplusplus
}
#endif

#endif // __AMXTUI_WIDGET_H__
