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

#include <amxtui/amxtui.h>
#include <amxtui/ctrl/amxtui_ctrl_message_box.h>

#include "amxtui_priv.h"

typedef enum _direction {
    direction_horizontal,
    direction_vertical
} direction_t;

static void amxtui_widget_slot_redraw(UNUSED const char* const sig_name,
                                      UNUSED const amxc_var_t* const data,
                                      void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;

    amxtui_widget_redraw(widget);
}

static void amxtui_widget_slot_reset(UNUSED const char* const sig_name,
                                     UNUSED const amxc_var_t* const data,
                                     void* const priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;

    if((widget->ctrl != NULL) && (widget->ctrl->get_data != NULL)) {
        amxc_var_delete(&widget->ctrl_data);
        widget->ctrl_data = widget->ctrl->get_data(widget, widget->ctrl);
        amxtui_widget_redraw(widget);
    }
}

static int amxtui_widget_calculate(const amxtui_widget_t* widget,
                                   int full,
                                   int set,
                                   amxtui_size_pos_type_t type,
                                   direction_t direction) {
    int calculated = 0;
    int box = widget->attrs.has_box? 2:0;

    switch(type) {
    case amxtui_absolute:
        if(set < 0) {
            calculated = full + set;
        } else {
            calculated = set;
        }
        break;
    case amxtui_percentage:
        calculated = ((double) full / 100) * set;
        break;
    case amxtui_alligned:
        if((set == amxtui_allign_top) ||
           (set == amxtui_allign_left)) {
            calculated = 0;
        } else if(set == amxtui_allign_bottom) {
            calculated = full - (amxtui_widget_height(widget) + box);
        } else if(set == amxtui_allign_right) {
            calculated = full - (amxtui_widget_width(widget) + box);
        } else if(set == amxtui_allign_center) {
            if(direction == direction_horizontal) {
                calculated = (full - (amxtui_widget_width(widget) + box)) >> 1;
            } else {
                calculated = (full - (amxtui_widget_height(widget) + box)) >> 1;
            }
        }
        break;
    case amxtui_maximized:
        if(direction == direction_horizontal) {
            int pos_x = amxtui_widget_calculate(widget, full, widget->x, widget->x_type, direction);
            calculated = full - pos_x;
        } else {
            int pos_y = amxtui_widget_calculate(widget, full, widget->y, widget->y_type, direction);
            calculated = full - pos_y;
        }
        break;
    }

    return calculated;
}

static int amxtui_widget_draw_buttons(const amxtui_widget_t* widget,
                                      int lines,
                                      int cols) {
    // buttons are only visisble when:
    // 1. the widget has a box
    // 2. the widget has focus
    int x = 3;
    int index = 0;
    amxc_llist_it_t* current = amxc_llist_get_first(&widget->buttons);

    if(widget->active_button >= 0) {
        int needed_width = 3;
        amxc_llist_for_each(it, &widget->buttons) {
            amxc_string_t* str = amxc_container_of(it, amxc_string_t, it);
            needed_width += amxc_string_text_length(str) + 2;
            if(index == widget->active_button) {
                needed_width -= 2;
                break;
            }
            index++;
        }

        index = 0;
        if(needed_width > cols - 3) {
            // print indicator that more buttons on the left are available
            wattron(widget->window, COLOR_PAIR(AMXTUI_COLOR_BLACK_CYAN));
            mvwprintw(widget->window, lines - 1, 1, "<");
            wattroff(widget->window, COLOR_PAIR(AMXTUI_COLOR_BLACK_CYAN));
        }
        while(needed_width > cols - 3) {
            amxc_string_t* str = amxc_container_of(current, amxc_string_t, it);
            needed_width -= amxc_string_text_length(str);
            index++;
            current = amxc_llist_it_get_next(current);
        }
    }

    while(current != NULL) {
        amxc_string_t* str = amxc_container_of(current, amxc_string_t, it);
        if(x + (int) amxc_string_text_length(str) > cols - 3) {
            // print indicator that more buttons on the right are available
            wattron(widget->window, COLOR_PAIR(AMXTUI_COLOR_BLACK_CYAN));
            mvwprintw(widget->window, lines - 1, cols - 2, ">");
            wattroff(widget->window, COLOR_PAIR(AMXTUI_COLOR_BLACK_CYAN));
            break;
        }
        if(index == widget->active_button) {
            wattron(widget->window, A_STANDOUT);
        }
        mvwprintw(widget->window, lines - 1, x, "%s", amxc_string_get(str, 0));
        if(index == widget->active_button) {
            wattroff(widget->window, A_STANDOUT);
        }
        x += amxc_string_text_length(str) + 2;
        current = amxc_llist_it_get_next(current);
        index++;
    }

    return 0;
}

