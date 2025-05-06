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
#include <amxtui/ctrl/amxtui_ctrl_message_box.h>

#define CTRL_NAME        "message-box"

typedef struct _amxtui_ctrl_message_box {
    amxc_string_t message;
    amxtui_ctrl_t ctrl;
    amxtui_alligment_t allign;
} amxtui_ctrl_message_box_t;

static int amxtui_get_text_lines(const amxtui_widget_t* widget,
                                 amxc_string_t* text,
                                 int max_width,
                                 int line) {
    int lines = 0;
    int text_left = amxc_string_text_length(text);
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    char* txt = NULL;
    int current_pos = 0;
    amxc_string_t tmp;

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    mbctrl = amxc_container_of(ctrl, amxtui_ctrl_message_box_t, ctrl);

    amxc_string_init(&tmp, max_width);

    while(text_left > max_width) {
        int len = 0;
        char* newline = NULL;
        txt = amxc_string_dup(text, current_pos, max_width);
        newline = strchr(txt, '\n');
        if(newline != NULL) {
            newline[0] = 0;
            len = strlen(txt);
        } else {
            len = strlen(txt);
            while(len > 0) {
                if(isblank(txt[len]) != 0) {
                    txt[len] = 0;
                    break;
                }
                len--;
            }
            if(len == 0) {
                free(txt);
                txt = amxc_string_dup(text, current_pos, max_width);
                len = strlen(txt);
            }
        }
        if(widget != NULL) {
            amxc_string_set(&tmp, txt);
            int rv = amxtui_widget_print(widget,
                                         line + lines,
                                         mbctrl->allign,
                                         amxtui_print_normal,
                                         &tmp,
                                         0);
            if(rv != 0) {
                free(txt);
                lines = -1;
                break;
            }
        }
        text_left -= (len + 1);
        current_pos += (len + 1);
        lines++;
        free(txt);
    }

    if(text_left > 0) {
        if(widget != NULL) {
            amxtui_widget_print(widget,
                                line + lines,
                                mbctrl->allign,
                                amxtui_print_normal,
                                text,
                                current_pos);
        }
        lines++;
    }

    amxc_string_clean(&tmp);
    return lines;
}

static void amxtui_ctrl_mb_show(const amxtui_widget_t* widget,
                                amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    int line = 0;
    int lines_needed = 0;
    int width = amxtui_widget_width(widget);
    amxc_llist_t lines;

    amxc_llist_init(&lines);
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    mbctrl = amxc_container_of(ctrl, amxtui_ctrl_message_box_t, ctrl);

    amxc_string_split_to_llist(&mbctrl->message, &lines, '\n');

    amxc_llist_for_each(it, &lines) {
        amxc_string_t* txt = amxc_string_from_llist_it(it);
        lines_needed = amxtui_get_text_lines(widget, txt, width, line);
        if(lines_needed == 0) {
            line++;
            continue;
        }
        line += lines_needed;
    }

    amxc_llist_clean(&lines, amxc_string_list_it_free);
}

static bool amxtui_ctrl_mb_handle_ctrl_key(UNUSED amxtui_widget_t* widget,
                                           amxtui_ctrl_t* ctrl,
                                           uint32_t ctrl_key) {
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    bool handled = true;
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    mbctrl = amxc_container_of(ctrl, amxtui_ctrl_message_box_t, ctrl);

    switch(ctrl_key) {
    case amxt_key_newline:
        amxtui_screen_hide(widget);
        amxtui_ctrl_trigger_data(&mbctrl->ctrl,
                                 CTRL_SIG_MB_CLOSE,
                                 NULL);
        break;
    default:
        handled = false;
        break;
    }

    return handled;
}

static int amxtui_ctrl_new_message_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    when_null(ctrl, exit);

    mbctrl = (amxtui_ctrl_message_box_t*) calloc(1, sizeof(amxtui_ctrl_message_box_t));
    amxc_string_init(&mbctrl->message, 10);
    mbctrl->allign = amxtui_allign_center;
    amxtui_ctrl_init(&mbctrl->ctrl,
                     amxtui_ctrl_mb_show,
                     amxtui_ctrl_mb_handle_ctrl_key,
                     NULL,
                     NULL);

    amxtui_ctrl_add_signal(&mbctrl->ctrl, CTRL_SIG_MB_CLOSE);

    *ctrl = &mbctrl->ctrl;
exit:
    return 0;
}

static int amxtui_ctrl_delete_message_box(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    mbctrl = amxc_container_of((*ctrl), amxtui_ctrl_message_box_t, ctrl);
    amxtui_ctrl_clean(&mbctrl->ctrl);
    amxc_string_clean(&mbctrl->message);
    free(mbctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_message_box = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_message_box,
    .dtor = amxtui_ctrl_delete_message_box
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    amxtui_ctrl_type_register(&amxtui_ctrl_message_box, CTRL_NAME);
}

int amxtui_ctrl_mb_set_message(amxtui_ctrl_t* ctrl,
                               const char* msg,
                               amxtui_alligment_t allign) {
    int rv = -1;
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    mbctrl = amxc_container_of(ctrl, amxtui_ctrl_message_box_t, ctrl);
    mbctrl->allign = allign;

    amxc_string_set(&mbctrl->message, msg);
    rv = 0;
exit:
    return rv;
}

int amxtui_ctrl_mb_lines_needed(amxtui_ctrl_t* ctrl, int max_width) {
    int lines_needed = 0;
    amxtui_ctrl_message_box_t* mbctrl = NULL;
    amxc_llist_t lines;

    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    mbctrl = amxc_container_of(ctrl, amxtui_ctrl_message_box_t, ctrl);
    amxc_llist_init(&lines);
    amxc_string_split_to_llist(&mbctrl->message, &lines, '\n');

    amxc_llist_for_each(it, &lines) {
        amxc_string_t* txt = amxc_string_from_llist_it(it);
        int line = amxtui_get_text_lines(NULL, txt, max_width, 0);
        if(line == 0) {
            lines_needed++;
        } else {
            lines_needed += amxtui_get_text_lines(NULL, txt, max_width, 0);
        }
    }

    amxc_llist_clean(&lines, amxc_string_list_it_free);

exit:
    return lines_needed;
}
