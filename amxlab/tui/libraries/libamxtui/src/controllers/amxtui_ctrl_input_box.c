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

#include <amxtui/amxtui.h>
#include <amxtui/ctrl/amxtui_ctrl_input_box.h>

#define CTRL_NAME     "input-box"
#define WD_CURSOR_POS "cursor-pos"
#define WD_OFFSET     "offset"

typedef struct _amxtui_ctrl_input_box {
    amxt_il_t il;
    amxtui_ctrl_t ctrl;
} amxtui_ctrl_input_box_t;

static void amxtui_ctrl_ib_show(const amxtui_widget_t* widget,
                                amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_input_box_t* ibctrl = NULL;
    int cols = amxtui_widget_width(widget);
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* cursor_pos = GET_ARG(widget_data, WD_CURSOR_POS);
    amxc_var_t* text_offset = GET_ARG(widget_data, WD_OFFSET);

    int pos = GET_INT32(cursor_pos, NULL);
    int offset = GET_INT32(text_offset, NULL);
    int len = 0;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    ibctrl = amxc_container_of(ctrl, amxtui_ctrl_input_box_t, ctrl);
    if(pos == -1) {
        pos = amxt_il_text_length(&ibctrl->il, amxt_il_no_flags, 0);
    }

    len = (int) amxt_il_text_length(&ibctrl->il, amxt_il_no_flags, 0);
    amxt_il_set_cursor_pos(&ibctrl->il, pos);

    if(len < cols) {
        offset = 0;
    } else {
        if(pos - offset > cols) {
            offset += 1;
            while(pos - offset > cols) {
                offset += 1;
            }
        } else if(pos - offset < 0) {
            offset -= 1;
            while(pos - offset < 0) {
                offset -= 1;
            }
        }
    }

    amxtui_widget_print(widget,
                        0,
                        amxtui_allign_left,
                        amxtui_print_normal,
                        &ibctrl->il.buffer,
                        offset);
    amxtui_widget_show_cursor(widget, pos - offset, 0);
    if(GET_INT32(cursor_pos, NULL) != -1) {
        amxc_var_set(int32_t, cursor_pos, pos);
    }
    amxc_var_set(int32_t, text_offset, offset);
}

