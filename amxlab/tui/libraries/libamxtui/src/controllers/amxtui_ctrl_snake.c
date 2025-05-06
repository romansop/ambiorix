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
#include <amxtui/ctrl/amxtui_ctrl_snake.h>

#define CTRL_NAME        "snake"

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN
} direction_type_t;

typedef struct _point {
    int x;
    int y;
} point_t;

typedef struct _amxtui_ctrl_snake {
    int x;
    int y;
    int nextx;
    int nexty;
    int tail_length;
    int score;
    bool game_over;
    direction_type_t current_dir;
    point_t snake_parts[255];
    point_t food;
    amxp_timer_t* update_timer;
    amxtui_ctrl_t ctrl;
} amxtui_ctrl_snake_t;

static void create_food(amxtui_ctrl_snake_t* sctrl,
                        int max_x,
                        int max_y) {
    //Food.x is a random int between 10 and maxX - 10
    sctrl->food.x = (rand() % (max_x - 20)) + 10;

    //Food.y is a random int between 5 and maxY - 5
    sctrl->food.y = (rand() % (max_y - 10)) + 5;
}

static void shift_snake(amxtui_ctrl_snake_t* sctrl) {
    point_t tmp = sctrl->snake_parts[sctrl->tail_length - 1];

    for(int i = sctrl->tail_length - 1; i > 0; i--) {
        sctrl->snake_parts[i] = sctrl->snake_parts[i - 1];
    }

    sctrl->snake_parts[0] = tmp;
}

static void update_score(amxtui_widget_t* widget,
                         amxtui_ctrl_snake_t* sctrl) {
    amxc_string_t txt;
    amxc_string_init(&txt, 0);
    amxc_string_setf(&txt, "Score : %i", sctrl->score);
    amxtui_widget_set_title(widget, amxc_string_get(&txt, 0));
    amxc_string_clean(&txt);
}

static void snake_update(amxp_timer_t* timer, void* priv) {
    amxtui_widget_t* widget = (amxtui_widget_t*) priv;
    amxtui_ctrl_snake_t* sctrl = NULL;
    amxtui_ctrl_t* ctrl = amxtui_widget_get_ctrl(widget);
    int next_x = 0;
    int next_y = 0;
    int max_y = amxtui_widget_height(widget);
    int max_x = amxtui_widget_width(widget);

    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    sctrl = amxc_container_of(ctrl, amxtui_ctrl_snake_t, ctrl);

    if(sctrl->game_over) {
        amxp_timer_stop(sctrl->update_timer);
        amxtui_ctrl_emit_data(&sctrl->ctrl, CTRL_SIG_S_GAME_OVER, NULL);
        return;
    }

    next_x = sctrl->snake_parts[0].x;
    next_y = sctrl->snake_parts[0].y;

    switch(sctrl->current_dir) {
    case RIGHT: next_x++; break;
    case LEFT: next_x--; break;
    case UP: next_y--; break;
    case DOWN: next_y++; break;
    }

    if((next_x == sctrl->food.x) && (next_y == sctrl->food.y)) {
        sctrl->snake_parts[sctrl->tail_length].x = next_x;
        sctrl->snake_parts[sctrl->tail_length].y = next_y;
        sctrl->score += 5;
        update_score(widget, sctrl);
        if(sctrl->tail_length < 255) {
            sctrl->tail_length++;
        }
        create_food(sctrl, max_x, max_y);
    } else {
        for(int i = 0; i < sctrl->tail_length; i++) {
            if((next_x == sctrl->snake_parts[i].x) && (next_y == sctrl->snake_parts[i].y)) {
                amxp_timer_set_interval(timer, 0);
                amxp_timer_start(timer, 2000);
                sctrl->game_over = true;
                break;
            }
        }

        //We are going to set the tail as the new head
        sctrl->snake_parts[sctrl->tail_length - 1].x = next_x;
        sctrl->snake_parts[sctrl->tail_length - 1].y = next_y;
    }

    shift_snake(sctrl);

    if(((next_x >= max_x) || (next_x < 0)) || ((next_y >= max_y) || (next_y < 0))) {
        amxp_timer_set_interval(timer, 0);
        amxp_timer_start(timer, 2000);
        sctrl->game_over = true;
    }

    amxtui_widget_redraw(widget);
}

static void amxtui_ctrl_snake_show(const amxtui_widget_t* widget,
                                   amxtui_ctrl_t* ctrl) {
    amxtui_ctrl_snake_t* sctrl = NULL;
    amxc_string_t txt;

    amxc_string_init(&txt, 1);
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    sctrl = amxc_container_of(ctrl, amxtui_ctrl_snake_t, ctrl);

    amxc_string_set(&txt, "o");
    for(int i = 0; i < sctrl->tail_length; i++) {
        amxtui_widget_print_at(widget,
                               sctrl->snake_parts[i].y, sctrl->snake_parts[i].x,
                               amxtui_print_normal, &txt, 0);
    }

    //Draw the current food
    amxtui_widget_print_at(widget,
                           sctrl->food.y, sctrl->food.x,
                           amxtui_print_normal, &txt, 0);
    amxc_string_clean(&txt);
}

