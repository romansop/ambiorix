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

#if !defined(__AMXTUI_PRIV_H__)
#define __AMXTUI_PRIV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <ncurses.h>
#include <amxc/amxc_macros.h>
#include <amxtui/amxtui_types.h>

typedef struct _amxtui_widget_attr {
    uint32_t has_focus : 1;
    uint32_t is_visible : 1;
    uint32_t has_box : 1;
    uint32_t is_modal : 1;
} amxtui_widget_attr_t;

struct _amxtui_widget {
    WINDOW* window;                     // ncurses window
    int32_t width;                      // configured width - exact width needs to be calculated
    int32_t height;                     // configured height - exact hieght needs to be calculated
    amxtui_size_pos_type_t width_type;  // width type - used in calculation of exact width
    amxtui_size_pos_type_t height_type; // hieght type - used in calucalation of exact height
    int32_t x;                          // configured x pos - exact pos needs to be calculated
    int32_t y;                          // configured y pos - exact pos needs to be calculated
    amxtui_size_pos_type_t x_type;      // x pos type - used in calculation of exact x pos
    amxtui_size_pos_type_t y_type;      // y pos type - used in calculation of exact y pos
    amxtui_widget_attr_t attrs;         // widget attributes
    char* title;                        // widget title - can be NULL
    amxc_llist_it_t it;                 // linked list iterator used to add widget to screen, only widgets added to screen are displayed
    amxc_llist_t buttons;               // linked list of buttons, displayed on bottom of widget box
    int32_t active_button;              // current active button, set to -1 if no button active
    amxtui_ctrl_t* ctrl;                // current widget controller
    amxc_var_t* ctrl_data;              // specific controller data for this widget
    int32_t colors[4];                  // color pairs indexes
    amxc_llist_t children;              // list of child widgets
};

PRIVATE
void amxtui_screen_disconnect_widget(amxtui_widget_t* widget);

PRIVATE
bool amxtui_widget_handle_ctrl_key(amxtui_widget_t* widget, uint32_t ctrl_key);

PRIVATE
amxc_llist_t* amxtui_screen_children(void);

PRIVATE
bool amxtui_widget_handle_key(amxtui_widget_t* widget, char key);

#ifdef __cplusplus
}
#endif

#endif // __AMXTUI_PRIV_H__
