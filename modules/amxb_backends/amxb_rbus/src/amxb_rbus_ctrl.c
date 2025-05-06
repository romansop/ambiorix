/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
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

#include <stdlib.h>
#include <errno.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"

/*
   RBus library can start threads.
   Messages can be received on another thread then the main thread.

   Ambiorix and the ambiorix data model is designed to work in a single
   threaded environment and is event-driven.

   The problem is here when a message is received on any thread that will
   manupulate objects and parameters in the data model conflicts can arrise
   when the data model is modified by multiple threads concurrently.

   To solve the multi-threading concurency problem a queue is created
   for each opened rbus connection. When an rbus message is received it is parsed
   and an rbus-item is added to this queue. The main thread which contains
   the eventloop will empty the queue and handle the items one by one. This ensures
   that everything is handled in the correct order and that there are no race conditions
   or concurency issues.

   One of the biggest issues with multi-threading is that it is not deterministic.
   The order in which threads are executed is not guaranteed. Every memory access
   must be synchronized using mutexes or other locking mechanism as the memory
   is shared by all threads.

   The rbus-item queue itself is protected with a mutex. Any thread can add
   an item, only the main thread will remove an item. This is according the
   producers-consumer concurrency pattern.

   Each time an item is added to the queue, a byte is written to a socket.
   The read end filedescriptor of the socket pair is added the the eventloop.
   While reading data from the read socket, only one single byte is read at a time,
   and an item is removed from the queue.
 */

int amxb_rbus_ctrl_lock(amxb_rbus_t* ctx) {
    // lock the queue
    return pthread_mutex_lock(&ctx->mutex);
}

int amxb_rbus_ctrl_unlock(amxb_rbus_t* ctx) {
    // unlock the queue
    return pthread_mutex_unlock(&ctx->mutex);
}

/*
   as rbus is lacking some common functionality regarding asynchronous I/O
   this function is used to have similar behavior.
   A thread is started to handle the item asynchronously.
   in the thread it is just a blocking call, but from the point of view
   of the main thread it looks asynchronous.
   The thread can be canceled or be waited on with a timeout.
 */
int amxb_rbus_ctrl_start(amxb_rbus_t* ctx,
                         amxb_rbus_item_t* item,
                         amxb_rbus_worker_t fn) {
    int retval = 0;
    amxc_llist_append(&ctx->out, &item->it);
    retval = pthread_create(&item->tid, NULL, fn, item);
    if(retval != 0) {
        amxc_llist_it_take(&item->it);
    }
    return retval;
}

void amxb_rbus_ctrl_wait(amxb_rbus_item_t* item) {
    when_true(item->tid == 0, exit);

    pthread_join(item->tid, NULL);
    item->tid = 0;

exit:
    return;
}

int amxb_rbus_ctrl_read(amxb_rbus_t* ctx, amxb_rbus_item_t** item) {
    char buf[1];
    ssize_t bytes = 0;
    int retval = 0;

    // lock the queue, if items are available it will be removed.
    amxb_rbus_ctrl_lock(ctx);

    // read 1 byte from the read-socket, if data is read an item is removed from the queue
    bytes = read(ctx->socket[0], buf, 1);
    if(bytes == 1) {
        // a bytes was read, so take the first item in the queue
        amxc_llist_it_t* it = amxc_llist_take_first(&ctx->in);
        *item = amxc_container_of(it, amxb_rbus_item_t, it);
    }

    // an error occured or no more data available
    if(bytes <= 0) {
        retval = -1;
    }

    // release the
    amxb_rbus_ctrl_unlock(ctx);
    return retval;
}

int amxb_rbus_ctrl_write(amxb_rbus_t* ctx, amxb_rbus_item_t* item) {
    int retval = -1;
    int write_length = 0;

    // lock the queue, if items are available it will be removed.
    amxb_rbus_ctrl_lock(ctx);

    // write 1 byte to the write-end socket, this will trigger the eventloop.
    write_length = write(ctx->socket[1], "I", 1);
    // failed to write the byte, exit this function
    // C lacks exception handling, so the goto exit is used to throw
    // an exception.
    when_true(write_length != 1, exit);

    // append the item to the queue.
    amxc_llist_append(&ctx->in, &item->it);

exit:
    // unlock the queue.
    amxb_rbus_ctrl_unlock(ctx);
    return retval;
}
