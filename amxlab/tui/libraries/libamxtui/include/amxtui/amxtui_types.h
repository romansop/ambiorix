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

#if !defined(__AMXTUI_TYPES_H__)
#define __AMXTUI_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _amxtui_ctrl amxtui_ctrl_t;
typedef struct _amxtui_widget amxtui_widget_t;

typedef int (* amxtui_ctrl_type_ctor_fn_t) (amxtui_ctrl_t** ctrl);
typedef int (* amxtui_ctrl_type_dtor_fn_t) (amxtui_ctrl_t** ctrl);

typedef struct _amxtui_ctrl_type {
    amxc_htable_it_t hit;
    amxtui_ctrl_type_ctor_fn_t ctor;
    amxtui_ctrl_type_dtor_fn_t dtor;
} amxtui_ctrl_type_t;

typedef enum _amxtui_size_pos_type {
    amxtui_percentage,
    amxtui_absolute,
    amxtui_alligned,
    amxtui_maximized,
} amxtui_size_pos_type_t;

typedef enum _amxtui_alligment {
    amxtui_allign_left,
    amxtui_allign_right,
    amxtui_allign_top,
    amxtui_allign_bottom,
    amxtui_allign_center,
} amxtui_alligment_t;

typedef enum _amxtui_print_type {
    amxtui_print_normal,
    amxtui_print_selected,
    amxtui_print_focused,
    amxtui_print_data,
} amxtui_print_type_t;

typedef enum _amxtui_message_type {
    amxtui_message_normal,
    amxtui_message_warning,
    amxtui_message_error,
} amxtui_message_type_t;

typedef bool (* amxtui_ctrl_key_fn_t) (amxtui_widget_t* widget,
                                       amxtui_ctrl_t* ctrl,
                                       uint32_t ctrl_key);

typedef bool (* amxtui_key_fn_t) (amxtui_widget_t* widget,
                                  amxtui_ctrl_t* ctrl,
                                  char key);

typedef void (* amxtui_show_fn_t) (const amxtui_widget_t* widget,
                                   amxtui_ctrl_t* ctrl);

typedef amxc_var_t* (* amxtui_get_data_fn_t) (const amxtui_widget_t* widget,
                                              amxtui_ctrl_t* ctrl);

struct _amxtui_ctrl {
    amxtui_ctrl_key_fn_t handle_ctrl_key; // called when ctrl key sequence code is available, can be NULL
    amxtui_key_fn_t handle_key;           // called when key code is a available, can be NULL
    amxtui_show_fn_t show_content;        // called when inner of widget needs to be updated, can be NULL
    amxtui_get_data_fn_t get_data;        // called to get widget specific controller data, can be NULL
    amxp_signal_mngr_t sigmngr;           // the controller signal manager
    amxtui_ctrl_type_t* ctrl_type;        // the controller type
    uint32_t ref;                         // number of references taken
    amxtui_ctrl_t* base;                  // base controller
};

#ifdef __cplusplus
}
#endif

#endif // __AMXTUI_TYPES_H__
