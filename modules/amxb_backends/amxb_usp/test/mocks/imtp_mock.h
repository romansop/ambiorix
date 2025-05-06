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

#ifndef __TEST_IMTP_MOCK_H__
#define __TEST_IMTP_MOCK_H__

#include <imtp/imtp_connection.h>

typedef enum _usp_type {
    usp_type_get,
    usp_type_get_resp,
    usp_type_set,
    usp_type_set_resp,
    usp_type_add,
    usp_type_add_resp,
    usp_type_delete,
    usp_type_delete_resp,
    usp_type_get_supported_dm,
    usp_type_get_supported_dm_resp,
    usp_type_operate,
    usp_type_operate_resp,
    usp_type_get_instances,
    usp_type_get_instances_resp,
    usp_type_handshake,
    usp_type_max
} usp_type_t;

struct _imtp_connection {
    int fd;
    struct sockaddr_un* addr_self;
    struct sockaddr_un* addr_other;
    bool (* imtp_connection_accept_cb)(struct _imtp_connection*);
    uint32_t flags;
    amxc_llist_it_t it;
    uint8_t* buffer;
    ssize_t read_len;
};

int __wrap_imtp_connection_connect(imtp_connection_t** icon, char* from_uri, char* to_uri);
int __wrap_imtp_connection_listen(imtp_connection_t** icon, char* uri, imtp_connection_accept_cb_t fn);
int __wrap_imtp_connection_accept(imtp_connection_t* icon);
imtp_connection_t* __wrap_imtp_connection_get_con(int fd);
void __wrap_imtp_connection_delete(imtp_connection_t** icon);
int __wrap_imtp_connection_read_frame(imtp_connection_t* icon, imtp_frame_t** frame);
int __wrap_imtp_connection_write_frame(imtp_connection_t* icon, imtp_frame_t* frame);

void imtp_mock_free_msg_id(void);
char* imtp_mock_get_msg_id(void);

#endif // __TEST_IMTP_MOCK_H__