bool amxtui_widget_handle_ctrl_key(amxtui_widget_t* widget,
                                   uint32_t ctrl_key) {
    bool handled = false;

    when_false(widget->attrs.is_visible, exit);

    // buttons are only available when
    // 1. the widget has focus
    // 2. the widget has a box
    // 3. the widget has buttons
    if(widget->attrs.has_focus &&
       widget->attrs.has_box &&
       !amxc_llist_is_empty(&widget->buttons)) {
        int nbuttons = (int) amxc_llist_size(&widget->buttons);
        switch(ctrl_key) {
        case amxt_key_left:
            if(widget->active_button >= 0) {
                handled = true;
                widget->active_button--;
                if(widget->active_button >= nbuttons) {
                    widget->active_button = -1;
                }
            }
            break;
        case amxt_key_right:
            if(widget->active_button >= 0) {
                handled = true;
                widget->active_button++;
            }
            break;
        case amxt_key_tab:
            handled = true;
            widget->active_button++;
            if(widget->active_button >= nbuttons) {
                widget->active_button = -1;
            }
            break;
        case amxt_key_newline:
            if(widget->active_button >= 0) {
                handled = true;
                amxc_llist_it_t* it = amxc_llist_get_at(&widget->buttons,
                                                        widget->active_button);
                amxc_string_t* str = amxc_container_of(it, amxc_string_t, it);
                amxtui_ctrl_emit_text(widget->ctrl,
                                      CTRL_SIG_BUTTON,
                                      amxc_string_get(str, 0));
                widget->active_button = -1;
            }
            break;
        default:
            handled = false;
            break;
        }
        if(handled) {
            amxtui_widget_redraw(widget);
            goto exit;
        }
    }

    amxc_llist_for_each(it, &widget->children) {
        amxtui_widget_t* child = amxc_container_of(it, amxtui_widget_t, it);
        handled = amxtui_widget_handle_ctrl_key(child, ctrl_key);
        if(handled) {
            break;
        }
    }

    if((widget->ctrl != NULL) && (widget->ctrl->handle_ctrl_key != NULL)) {
        handled = widget->ctrl->handle_ctrl_key(widget, widget->ctrl, ctrl_key);
    }

exit:
    return handled;
}

bool amxtui_widget_handle_key(amxtui_widget_t* widget,
                              char key) {
    bool handled = false;

    when_false(widget->attrs.is_visible, exit);

    if((widget->ctrl != NULL) && (widget->ctrl->handle_key != NULL)) {
        handled = widget->ctrl->handle_key(widget, widget->ctrl, key);
    }
    when_true(handled, exit);

    amxc_llist_for_each(it, &widget->children) {
        amxtui_widget_t* child = amxc_container_of(it, amxtui_widget_t, it);
        handled = amxtui_widget_handle_key(child, key);
        if(handled) {
            break;
        }
    }

exit:
    return handled;
}

int amxtui_widget_new(amxtui_widget_t** widget,
                      const char* ctrl_type,
                      int32_t width,
                      amxtui_size_pos_type_t width_type,
                      int32_t height,
                      amxtui_size_pos_type_t height_type) {
    int rv = 0;
    when_null(widget, exit);
    *widget = (amxtui_widget_t*) calloc(1, sizeof(amxtui_widget_t));
    rv = amxtui_widget_init(*widget,
                            ctrl_type,
                            width,
                            width_type,
                            height,
                            height_type);
    if(rv != 0) {
        free(*widget);
        *widget = 0;
    }

exit:
    return rv;
}

