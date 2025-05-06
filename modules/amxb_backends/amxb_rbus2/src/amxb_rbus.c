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

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>

#include <rtmessage/rtLog.h>

#include "amxb_rbus.h"
#include "amxb_rbus_ctrl.h"
#include "amxb_rbus_version.h"

static amxc_var_t* config_opts = NULL;
static amxc_llist_t connections;

static void amxb_rbus_clean_session(UNUSED const char* key, amxc_htable_it_t* it) {
    rbus_session_t* session = amxc_container_of(it, rbus_session_t, hit);
    when_null(session, exit);

    amxd_trans_clean(&session->transaction);
    free(session);

exit:
    return;
}


/*
   An application that loads this backend could provide specific configuration
   options for this backend.
   When a amxc htable variant is given , it will be stored.
   The application should never remove the provided configuration, before
   unloading this back-end.

   The backend can add its default socket uris to the configuration.
   The main application can use these to set-up a connection.
 */
static int amxb_rbus_set_config(amxc_var_t* const configuration) {
    struct stat sb;
    int retval = 0;
    amxc_var_t* uris = GET_ARG(configuration, "uris");
    amxc_var_t* use_amx = GET_ARG(configuration, "use-amx-calls");
    amxc_var_t* translate = GET_ARG(configuration, "translate");

    config_opts = configuration;

    if((configuration != NULL) && (uris == NULL)) {
        uris = amxc_var_add_key(amxc_llist_t, configuration, "uris", NULL);
    }

    // if the provided uris are not of the list type, make it an empty list
    if(amxc_var_type_of(uris) != AMXC_VAR_ID_LIST) {
        amxc_var_set_type(uris, AMXC_VAR_ID_LIST);
    }

    // if the uris list is empty and /tmp/rtrouted exists add the default uri
    if(amxc_llist_is_empty(amxc_var_constcast(amxc_llist_t, uris))) {
        if(stat("/tmp/rtrouted", &sb) == 0) {
            amxc_var_add(cstring_t, uris, "rbus:");
        }
    }

    if(use_amx == NULL) {
        // By default use amx calls when possible.
        // When this is set to false, the rbus api is used, also on
        // ambiorix based data model implementations
        amxc_var_add_key(bool, configuration, "use-amx-calls", true);
    }

    if(translate != NULL) {
        amxb_rbus_translate_set_paths();
    }

    return retval;
}

/*
   make a file descriptor non-blocking.
   A important part of event-driven programming is that all I/O is done
   asynchronously.
 */
static int amxb_rbus_set_non_blocking(int fd) {
    int opt;

    opt = fcntl(fd, F_GETFL);
    opt |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, opt);
}

static void amxb_rbus_logger(UNUSED rbusLogLevel level,
                             UNUSED const char* file,
                             UNUSED int line,
                             UNUSED int threadId,
                             UNUSED char* message) {
    // When using syslog to write log message it can cause failures.
    // For now just drop all messages
    return;
}

int isdot(int c) {
    return (c == '.') ? 1 : 0;
}

int isbraces(int c) {
    return (c == '(' || c == ')') ? 1 : 0;
}

amxc_var_t* amxb_rbus_get_config_option(const char* name) {
    return GET_ARG(config_opts, name);
}

amxb_rbus_t* amxb_rbus_get_ctx(rbusHandle_t handle) {
    amxb_rbus_t* rbus_ctx = NULL;
    amxc_llist_iterate(it, &connections) {
        rbus_ctx = amxc_container_of(it, amxb_rbus_t, it);
        if(rbus_ctx->handle == handle) {
            break;
        }
        rbus_ctx = NULL;
    }

    return rbus_ctx;
}