static bool amxtui_ctrl_ib_handle_ctrl_key(amxtui_widget_t* widget,
                                           amxtui_ctrl_t* ctrl,
                                           uint32_t ctrl_key) {
    amxtui_ctrl_input_box_t* ibctrl = NULL;
    int len = 0;
    bool handled = false;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* cursor_pos = GET_ARG(widget_data, WD_CURSOR_POS);

    // an input box that doesn't have focus is not handling any keys
    when_false(amxtui_widget_has_focus(widget), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    ibctrl = amxc_container_of(ctrl, amxtui_ctrl_input_box_t, ctrl);
    len = amxt_il_text_length(&ibctrl->il, amxt_il_no_flags, 0);
    if(GET_INT32(cursor_pos, NULL) == -1) {
        amxt_il_set_cursor_pos(&ibctrl->il, len);
    } else {
        amxt_il_set_cursor_pos(&ibctrl->il, GET_INT32(cursor_pos, NULL));
    }
    handled = true;

    switch(ctrl_key) {
    case amxt_key_backspace:
        amxt_il_remove_char(&ibctrl->il, amxt_il_no_flags);
        amxtui_ctrl_emit_text(&ibctrl->ctrl,
                              CTRL_SIG_IB_UPDATE,
                              amxt_il_text(&ibctrl->il, amxt_il_no_flags, 0));
        break;
    case amxt_key_left:
        amxt_il_move_cursor(&ibctrl->il, -1);
        break;
    case amxt_key_right:
        amxt_il_move_cursor(&ibctrl->il, 1);
        break;
    case amxt_key_delete:
        amxt_il_remove_char(&ibctrl->il, amxt_il_keep_cursor_pos);
        amxtui_ctrl_emit_text(&ibctrl->ctrl,
                              CTRL_SIG_IB_UPDATE,
                              amxt_il_text(&ibctrl->il, amxt_il_no_flags, 0));
        break;
    case amxt_key_newline:
        amxtui_ctrl_emit_text(&ibctrl->ctrl,
                              CTRL_SIG_IB_SELECT,
                              amxt_il_text(&ibctrl->il, amxt_il_no_flags, 0));
        amxt_il_reset(&ibctrl->il);
        break;
    case amxt_key_home:
        amxt_il_set_cursor_pos(&ibctrl->il, 0);
        break;
    case amxt_key_end:
        amxt_il_set_cursor_pos(&ibctrl->il, len);
        break;
    case amxt_key_insert:
        if(amxt_il_mode(&ibctrl->il) == amxc_string_overwrite) {
            amxt_il_set_mode(&ibctrl->il, amxc_string_insert);
        } else {
            amxt_il_set_mode(&ibctrl->il, amxc_string_overwrite);
        }
        break;
    default:
        handled = false;
        break;
    }

    len = (int) amxt_il_text_length(&ibctrl->il, amxt_il_no_flags, 0);
    if(amxt_il_cursor_pos(&ibctrl->il) < len) {
        amxc_var_set(int32_t, cursor_pos, amxt_il_cursor_pos(&ibctrl->il));
    } else {
        amxc_var_set(int32_t, cursor_pos, -1);
    }

    if(handled) {
        amxtui_screen_hide(widget);
        amxtui_ctrl_trigger_data(&ibctrl->ctrl, CTRL_SIG_REDRAW, NULL);
        amxtui_screen_show(widget);
    }

exit:
    return handled;
}

static bool amxtui_ctrl_ib_handle_key(amxtui_widget_t* widget,
                                      amxtui_ctrl_t* ctrl,
                                      char key) {
    amxtui_ctrl_input_box_t* ibctrl = NULL;
    amxc_var_t* widget_data = amxtui_widget_ctrl_data(widget);
    amxc_var_t* cursor_pos = GET_ARG(widget_data, WD_CURSOR_POS);

    char txt[2] = "";
    bool handled = false;

    // an input box that doesn't have focus is not handling any keys
    when_false(amxtui_widget_has_focus(widget), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    ibctrl = amxc_container_of(ctrl, amxtui_ctrl_input_box_t, ctrl);
    txt[0] = key;

    if(key == 0x1b) {
        amxtui_ctrl_emit_data(&ibctrl->ctrl, CTRL_SIG_IB_CANCEL, NULL);
        amxt_il_reset(&ibctrl->il);
        handled = true;
    } else if(isprint(key)) {
        amxt_il_set_cursor_pos(&ibctrl->il, GET_INT32(cursor_pos, NULL));
        amxt_il_insert_block(&ibctrl->il, txt, 1);
        if(GET_INT32(cursor_pos, NULL) != -1) {
            amxc_var_set(int32_t, cursor_pos, amxt_il_cursor_pos(&ibctrl->il));
        }

        amxtui_screen_hide(widget);
        amxtui_ctrl_trigger_data(&ibctrl->ctrl, CTRL_SIG_REDRAW, NULL);
        amxtui_screen_show(widget);

        amxtui_ctrl_emit_text(&ibctrl->ctrl,
                              CTRL_SIG_IB_UPDATE,
                              amxt_il_text(&ibctrl->il, amxt_il_no_flags, 0));
        handled = true;
    }

exit:
    return handled;
}

static amxc_var_t* amxtui_ctrl_ib_get_data(UNUSED const amxtui_widget_t* widget,
                                           UNUSED amxtui_ctrl_t* ctrl) {
    amxc_var_t* widget_data = NULL;

    amxc_var_new(&widget_data);
    amxc_var_set_type(widget_data, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(int32_t, widget_data, WD_CURSOR_POS, -1);
    amxc_var_add_key(int32_t, widget_data, WD_OFFSET, 0);

    return widget_data;
}

static int amxtui_ctrl_new_input_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_input_box_t* ibctrl = NULL;
    when_null(ctrl, exit);

    ibctrl = (amxtui_ctrl_input_box_t*) calloc(1, sizeof(amxtui_ctrl_input_box_t));
    amxt_il_init(&ibctrl->il, 100);
    amxtui_ctrl_init(&ibctrl->ctrl,
                     amxtui_ctrl_ib_show,
                     amxtui_ctrl_ib_handle_ctrl_key,
                     amxtui_ctrl_ib_handle_key,
                     amxtui_ctrl_ib_get_data);

    amxtui_ctrl_add_signal(&ibctrl->ctrl, CTRL_SIG_IB_UPDATE);
    amxtui_ctrl_add_signal(&ibctrl->ctrl, CTRL_SIG_IB_SELECT);
    amxtui_ctrl_add_signal(&ibctrl->ctrl, CTRL_SIG_IB_CANCEL);

    *ctrl = &ibctrl->ctrl;

exit:
    return 0;
}

static int amxtui_ctrl_delete_input_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_input_box_t* ibctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    ibctrl = amxc_container_of((*ctrl), amxtui_ctrl_input_box_t, ctrl);
    amxtui_ctrl_clean(&ibctrl->ctrl);
    amxt_il_clean(&ibctrl->il);
    free(ibctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_input_box = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_input_box,
    .dtor = amxtui_ctrl_delete_input_box
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_input_box, CTRL_NAME);
}

void amxtui_ib_widget_slot_set_text(UNUSED const char* const sig_name,
                                    const amxc_var_t* const data,
                                    void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    const char* txt = NULL;

    when_false(amxc_var_type_of(data) == AMXC_VAR_ID_CSTRING, exit);
    txt = GET_CHAR(data, NULL);

    amxtui_ctrl_ib_set_text(amxtui_widget_get_ctrl(widget), txt);

exit:
    return;
}

int amxtui_ctrl_ib_set_text(amxtui_ctrl_t* ctrl, const char* txt) {
    int rv = -1;
    amxtui_ctrl_input_box_t* ibctrl = NULL;

    when_null(ctrl, exit);
    when_null(txt, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    ibctrl = amxc_container_of(ctrl, amxtui_ctrl_input_box_t, ctrl);
    amxt_il_reset(&ibctrl->il);
    amxt_il_insert_block(&ibctrl->il, txt, strlen(txt));
    amxtui_ctrl_trigger_data(&ibctrl->ctrl, CTRL_SIG_REDRAW, NULL);
    rv = 0;

exit:
    return rv;
}
