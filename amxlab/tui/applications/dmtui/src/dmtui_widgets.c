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

#include "dmtui.h"

void dmtui_widgets_create(dmtui_app_t* app) {
    // the left widget will contain the data model tree
    amxtui_widget_new(&app->left,
                      "dm-browser",
                      50, amxtui_percentage,
                      -3, amxtui_absolute);

    // the right widget contains two child widgets, doesn't need a controller
    amxtui_widget_new(&app->right,
                      NULL,
                      50, amxtui_percentage,
                      -3, amxtui_absolute);
    amxtui_widget_set_box(app->right, false); // don't draw a box

    amxtui_widget_new(&app->right_top,
                      "variant-viewer", 100,
                      amxtui_percentage, 50, amxtui_percentage);
    amxtui_ctrl_vv_editable(amxtui_widget_get_ctrl(app->right_top), false);

    amxtui_widget_new(&app->right_bottom,
                      "list-box",
                      100, amxtui_percentage,
                      50, amxtui_percentage);
    app->ctrl_right_bottom = amxtui_widget_get_ctrl(app->right_bottom);
    amxtui_ctrl_take_reference(app->ctrl_right_bottom);
    amxtui_ctrl_new(&app->ctrl_snake, "snake");
    amxtui_widget_add_widget(app->right, app->right_top);
    amxtui_widget_add_widget(app->right, app->right_bottom);

    // the bottom widget is an text input box
    amxtui_widget_new(&app->bottom,
                      "input-box",
                      100, amxtui_percentage,
                      3, amxtui_absolute);

    amxtui_widget_set_title(app->left, "Data Model Tree");
    amxtui_widget_set_title(app->right_top, "Meta Information");
    amxtui_widget_set_title(app->right_bottom, "Search Result");

    amxtui_widget_set_pos(app->left,
                          amxtui_allign_left, amxtui_alligned,
                          amxtui_allign_top, amxtui_alligned);
    amxtui_widget_set_pos(app->right,
                          amxtui_allign_right, amxtui_alligned,
                          amxtui_allign_top, amxtui_alligned);
    amxtui_widget_set_pos(app->right_top,
                          amxtui_allign_right, amxtui_alligned,
                          amxtui_allign_top, amxtui_alligned);
    amxtui_widget_set_pos(app->right_bottom,
                          amxtui_allign_right, amxtui_alligned,
                          amxtui_allign_bottom, amxtui_alligned);
    amxtui_widget_set_pos(app->bottom,
                          0, amxtui_absolute,
                          amxtui_allign_bottom, amxtui_alligned);

    amxtui_screen_add_widget(app->left);
    amxtui_screen_add_widget(app->right);
    amxtui_screen_add_widget(app->bottom);
}

void dmtui_widgets_delete(dmtui_app_t* app) {
    amxtui_ctrl_delete(&app->ctrl_right_bottom);
    amxtui_ctrl_delete(&app->ctrl_snake);
    amxtui_widget_delete(&app->bottom);
    amxtui_widget_delete(&app->left);
    amxtui_widget_delete(&app->right_top);
    amxtui_widget_delete(&app->right_bottom);
    amxtui_widget_delete(&app->right);
}