/*
   set-up a connection to RBus.
   As RBus is not exposing its sockets/filedescriptors, but ambiorix based applications
   are expecting an fd that can be watched with select, poll or any other system
   call that monitors filedescriptors for data availability.
   In that regard RBus is not like any other bus-system that do provide
   the file descriptors of the created connections.
   In this backend a work-arround is defined.
   A socket-pair is created, one end is the read socket, the other is the write socket.
   The read end is added to the event-loop.
   If messages are received from RBus, they are added to a queue and a byte is written
   to the write-end socket. This way the event loop is triggered as it would for
   any other bus system.
 */
void* amxb_rbus_connect(UNUSED const char* host,
                        UNUSED const char* port,
                        UNUSED const char* path,
                        amxp_signal_mngr_t* sigmngr) {
    amxb_rbus_t* amxb_rbus_ctx = NULL;
    int rc = RBUS_ERROR_SUCCESS;
    // fetch the register-name configuration option
    const amxc_var_t* register_name = amxb_rbus_get_config_option("register-name");
    char* name = amxc_var_dyncast(cstring_t, register_name);

    if(name == NULL) {
        // if no register-name option was set, create a default one
        amxc_string_t id;
        amxc_string_init(&id, 0);
        // The default one will be "amx-<PID>"
        amxc_string_setf(&id, "amx-%d", getpid());
        name = amxc_string_take_buffer(&id);
        amxc_var_add_key(cstring_t, config_opts, "register-name", name);
        amxc_string_clean(&id);
    }

    // Check rbus is enabled
    when_false(rbus_checkStatus() == RBUS_ENABLED, exit);

    // allocate an rbus context where all specific rbus data can be stored
    amxb_rbus_ctx = (amxb_rbus_t*) calloc(1, sizeof(amxb_rbus_t));
    when_null(amxb_rbus_ctx, exit);

    /*
       open a connection to rbus.
       It is not possible with this API to pass the socket name or any
       socket parameters, that is why the arguments "host", "port", "path" are unused.
       TODO (optional):
             RBus has support for direct connections between differen applications
             This can be added, but for now only a connection to the RBus daemon
             is supported.
     */
    rc = rbus_open(&amxb_rbus_ctx->handle, name);
    if(rc != RBUS_ERROR_SUCCESS) {
        free(amxb_rbus_ctx);
        amxb_rbus_ctx = NULL;
        goto exit;
    }

    /*
       Connection is open, now create the socket pair so the connection can be
       added to the eventloop, rbus lib doesn't provide any API to get
       direct access to the sockets or to dispatch the messages when received.
     */
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, amxb_rbus_ctx->socket);
    if(rc != 0) {
        /*
           Failed to create the socket pair, close the connection and
           free the allocated structure, use goto exit as throw mechanism
         */
        rbus_close(amxb_rbus_ctx->handle);
        free(amxb_rbus_ctx);
        amxb_rbus_ctx = NULL;
        goto exit;
    }

    // Set the read-end of the socket pair as non-blcoking
    amxb_rbus_set_non_blocking(amxb_rbus_ctx->socket[0]);
    // Initialize the individiual parts of the allocated rbus_ctx data structure.
    pthread_mutex_init(&amxb_rbus_ctx->mutex, NULL);
    amxc_llist_init(&amxb_rbus_ctx->in);
    amxc_llist_init(&amxb_rbus_ctx->out);
    amxc_htable_init(&amxb_rbus_ctx->subscribers, 5);
    amxc_htable_init(&amxb_rbus_ctx->subscriptions, 5);
    amxc_var_init(&amxb_rbus_ctx->ignore);
    amxc_var_set_type(&amxb_rbus_ctx->ignore, AMXC_VAR_ID_HTABLE);
    amxc_htable_init(&amxb_rbus_ctx->sessions, 5);
    amxb_rbus_ctx->tid = pthread_self();
    amxb_rbus_ctx->register_done = false;
    /*
       Store the provided signal manager, this will give the back-end to
       emit received events to the eventloop.
     */
    amxb_rbus_ctx->sigmngr = sigmngr;

    /*
       add this rbus connection to the list of open rbus connections.
       it is the responsibility of the backend to manage its own connections.
     */
    amxc_llist_append(&connections, &amxb_rbus_ctx->it);