static bool amxtui_ctrl_snake_handle_ctrl_key(UNUSED amxtui_widget_t* widget,
                                              amxtui_ctrl_t* ctrl,
                                              uint32_t ctrl_key) {
    bool handled = true;
    amxtui_ctrl_snake_t* sctrl = NULL;
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    sctrl = amxc_container_of(ctrl, amxtui_ctrl_snake_t, ctrl);

    switch(ctrl_key) {
    case amxt_key_up:
        if(sctrl->current_dir != DOWN) {
            sctrl->current_dir = UP;
        }
        break;
    case amxt_key_down:
        if(sctrl->current_dir != UP) {
            sctrl->current_dir = DOWN;
        }
        break;
    case amxt_key_left:
        if(sctrl->current_dir != RIGHT) {
            sctrl->current_dir = LEFT;
        }
        break;
    case amxt_key_right:
        if(sctrl->current_dir != LEFT) {
            sctrl->current_dir = RIGHT;
        }
        break;
    default:
        handled = false;
        break;
    }

    return handled;
}

static int amxtui_ctrl_new_snake(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_snake_t* sctrl = NULL;
    when_null(ctrl, exit);

    sctrl = (amxtui_ctrl_snake_t*) calloc(1, sizeof(amxtui_ctrl_snake_t));
    amxtui_ctrl_init(&sctrl->ctrl,
                     amxtui_ctrl_snake_show,
                     amxtui_ctrl_snake_handle_ctrl_key,
                     NULL,
                     NULL);

    amxtui_ctrl_add_signal(&sctrl->ctrl, CTRL_SIG_S_GAME_OVER);

    *ctrl = &sctrl->ctrl;
exit:
    return 0;
}

static int amxtui_ctrl_delete_snake(amxtui_ctrl_t** ctrl) {
    amxtui_ctrl_snake_t* sctrl = NULL;
    when_null(ctrl, exit);
    when_null(*ctrl, exit);
    when_false(strcmp(amxtui_ctrl_type_name(*ctrl), CTRL_NAME) == 0, exit);

    sctrl = amxc_container_of((*ctrl), amxtui_ctrl_snake_t, ctrl);
    amxp_timer_delete(&sctrl->update_timer);
    amxtui_ctrl_clean(&sctrl->ctrl);
    free(sctrl);

exit:
    return 0;
}

static amxtui_ctrl_type_t amxtui_ctrl_snake = {
    .hit = { NULL, NULL, NULL },
    .ctor = amxtui_ctrl_new_snake,
    .dtor = amxtui_ctrl_delete_snake
};

CONSTRUCTOR_LVL(102) static void amxtui_ctrl_types_init(void) {
    srand(time(NULL));
    amxtui_ctrl_type_register(&amxtui_ctrl_snake, CTRL_NAME);
}

void amxtui_snake_widget_start(amxtui_widget_t* widget) {
    amxtui_ctrl_snake_t* sctrl = NULL;
    amxtui_ctrl_t* ctrl = NULL;
    int max_y = 0;
    int max_x = 0;
    int j = 0;

    when_null(widget, exit);
    ctrl = amxtui_widget_get_ctrl(widget);
    max_y = amxtui_widget_height(widget);
    max_x = amxtui_widget_width(widget);
    when_null(ctrl, exit);
    when_false(amxtui_ctrl_is_type(ctrl, CTRL_NAME), exit);
    ctrl = amxtui_ctrl_get_type(ctrl, CTRL_NAME);
    sctrl = amxc_container_of(ctrl, amxtui_ctrl_snake_t, ctrl);

    sctrl->tail_length = 5;
    sctrl->current_dir = RIGHT;
    sctrl->tail_length = 5;
    sctrl->score = 0;
    sctrl->game_over = false;

    amxtui_widget_set_title(widget, "Score : 0");

    for(int i = sctrl->tail_length; i >= 0; i--) {
        point_t curr_point;

        curr_point.x = i;
        curr_point.y = max_y / 2; //Start mid widget on the y axis

        sctrl->snake_parts[j] = curr_point;
        j++;
    }

    create_food(sctrl, max_x, max_y);

    amxp_timer_new(&sctrl->update_timer, snake_update, widget);
    amxp_timer_set_interval(sctrl->update_timer, 100);
    amxp_timer_start(sctrl->update_timer, 1000);

exit:
    return;
}