int amxtui_widget_delete(amxtui_widget_t** widget) {
    when_null(widget, exit);
    when_null((*widget), exit);

    amxtui_widget_clean(*widget);
    free(*widget);
    *widget = NULL;

exit:
    return 0;
}

int amxtui_widget_init(amxtui_widget_t* widget,
                       const char* ctrl_type,
                       int32_t width,
                       amxtui_size_pos_type_t width_type,
                       int32_t height,
                       amxtui_size_pos_type_t height_type) {
    int nlines = 0;
    int ncols = 0;
    int rv = -1;

    when_null(widget, exit);
    when_true(width_type == amxtui_alligned, exit);
    when_true(height_type == amxtui_alligned, exit);

    widget->width = width;
    widget->height = height;
    widget->width_type = width_type;
    widget->height_type = height_type;
    widget->x = 0;
    widget->y = 0;
    widget->x_type = amxtui_absolute;
    widget->x_type = amxtui_absolute;
    widget->attrs.has_focus = false;
    widget->attrs.is_visible = true;
    widget->attrs.has_box = true;
    widget->title = NULL;
    widget->active_button = -1;
    widget->ctrl = NULL;
    widget->ctrl_data = NULL;
    widget->colors[amxtui_print_normal] = AMXTUI_COLOR_WHITE_BLACK;
    widget->colors[amxtui_print_focused] = AMXTUI_COLOR_GREEN_BLACK;
    widget->colors[amxtui_print_data] = AMXTUI_COLOR_BLUE_BLACK;
    widget->colors[amxtui_print_selected] = AMXTUI_COLOR_BLACK_WHITE;

    amxc_llist_it_init(&widget->it);
    amxc_llist_init(&widget->buttons);
    amxc_llist_init(&widget->children);

    nlines = amxtui_widget_calculate(widget,
                                     amxtui_screen_height(),
                                     widget->height,
                                     widget->height_type,
                                     direction_vertical);
    ncols = amxtui_widget_calculate(widget,
                                    amxtui_screen_width(),
                                    widget->width,
                                    widget->width_type,
                                    direction_horizontal);

    widget->window = newwin(nlines, ncols, widget->y, widget->x);
    wbkgdset(widget->window, COLOR_PAIR(widget->colors[amxtui_print_normal]));

    if((ctrl_type != NULL) && (*ctrl_type != 0)) {
        amxtui_ctrl_new(&widget->ctrl, ctrl_type);
        when_null(widget->ctrl, exit);
        amxp_slot_connect(&widget->ctrl->sigmngr,
                          CTRL_SIG_REDRAW,
                          NULL,
                          amxtui_widget_slot_redraw,
                          widget);
        amxp_slot_connect(&widget->ctrl->sigmngr,
                          CTRL_SIG_RESET,
                          NULL,
                          amxtui_widget_slot_reset,
                          widget);
        if((widget->ctrl != NULL) && (widget->ctrl->get_data != NULL)) {
            widget->ctrl_data = widget->ctrl->get_data(widget, widget->ctrl);
        }
    }

    rv = 0;

exit:
    return rv;
}

int amxtui_widget_clean(amxtui_widget_t* widget) {
    when_null(widget, exit);

    amxtui_screen_disconnect_widget(widget);
    amxtui_screen_remove_widget(widget);

    amxtui_ctrl_delete(&widget->ctrl);
    amxc_var_delete(&widget->ctrl_data);

    free(widget->title);
    delwin(widget->window);
    amxc_llist_clean(&widget->buttons, amxc_string_list_it_free);

exit:
    return 0;
}

int amxtui_widget_add_widget(amxtui_widget_t* parent,
                             amxtui_widget_t* child) {
    when_null(parent, exit);
    when_null(child, exit);

    amxc_llist_append(&parent->children, &child->it);

exit:
    return 0;
}