exit:
    free(name);
    return amxb_rbus_ctx;
}

int amxb_rbus_disconnect(void* ctx) {
    int retval = 0;
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;

    when_null(amxb_rbus_ctx, exit);
    when_null(amxb_rbus_ctx->handle, exit);

    amxb_rbus_remove_subs(amxb_rbus_ctx);

    // the item lists must be cleaned before desroying the mutex
    amxb_rbus_ctrl_lock(amxb_rbus_ctx);
    amxc_llist_clean(&amxb_rbus_ctx->out, amxb_rbus_clean_item);
    amxc_llist_clean(&amxb_rbus_ctx->in, amxb_rbus_clean_item);
    amxb_rbus_ctrl_unlock(amxb_rbus_ctx);

    if(amxb_rbus_ctx->handle != NULL) {
        // unregister the data model
        amxb_rbus_unregister(amxb_rbus_ctx);
        retval = rbus_close(amxb_rbus_ctx->handle);
    }
    if(amxb_rbus_ctx->socket[0] != -1) {
        close(amxb_rbus_ctx->socket[0]);
    }
    if(amxb_rbus_ctx->socket[1] != -1) {
        close(amxb_rbus_ctx->socket[1]);
    }

    pthread_mutex_destroy(&amxb_rbus_ctx->mutex);

    amxc_htable_clean(&amxb_rbus_ctx->sessions, amxb_rbus_clean_session);
    amxc_htable_clean(&amxb_rbus_ctx->subscriptions, NULL);
    amxc_htable_clean(&amxb_rbus_ctx->subscribers, NULL);
    amxc_var_clean(&amxb_rbus_ctx->ignore);

    amxb_rbus_ctx->handle = NULL;
    amxb_rbus_ctx->socket[0] = -1;
    amxb_rbus_ctx->socket[1] = -1;
    amxb_rbus_ctx->sigmngr = NULL;
    amxc_llist_it_take(&amxb_rbus_ctx->it);

exit:
    return retval;
}

int amxb_rbus_get_fd(void* ctx) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    int fd = -1;

    /*
       input argument validation.
       the provided ctx must be not NULL (it is not possible in C to check if
       a pointer is valid or not). The rbus handle should also not be NULL.
       As C doesn't provide exception handling, use goto as throw mechanism
     */
    when_null(amxb_rbus_ctx, exit);
    when_null(amxb_rbus_ctx->handle, exit);

    // return the read-end of the socket pair.
    fd = amxb_rbus_ctx->socket[0];

exit:
    return fd;
}

int amxb_rbus_read(void* ctx) {
    int retval = 0;
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;
    amxb_rbus_item_t* item = NULL;

    /*
       input argument validation.
       the provided ctx must be not NULL (it is not possible in C to check if
       a pointer is valid or not). The rbus handle should also not be NULL.
       As C doesn't provide exception handling, use goto as throw mechanism
     */
    when_null(amxb_rbus_ctx, exit);
    when_null(amxb_rbus_ctx->handle, exit);

    /*
       Check if an item is available.
       If a byte can be read fron the read-end socket of the socket pair and
       an item can be fetched from the queue, the next call will fill the
       item pointer.
     */
    retval = amxb_rbus_ctrl_read(amxb_rbus_ctx, &item);
    when_failed(retval, exit);
    when_null(item, exit);

    if(item->tid != 0) {
        // if the item contains a thread id, wait for the thread to finish
        amxb_rbus_ctrl_wait(item);
    }

    if((item->handler != NULL) && (item->state == AMXB_RBUS_ITEM_OK)) {
        // only accept items for which the state is OK.
        // all others items are dropped and freed.
        item->handler(amxb_rbus_ctx, item);
    }

    // safe to clean-up
    amxb_rbus_item_free(item);

exit:
    return retval;
}

