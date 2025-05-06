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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <sys/ioctl.h>

#include <amxtui/amxtui.h>
#include <amxtui/ctrl/amxtui_ctrl_message_box.h>
#include "amxtui_priv.h"

typedef struct _amxtui_screen {
    amxp_signal_mngr_t sigmngr;
    amxc_llist_t windows;
    struct winsize ws;
    amxtui_widget_t* focused;
    amxtui_ctrl_t* ctrl;
    amxtui_widget_t* restore;
} amxtui_screen_t;

static amxtui_screen_t screen;

static void amxtui_screen_resize(UNUSED const char* const sig_name,
                                 UNUSED const amxc_var_t* const data,
                                 UNUSED void* const priv) {
    ioctl(STDIN_FILENO, TIOCGWINSZ, &screen.ws);

    resize_term(screen.ws.ws_row, screen.ws.ws_col);

    // Refresh windows
    erase();
    clear();
    amxtui_screen_redraw();
}

static bool amxtui_screen_contains_widget(amxtui_widget_t* widget) {
    bool in_screen = false;

    while(amxtui_widget_is_child(widget)) {
        widget = amxc_container_of(widget->it.llist, amxtui_widget_t, children);
    }

    when_false(amxc_llist_it_is_in_list(&widget->it), exit);
    when_true(widget->it.llist != &screen.windows, exit);
    in_screen = true;

exit:
    return in_screen;
}

static void amxtui_screen_emit_key(amxt_ctrl_key_sequence_t* ctrl_key,
                                   char key) {
    amxc_var_t key_stroke;
    amxc_var_t* key_type = NULL;
    amxc_var_t* key_value = NULL;

    amxc_var_init(&key_stroke);
    amxc_var_set_type(&key_stroke, AMXC_VAR_ID_HTABLE);
    key_type = amxc_var_add_new_key(&key_stroke, "type");
    key_value = amxc_var_add_new_key(&key_stroke, "value");

    if(ctrl_key != NULL) {
        amxc_var_set(uint32_t, key_type, 1);
        amxc_var_set(uint32_t, key_value, ctrl_key->key);
    } else {
        amxc_var_set(uint32_t, key_type, 0);
        amxc_var_set(uint8_t, key_value, key);
    }
    amxp_sigmngr_trigger_signal(&screen.sigmngr, "key", &key_stroke);

    amxc_var_clean(&key_stroke);
}

static bool amxtui_screen_handle_key(amxt_ctrl_key_sequence_t* ctrl_key,
                                     char key) {
    bool handled = false;

    // first pass to widget that has focus
    if(screen.focused != NULL) {
        if(ctrl_key != NULL) {
            handled = amxtui_widget_handle_ctrl_key(screen.focused, ctrl_key->key);
        } else {
            handled = amxtui_widget_handle_key(screen.focused, key);
        }
    }
    when_true(handled, exit);

    when_true(screen.focused != NULL && screen.focused->attrs.is_modal, exit);

    // then pass to screen controller if available
    if(screen.ctrl != NULL) {
        if(ctrl_key != NULL) {
            if(screen.ctrl->handle_ctrl_key != NULL) {
                handled = screen.ctrl->handle_ctrl_key(NULL, screen.ctrl, ctrl_key->key);
            }
        } else {
            if(screen.ctrl->handle_key != NULL) {
                handled = screen.ctrl->handle_key(NULL, screen.ctrl, key);
            }
        }
    }
    when_true(handled, exit);

    // then pass to non-focused widgets
    // the first that handles the key takes it.
    if((screen.focused == NULL) || !screen.focused->attrs.is_modal) {
        amxc_llist_for_each(it, &screen.windows) {
            amxtui_widget_t* widget = amxc_container_of(it, amxtui_widget_t, it);
            if(widget == screen.focused) {
                continue;
            }
            if(ctrl_key != NULL) {
                handled = amxtui_widget_handle_ctrl_key(widget, ctrl_key->key);
            } else {
                handled = amxtui_widget_handle_key(widget, key);
            }
            if(handled) {
                break;
            }
        }
        when_true(handled, exit);

        amxtui_screen_emit_key(ctrl_key, key);
    }

exit:
    amxtui_widget_redraw(screen.focused);
    return handled;
}

static void amxtui_screen_mb_resize(UNUSED const char* const sig_name,
                                    UNUSED const amxc_var_t* const data,
                                    void* const priv) {
    amxtui_widget_t* message_box = (amxtui_widget_t*) priv;
    amxtui_ctrl_t* ctrl = NULL;
    int max_width = 0;

    ctrl = amxtui_widget_get_ctrl(message_box);
    max_width = amxtui_widget_width(message_box);
    amxtui_widget_set_height(message_box,
                             amxtui_ctrl_mb_lines_needed(ctrl, max_width) + 2,
                             amxtui_absolute);
    amxtui_widget_redraw(message_box);
}