int amxtui_widget_redraw(const amxtui_widget_t* widget) {
    int nlines = 0;
    int ncols = 0;
    int line = 0;
    int col = 0;
    int parent_width = amxtui_screen_width();
    int parent_height = amxtui_screen_height();
    int parent_x = 0;
    int parent_y = 0;

    when_true(amxtui_screen_resized(), exit);
    when_null(widget, exit);

    when_false(widget->attrs.is_visible, exit);
    werase(widget->window);

    if(amxtui_widget_is_child(widget)) {
        amxtui_widget_t* parent = amxc_container_of(widget->it.llist,
                                                    amxtui_widget_t,
                                                    children);
        parent_height = amxtui_widget_height(parent);
        parent_width = amxtui_widget_width(parent);
        parent_x = amxtui_widget_screen_pos_x(parent);
        parent_y = amxtui_widget_screen_pos_y(parent);
        if(parent->attrs.has_box) {
            parent_x++;
            parent_y++;
        }
    }

    // recalculate width, height, x, and y
    nlines = amxtui_widget_calculate(widget,
                                     parent_height,
                                     widget->height,
                                     widget->height_type,
                                     direction_vertical);
    ncols = amxtui_widget_calculate(widget,
                                    parent_width,
                                    widget->width,
                                    widget->width_type,
                                    direction_horizontal);
    line = amxtui_widget_calculate(widget,
                                   parent_height,
                                   widget->y,
                                   widget->y_type,
                                   direction_vertical);
    col = amxtui_widget_calculate(widget,
                                  parent_width,
                                  widget->x,
                                  widget->x_type,
                                  direction_horizontal);

    wresize(widget->window, nlines, ncols);
    mvwin(widget->window, line + parent_y, col + parent_x);

    if(widget->attrs.has_focus) {
        wattron(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
    } else {
        wattron(widget->window, COLOR_PAIR(widget->colors[amxtui_print_normal]));
    }

    if(widget->attrs.has_box) {
        box(widget->window, 0, 0);
        if(widget->title != NULL) {
            mvwprintw(widget->window, 0, 2, "%s", widget->title);
        }
    }

    if(widget->attrs.has_focus && widget->attrs.has_box) {
        amxtui_widget_draw_buttons(widget, nlines, ncols);
    }

    if(widget->attrs.has_focus) {
        wattroff(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
    } else {
        wattroff(widget->window, COLOR_PAIR(widget->colors[amxtui_print_normal]));
    }

    curs_set(0); // turn off cursor - if needed the controller will turn it on
    if((widget->ctrl != NULL) && (widget->ctrl->show_content != NULL)) {
        widget->ctrl->show_content(widget, widget->ctrl);
    }

    wrefresh(widget->window);

    amxc_llist_for_each(it, &widget->children) {
        amxtui_widget_t* child = amxc_container_of(it, amxtui_widget_t, it);
        amxtui_widget_redraw(child);
    }

    if((widget != amxtui_screen_get_focused()) &&
       !amxtui_widget_is_child_of(widget, amxtui_screen_get_focused())) {
        amxtui_widget_redraw(amxtui_screen_get_focused());
    }

exit:
    return 0;
}

int amxtui_widget_set_pos(amxtui_widget_t* widget,
                          int32_t x,
                          amxtui_size_pos_type_t x_type,
                          int32_t y,
                          amxtui_size_pos_type_t y_type) {
    when_null(widget, exit);
    when_true(x_type == amxtui_maximized, exit);
    when_true(y_type == amxtui_maximized, exit);

    widget->x = x;
    widget->y = y;
    widget->x_type = x_type;
    widget->y_type = y_type;

exit:
    return 0;
}

int amxtui_widget_set_width(amxtui_widget_t* widget,
                            int32_t width,
                            amxtui_size_pos_type_t width_type) {
    when_null(widget, exit);

    widget->width = width;
    widget->width_type = width_type;

exit:
    return 0;
}

int amxtui_widget_set_height(amxtui_widget_t* widget,
                             int32_t height,
                             amxtui_size_pos_type_t height_type) {
    when_null(widget, exit);

    widget->height = height;
    widget->height_type = height_type;

exit:
    return 0;
}

int amxtui_widget_set_title(amxtui_widget_t* widget,
                            const char* title) {
    when_null(widget, exit);

    free(widget->title);
    widget->title = strdup(title);

exit:
    return 0;
}

int amxtui_widget_set_box(amxtui_widget_t* widget,
                          bool has_box) {
    when_null(widget, exit);

    widget->attrs.has_box = has_box;

exit:
    return 0;

}

int amxtui_widget_set_color(amxtui_widget_t* widget,
                            amxtui_print_type_t type,
                            int32_t color_pair) {
    when_null(widget, exit);
    when_true(type > amxtui_print_data, exit);

    widget->colors[type] = color_pair;

exit:
    return 0;
}

int amxtui_widget_set_ctrl(amxtui_widget_t* widget,
                           amxtui_ctrl_t* ctrl) {
    when_null(widget, exit);

    if(widget->ctrl != NULL) {
        amxp_slot_disconnect_with_priv(&widget->ctrl->sigmngr, NULL, widget);
        amxc_var_delete(&widget->ctrl_data);
        amxtui_ctrl_delete(&widget->ctrl);
    }

    widget->ctrl = ctrl;
    if(widget->ctrl) {
        amxtui_ctrl_take_reference(widget->ctrl);
        amxp_slot_connect(&widget->ctrl->sigmngr,
                          CTRL_SIG_REDRAW,
                          NULL,
                          amxtui_widget_slot_redraw,
                          widget);
        amxp_slot_connect(&widget->ctrl->sigmngr,
                          CTRL_SIG_RESET,
                          NULL,
                          amxtui_widget_slot_reset,
                          widget);
        if(widget->ctrl->get_data != NULL) {
            widget->ctrl_data = widget->ctrl->get_data(widget, widget->ctrl);
        }
    }

exit:
    return 0;
}

int amxtui_widget_set_modal(amxtui_widget_t* widget,
                            bool modal) {
    when_null(widget, exit);

    widget->attrs.is_modal = modal;

exit:
    return 0;
}

amxtui_ctrl_t* amxtui_widget_get_ctrl(const amxtui_widget_t* widget) {
    amxtui_ctrl_t* ctrl = NULL;
    when_null(widget, exit);

    ctrl = widget->ctrl;

exit:
    return ctrl;
}

int amxtui_widget_add_button(amxtui_widget_t* widget,
                             const char* button_txt) {
    int rv = -1;

    when_null(widget, exit);
    when_null(button_txt, exit);

    amxc_llist_for_each(it, &widget->buttons) {
        amxc_string_t* str = amxc_container_of(it, amxc_string_t, it);
        if(strcmp(amxc_string_get(str, 0), button_txt) == 0) {
            goto exit;
        }
    }
    amxc_llist_add_string(&widget->buttons, button_txt);
    rv = 0;

exit:
    return rv;
}

int amxtui_widget_remove_button(amxtui_widget_t* widget,
                                const char* button_txt) {
    int rv = -1;

    when_null(widget, exit);
    when_null(button_txt, exit);

    amxc_llist_for_each(it, &widget->buttons) {
        amxc_string_t* str = amxc_container_of(it, amxc_string_t, it);
        if(strcmp(amxc_string_get(str, 0), button_txt) == 0) {
            amxc_string_delete(&str);
        }
    }

    rv = 0;

exit:
    return rv;
}

int amxtui_widget_print_at(const amxtui_widget_t* widget,
                           int line,
                           int col,
                           amxtui_print_type_t type,
                           const amxc_string_t* txt,
                           size_t str_offset) {
    int rv = -1;
    int nlines = 0;
    int ncols = 0;
    int offset = 0;
    char* print_txt = NULL;
    when_null(widget, exit);

    // get the inner height
    nlines = amxtui_widget_height(widget);
    // get the inner width
    ncols = amxtui_widget_width(widget);

    if(widget->attrs.has_box) {
        // add 1 for the box
        offset = 1;
    }

    col += offset;

    when_true(line >= nlines, exit);
    when_true(col >= ncols, exit);

    print_txt = amxc_string_dup(txt, str_offset, ncols - col);

    if(type == amxtui_print_selected) {
        if(widget->attrs.has_focus) {
            wattron(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
        }
        wattron(widget->window, A_STANDOUT);
    } else if(type == amxtui_print_focused) {
        wattron(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
    } else if(type == amxtui_print_data) {
        wattron(widget->window, COLOR_PAIR(widget->colors[amxtui_print_data]));
    }

    // print the text, inner coordinates start at 1 when a box is drawn
    mvwprintw(widget->window, line + offset, col, "%s", print_txt);
    if(type == amxtui_print_selected) {
        if(widget->attrs.has_focus) {
            wattroff(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
        }
        wattroff(widget->window, A_STANDOUT);
    } else if(type == amxtui_print_focused) {
        wattroff(widget->window, COLOR_PAIR(widget->colors[amxtui_print_focused]));
    } else if(type == amxtui_print_data) {
        wattroff(widget->window, COLOR_PAIR(widget->colors[amxtui_print_data]));
    }

    rv = 0;
    free(print_txt);

exit:
    return rv;
}

int amxtui_widget_print(const amxtui_widget_t* widget,
                        int line,
                        amxtui_alligment_t allign,
                        amxtui_print_type_t type,
                        const amxc_string_t* txt,
                        size_t str_offset) {
    int rv = -1;
    int nlines = 0;
    int ncols = 0;
    int col = 0;
    when_null(widget, exit);

    // get the inner height
    nlines = amxtui_widget_height(widget);
    // get the inner width
    ncols = amxtui_widget_width(widget);

    when_true(line >= nlines, exit);
    switch(allign) {
    case amxtui_allign_left:
        col = 0;
        break;
    case amxtui_allign_right:
        col = ncols - (amxc_string_text_length(txt) - str_offset);
        if(col <= 0) {
            col = 0;
        }
        break;
    case amxtui_allign_center:
        col = ncols - (amxc_string_text_length(txt) - str_offset);
        if(col <= 0) {
            col = 0;
        } else {
            col >>= 1;
        }
        break;
    default:
        // invalid allignment
        goto exit;
        break;
    }

    rv = amxtui_widget_print_at(widget, line, col, type, txt, str_offset);

exit:
    return rv;
}

int amxtui_widget_show_cursor(const amxtui_widget_t* widget,
                              int x,
                              int y) {
    int offset = 0;
    when_null(widget, exit);

    if(widget->attrs.has_box) {
        // add 1 to take box into account
        offset = 1;
    }
    if(widget->attrs.has_focus) {
        curs_set(1);
        wmove(widget->window, y + offset, x + offset);
    } else {
        curs_set(0);
    }

    wrefresh(widget->window);

exit:
    return 0;
}

bool amxtui_widget_is_child(const amxtui_widget_t* widget) {
    bool is_child = false;
    when_null(widget, exit);

    if((widget->it.llist != NULL) &&
       (widget->it.llist != amxtui_screen_children())) {
        is_child = true;
    }

exit:
    return is_child;
}

bool amxtui_widget_is_child_of(const amxtui_widget_t* widget,
                               const amxtui_widget_t* parent) {
    bool is_child = false;
    when_null(widget, exit);
    when_null(parent, exit);

    if(widget->it.llist != NULL) {
        if(widget->it.llist == amxtui_screen_children()) {
            is_child = false;
        } else if(widget->it.llist == &parent->children) {
            is_child = true;
        } else {
            amxtui_widget_t* up = amxc_container_of(widget->it.llist, amxtui_widget_t, children);
            is_child = amxtui_widget_is_child_of(up, parent);
        }
    }

exit:
    return is_child;
}

bool amxtui_widget_has_focus(const amxtui_widget_t* widget) {
    bool focus = false;

    when_null(widget, exit);
    focus = widget->attrs.has_focus;

exit:
    return focus;
}

size_t amxtui_widget_width(const amxtui_widget_t* widget) {
    size_t size = 0;
    int offset = 0;
    int parent_width = amxtui_screen_width();

    when_null(widget, exit);
    if(widget->attrs.has_box) {
        offset = 2;
    }

    if(amxtui_widget_is_child(widget)) {
        amxtui_widget_t* parent = amxc_container_of(widget->it.llist,
                                                    amxtui_widget_t,
                                                    children);
        parent_width = amxtui_widget_width(parent);
    }

    // get the inner width of a widget, subtract 2 for the box
    size = amxtui_widget_calculate(widget,
                                   parent_width,
                                   widget->width,
                                   widget->width_type,
                                   direction_horizontal) - offset;

exit:
    return size;
}

size_t amxtui_widget_height(const amxtui_widget_t* widget) {
    size_t size = 0;
    int offset = 0;
    int parent_height = amxtui_screen_height();

    when_null(widget, exit);
    if(widget->attrs.has_box) {
        offset = 2;
    }

    if(amxtui_widget_is_child(widget)) {
        amxtui_widget_t* parent = amxc_container_of(widget->it.llist,
                                                    amxtui_widget_t,
                                                    children);
        parent_height = amxtui_widget_height(parent);
    }

    // get the inner height of a widget, subtract 2 for the box
    size = amxtui_widget_calculate(widget,
                                   parent_height,
                                   widget->height,
                                   widget->height_type,
                                   direction_vertical) - offset;

exit:
    return size;
}

int amxtui_widget_screen_pos_x(const amxtui_widget_t* widget) {
    int pos = 0;
    int parent_width = amxtui_screen_width();
    int parent_x = 0;

    when_null(widget, exit);

    if(amxtui_widget_is_child(widget)) {
        amxtui_widget_t* parent = amxc_container_of(widget->it.llist,
                                                    amxtui_widget_t,
                                                    children);
        parent_width = amxtui_widget_width(parent);
        parent_x = amxtui_widget_screen_pos_x(parent);
        if(parent->attrs.has_box) {
            parent_x++;
        }
    }

    pos = amxtui_widget_calculate(widget,
                                  parent_width,
                                  widget->x,
                                  widget->x_type,
                                  direction_horizontal);

    pos += parent_x;

exit:
    return pos;
}

int amxtui_widget_screen_pos_y(const amxtui_widget_t* widget) {
    int pos = 0;
    int parent_height = amxtui_screen_height();
    int parent_y = 0;

    when_null(widget, exit);

    if(amxtui_widget_is_child(widget)) {
        amxtui_widget_t* parent = amxc_container_of(widget->it.llist,
                                                    amxtui_widget_t,
                                                    children);
        parent_height = amxtui_widget_height(parent);
        parent_y = amxtui_widget_screen_pos_y(parent);
        if(parent->attrs.has_box) {
            parent_y++;
        }
    }

    pos = amxtui_widget_calculate(widget,
                                  parent_height,
                                  widget->y,
                                  widget->y_type,
                                  direction_vertical);

    pos += parent_y;

exit:
    return pos;
}

int amxtui_widget_connect(const amxtui_widget_t* source_widget,
                          amxtui_widget_t* dest_widget,
                          const char* signal,
                          amxp_slot_fn_t fn) {
    int rv = -1;
    when_null(source_widget, exit);
    when_null(dest_widget, exit);
    when_null(signal, exit);
    when_null(source_widget->ctrl, exit);
    when_false(amxc_llist_it_is_in_list(&source_widget->it), exit);
    when_false(amxc_llist_it_is_in_list(&dest_widget->it), exit);

    rv = amxp_slot_connect(&source_widget->ctrl->sigmngr,
                           signal,
                           NULL,
                           fn,
                           dest_widget);

exit:
    return rv;
}

amxc_var_t* amxtui_widget_ctrl_data(const amxtui_widget_t* widget) {
    amxc_var_t* data = NULL;
    when_null(widget, exit);

    data = widget->ctrl_data;

exit:
    return data;
}
