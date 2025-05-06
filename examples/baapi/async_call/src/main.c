/****************************************************************************
**
** - DISCLAIMER OF WARRANTY -
**
** THIS FILE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
** EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE SOURCE
** CODE IS WITH YOU. SHOULD THE SOURCE CODE PROVE DEFECTIVE, YOU
** ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
**
** - LIMITATION OF LIABILITY -
**
** IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN
** WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES
** AND/OR DISTRIBUTES THE SOURCE CODE, BE LIABLE TO YOU FOR DAMAGES,
** INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES
** ARISING OUT OF THE USE OR INABILITY TO USE THE SOURCE CODE
** (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED
** INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE
** OF THE SOURCE CODE TO OPERATE WITH ANY OTHER PROGRAM), EVEN IF SUCH
** HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGES.
**
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <amxrt/amxrt.h>
#include <yajl/yajl_gen.h>
#include <amxj/amxj_variant.h>

#include "main.h"

static int check_args(int argc, int index) {
    int retval = -1;

    if(argc - index < 2) {
        printf("\n\nInvalid number of arguments\n");
        amxrt_print_usage();
        goto leave;
    }

    retval = 0;

leave:
    return retval;
}

static void print_extended_help(void) {
    printf("\n");
    printf("OBJECT PATH : full path to a data model object\n");
    printf("METHOD      : RPC method that needs to be called\n");
    printf("ARGS        : RPC method in arguments in JSON format\n");
    printf("\n");
    printf("Example :\n");
    printf("amx-async-call Greeter. say '{\"from\":\"me\", \"message\":\"Hello World\"}'\n");
}

static int handle_cmd_line_arg(amxc_var_t* config,
                               int arg_id,
                               UNUSED const char* value) {
    int rv = -1;
    switch(arg_id) {
    case 'H': {
        amxc_var_t* dump_config = GET_ARG(config, AMXRT_COPT_DUMP_CONFIG);
        amxc_var_set(bool, dump_config, false);
        amxrt_print_usage();
        print_extended_help();
    }
    break;
    default:
        rv = -2;
        break;
    }

    return rv;
}

static void call_done(UNUSED const amxb_bus_ctx_t* bus_ctx,
                      amxb_request_t* req,
                      int status,
                      UNUSED void* priv) {
    printf("Function call done status = %d\n", status);
    printf("Function return value = \n");
    fflush(stdout);
    amxc_var_dump(GETI_ARG(req->result, 0), STDOUT_FILENO);
    printf("Function out args = \n");
    fflush(stdout);
    amxc_var_dump(GETI_ARG(req->result, 1), STDOUT_FILENO);
    amxb_close_request(&req);
    amxrt_el_stop();
}

static int async_call(const char* object_path, const char* method, amxc_var_t* args) {
    int retval = -1;
    amxb_request_t* request = 0;
    amxb_bus_ctx_t* bus_ctx = amxb_be_who_has(object_path);
    if(bus_ctx == NULL) {
        printf("Object [%s] not found\n", object_path);
        goto leave;
    }

    request = amxb_async_call(bus_ctx, object_path, method, args, call_done, NULL);
    if(request == NULL) {
        printf("Failed to call [%s] for [%s]\n", method, object_path);
        goto leave;
    }

    retval = 0;

leave:
    return retval;
}

int ASYNC_CALL_MAIN(int argc, char* argv[]) {
    int retval = 0;
    int index = 0;
    amxc_var_t* config = NULL;
    amxc_var_t args;

    amxrt_new();

    config = amxrt_get_config();
    amxc_var_init(&args);

    // set the usage doc
    amxrt_cmd_line_set_usage_doc("[OPTIONS] <OBJECT PATH> <METHOD> [<ARGS>]");
    // remove all default options and re-add some of them
    amxrt_cmd_line_options_reset();
    amxrt_cmd_line_add_option(0, 'h', "help", no_argument, "Print usage help and exit", NULL);
    amxrt_cmd_line_add_option(0, 'H', "HELP", no_argument, "Print extended help and exit", NULL);
    amxrt_cmd_line_add_option(0, 'B', "backend", required_argument, "Loads the shared object as bus backend", "so file");
    amxrt_cmd_line_add_option(0, 'u', "uri", required_argument, "Adds an uri to the list of uris", "uri");
    amxrt_cmd_line_add_option(0, 'A', "no-auto-detect", no_argument, "Do not auto detect unix domain sockets and back-ends", NULL);

    retval = amxrt_config_init(argc, argv, &index, handle_cmd_line_arg);
    when_failed(retval, leave);

    // turn off creation of the pid file
    amxc_var_set(bool, GET_ARG(config, AMXRT_COPT_PID_FILE), false);

    retval = check_args(argc, index);
    when_failed(retval, leave);

    if(index + 2 < argc) {
        retval = amxc_var_set(jstring_t, &args, argv[index + 2]);
        when_failed(retval, leave);
        amxc_var_cast(&args, AMXC_VAR_ID_ANY);
    }

    retval = amxrt_connect();
    when_failed(retval, leave);

    retval = amxrt_el_create();
    when_failed(retval, leave);

    retval = async_call(argv[index], argv[index + 1], &args);
    when_failed(retval, leave);

    // to recieve the reply, an event loop is needed.
    retval = amxrt_el_start(); // will block until amxrt_el_stop is called

leave:
    amxc_var_clean(&args);
    amxrt_stop();
    amxrt_delete();
    return retval;
}