static void amxtui_screen_delete_mb(UNUSED const amxc_var_t* const data,
                                    void* const priv) {
    amxtui_widget_t* mb = (amxtui_widget_t*) priv;
    amxtui_widget_delete(&mb);
}

static void amxtui_screen_close_mb(UNUSED const char* const sig_name,
                                   UNUSED const amxc_var_t* const data,
                                   void* const priv) {
    amxtui_widget_t* mb = (amxtui_widget_t*) priv;
    amxtui_widget_t* parent_widget = NULL;

    if(amxtui_widget_is_child(mb)) {
        parent_widget = amxc_container_of(mb->it.llist, amxtui_widget_t, children);
    }

    amxp_slot_disconnect_all(amxtui_screen_mb_resize);

    if(parent_widget == NULL) {
        amxtui_screen_set_focus(screen.restore);
        screen.restore = NULL;
        amxp_sigmngr_deferred_call(NULL, amxtui_screen_delete_mb, NULL, mb);
        amxtui_screen_redraw();
    } else {
        amxtui_screen_set_focus(parent_widget);
        amxp_sigmngr_deferred_call(NULL, amxtui_screen_delete_mb, NULL, mb);
        amxtui_widget_redraw(parent_widget);
    }
}

void amxtui_screen_disconnect_widget(amxtui_widget_t* widget) {
    amxc_llist_for_each(it, &screen.windows) {
        amxtui_widget_t* w = amxc_container_of(it, amxtui_widget_t, it);
        if(w->ctrl != NULL) {
            amxp_slot_disconnect_with_priv(&w->ctrl->sigmngr, NULL, widget);
        }
    }
}

amxc_llist_t* amxtui_screen_children(void) {
    return &screen.windows;
}

int amxtui_screen_init(void) {
    // Initialize ncurses
    initscr();
    start_color();
    cbreak();

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_BLACK, COLOR_WHITE);
    init_pair(7, COLOR_BLACK, COLOR_CYAN);
    init_pair(8, COLOR_RED, COLOR_CYAN);
    init_pair(9, COLOR_GREEN, COLOR_CYAN);
    init_pair(10, COLOR_BLUE, COLOR_CYAN);
    init_pair(11, COLOR_MAGENTA, COLOR_CYAN);
    init_pair(12, COLOR_BLUE, COLOR_WHITE);

    keypad(stdscr, TRUE);

    amxp_sigmngr_init(&screen.sigmngr);

    amxp_syssig_enable(SIGWINCH, true);
    amxp_slot_connect(NULL,
                      strsignal(SIGWINCH),
                      NULL,
                      amxtui_screen_resize,
                      NULL);

    noecho();

    ioctl(STDIN_FILENO, TIOCGWINSZ, &screen.ws);
    resize_term(screen.ws.ws_row, screen.ws.ws_col);

    return 0;
}

int amxtui_screen_cleanup(void) {
    amxp_syssig_enable(SIGWINCH, false);
    amxp_sigmngr_clean(&screen.sigmngr);
    endwin();
    return 0;
}

int amxtui_screen_set_controller(amxtui_ctrl_t* ctrl) {
    screen.ctrl = ctrl;
    return 0;
}

int amxtui_screen_read(void) {
    char buf[32];
    ssize_t bytes = 0;
    int flags = 0;
    int fd = fileno(stdin);
    amxt_ctrl_key_sequence_t* ctrl_seq = NULL;

    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    bytes = amxt_read(fd, buf, sizeof(buf));
    fcntl(fd, F_SETFL, flags);

    for(ssize_t buf_pos = 0; buf_pos < bytes; buf_pos++) {
        ctrl_seq = amxt_is_ctrl_sequence(buf + buf_pos);
        if(ctrl_seq != NULL) {
            amxtui_screen_handle_key(ctrl_seq, 0);
            buf_pos += ctrl_seq->seqlen;
        } else {
            amxtui_screen_handle_key(NULL, buf[buf_pos]);
        }
    }

    return 0;
}

int amxtui_screen_width(void) {
    return screen.ws.ws_col;
}

int amxtui_screen_height(void) {
    return screen.ws.ws_row;
}

bool amxtui_screen_resized(void) {
    struct winsize ws;
    bool resized = false;

    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

    if((ws.ws_row != screen.ws.ws_row) ||
       (ws.ws_col != screen.ws.ws_col)) {
        screen.ws.ws_col = ws.ws_col;
        screen.ws.ws_row = ws.ws_row;
        resize_term(screen.ws.ws_row, screen.ws.ws_col);

        // Refresh windows
        clear();
        amxtui_screen_redraw();
        resized = true;
    }

    return resized;
}