void amxb_rbus_free(void* ctx) {
    amxb_rbus_t* amxb_rbus_ctx = (amxb_rbus_t*) ctx;

    when_null(amxb_rbus_ctx, exit);
    free(amxb_rbus_ctx);

exit:
    return;
}

static amxb_be_funcs_t amxb_rbus_impl = {
    .it = { .ait = NULL, .key = NULL, .next = NULL },
    .handle = NULL,
    .connections = { .head = NULL, .tail = NULL },
    .name = "rbus",
    .size = sizeof(amxb_be_funcs_t),
    .connect = amxb_rbus_connect,
    .disconnect = amxb_rbus_disconnect,
    .get_fd = amxb_rbus_get_fd,
    .read = amxb_rbus_read,
    .new_invoke = NULL,
    .free_invoke = NULL,
    .invoke = amxb_rbus_invoke,
    .async_invoke = amxb_rbus_async_invoke,
    .close_request = amxb_rbus_close_request,
    .wait_request = amxb_rbus_wait_request,
    .subscribe = amxb_rbus_subscribe,
    .unsubscribe = amxb_rbus_unsubscribe,
    .free = amxb_rbus_free,
    .register_dm = amxb_rbus_register,
    .get = amxb_rbus_get,
    .set = amxb_rbus_set,
    .add = amxb_rbus_add,
    .del = amxb_rbus_del,
    .get_supported = amxb_rbus_gsdm,
    .set_config = amxb_rbus_set_config,
    .describe = amxb_rbus_describe,
    .list = amxb_rbus_list,
    .listen = NULL,
    .accept = NULL,
    .read_raw = NULL,
    .wait_for = amxb_rbus_wait_for,
    .capabilities = amxb_rbus_capabilites,
    .has = amxb_rbus_has,
    .get_instances = amxb_rbus_get_instances,
    // This line can be added when libmaxb v4.8.0 or higher is used
    // When adding the get_filtered, the min_lib_version must be set to 4.8.0
    //.get_filtered = amxb_rbus_get_filtered,
};

static amxb_version_t sup_min_lib_version = {
    .major = 4,
    .minor = 7,
    .build = 0
};

static amxb_version_t sup_max_lib_version = {
    .major = 4,
    .minor = -1,
    .build = -1
};

static amxb_version_t rbus_be_version = {
    .major = AMXB_RBUS_VERSION_MAJOR,
    .minor = AMXB_RBUS_VERSION_MINOR,
    .build = AMXB_RBUS_VERSION_BUILD,
};

amxb_be_info_t amxb_rbus_be_info = {
    .min_supported = &sup_min_lib_version,
    .max_supported = &sup_max_lib_version,
    .be_version = &rbus_be_version,
    .name = "rbus",
    .description = "AMXB Backend for RBUS (RDK-B)",
    .funcs = &amxb_rbus_impl,
};

amxb_be_info_t* amxb_be_info(void) {
    return &amxb_rbus_be_info;
}


CONSTRUCTOR static void amxb_rbus_set_logger(void) {
    /*
       Add an rbus log handler, otherwise rbus lib is spamming messages
       to the stdout/stderr
       The logger callback is not per connection but global for librbus.
       So set it when the backend is loaded in this constructor function.
       And removed it using a destructor function when the backend is removed.
     */
    rbus_registerLogHandler(amxb_rbus_logger);
    rtLog_SetLevel(RT_LOG_FATAL);
}

DESTRUCTOR static void amxb_rbus_reset_logger(void) {
    /*
       reset the rbus logger when the back-end is unloaded.
       Issue: rbus doesn't accept a NULL pointer as valid log handler
              If rbus is calling the logcallback after this back-end is removed
              from memory (unloaded) a segmentation fault will occur.
     */
    rbus_registerLogHandler(NULL);
}