int amxtui_screen_redraw(void) {
    refresh();

    amxc_llist_for_each(it, &screen.windows) {
        amxtui_widget_t* widget = amxc_container_of(it, amxtui_widget_t, it);
        if(widget != screen.focused) {
            amxtui_widget_redraw(widget);
        }
    }

    if(screen.focused != NULL) {
        amxtui_widget_redraw(screen.focused);
    }

    return 0;
}

int amxtui_screen_add_widget(amxtui_widget_t* widget) {
    when_null(widget, exit);

    amxc_llist_append(&screen.windows, &widget->it);
exit:
    return 0;
}

int amxtui_screen_remove_widget(amxtui_widget_t* widget) {
    when_null(widget, exit);

    amxc_llist_it_take(&widget->it);
    if(widget == screen.focused) {
        screen.focused = NULL;
    }

exit:
    return 0;
}

int amxtui_screen_show(amxtui_widget_t* widget) {
    when_null(widget, exit);
    when_false(amxtui_screen_contains_widget(widget), exit);

    widget->attrs.is_visible = true;

exit:
    return 0;
}

int amxtui_screen_hide(amxtui_widget_t* widget) {
    when_null(widget, exit);
    when_false(amxtui_screen_contains_widget(widget), exit);

    widget->attrs.is_visible = false;

exit:
    return 0;
}

int amxtui_screen_set_focus(amxtui_widget_t* widget) {
    when_null(widget, exit);
    when_false(amxtui_screen_contains_widget(widget), exit);

    if(screen.focused != NULL) {
        screen.focused->attrs.has_focus = false;
        wbkgdset(screen.focused->window, COLOR_PAIR(screen.focused->colors[amxtui_print_normal]));
        amxtui_widget_redraw(screen.focused);
    }

    screen.focused = widget;

    screen.focused->attrs.has_focus = true;
    wbkgdset(screen.focused->window, COLOR_PAIR(screen.focused->colors[amxtui_print_focused]));

    while(amxtui_widget_is_child(widget)) {
        widget = amxc_container_of(widget->it.llist, amxtui_widget_t, children);
        widget->attrs.has_focus = true;
        wbkgdset(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
    }
    amxtui_widget_redraw(screen.focused);

exit:
    return 0;
}

amxtui_widget_t* amxtui_screen_get_focused(void) {
    return screen.focused;
}

int amxtui_screen_show_message(const char* message,
                               amxtui_widget_t* parent,
                               amxtui_message_type_t type,
                               amxtui_alligment_t allign) {
    int rv = -1;
    amxtui_widget_t* message_box = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    int max_width = 0;

    when_str_empty(message, exit);

    amxtui_widget_new(&message_box,
                      "message-box",
                      50,
                      amxtui_percentage,
                      3,
                      amxtui_absolute);


    ctrl = amxtui_widget_get_ctrl(message_box);
    amxtui_ctrl_mb_set_message(ctrl, message, allign);
    if(parent != NULL) {
        amxtui_widget_add_widget(parent, message_box);
    } else {
        amxtui_screen_add_widget(message_box);
        screen.restore = screen.focused;
    }
    max_width = amxtui_widget_width(message_box);
    amxtui_widget_set_height(message_box,
                             amxtui_ctrl_mb_lines_needed(ctrl, max_width) + 2,
                             amxtui_absolute);
    amxtui_widget_set_modal(message_box, true);

    switch(type) {
    case amxtui_message_normal:
        amxtui_widget_set_color(message_box, amxtui_print_normal, AMXTUI_COLOR_BLACK_CYAN);
        amxtui_widget_set_color(message_box, amxtui_print_focused, AMXTUI_COLOR_BLACK_CYAN);
        amxtui_widget_set_title(message_box, "Info");
        break;
    case amxtui_message_warning:
        amxtui_widget_set_color(message_box, amxtui_print_normal, AMXTUI_COLOR_BLUE_CYAN);
        amxtui_widget_set_color(message_box, amxtui_print_focused, AMXTUI_COLOR_BLUE_CYAN);
        amxtui_widget_set_title(message_box, "Warning");
        break;
    case amxtui_message_error:
        amxtui_widget_set_color(message_box, amxtui_print_normal, AMXTUI_COLOR_RED_CYAN);
        amxtui_widget_set_color(message_box, amxtui_print_focused, AMXTUI_COLOR_RED_CYAN);
        amxtui_widget_set_title(message_box, "Error");
        break;
    }
    amxtui_widget_set_pos(message_box,
                          amxtui_allign_center,
                          amxtui_alligned,
                          amxtui_allign_center,
                          amxtui_alligned);

    amxtui_screen_set_focus(message_box);

    amxtui_widget_connect(message_box, message_box, CTRL_SIG_MB_CLOSE, amxtui_screen_close_mb);
    amxp_slot_connect(NULL,
                      strsignal(SIGWINCH),
                      NULL,
                      amxtui_screen_mb_resize,
                      message_box);

    rv = 0;

exit:
    return rv;
}