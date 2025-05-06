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
#include "main.h"

static int wait_for_check_args(int argc, int index) {
    int retval = -1;

    if(argc - index < 1) {
        printf("\n\nInvalid number of arguments\n");
        amxrt_print_usage();
        goto leave;
    }

    retval = 0;

leave:
    return retval;
}

static void wait_for_done(UNUSED const char* const sig_name,
                          UNUSED const amxc_var_t* const d,
                          UNUSED void* const priv) {
    char* object = (char*) priv;
    printf("Wait done - object [%s] is available\n", object);
    amxrt_el_stop();
}

static int wait_for(char* object) {
    int retval = 0;

    printf("Wait until object [%s] is available\n", object);
    retval = amxp_slot_connect(NULL, "wait:done", NULL, wait_for_done, object);
    when_failed(retval, leave);
    retval = amxb_wait_for_object(object);

leave:
    return retval;
}

static void wait_for_set_cmd_line_options(void) {
    amxrt_cmd_line_set_usage_doc("[OPTIONS] <OBJECT PATH>");
    amxrt_cmd_line_options_reset();
    amxrt_cmd_line_add_option(0, 'h', "help", no_argument, "Print usage help and exit", NULL);
    amxrt_cmd_line_add_option(0, 'B', "backend", required_argument, "Loads the shared object as bus backend", "so file");
    amxrt_cmd_line_add_option(0, 'u', "uri", required_argument, "Adds an uri to the list of uris", "uri");
    amxrt_cmd_line_add_option(0, 'A', "no-auto-detect", no_argument, "Do not auto detect unix domain sockets and back-ends", NULL);
}

static int wait_for_init(int argc, char* argv[], int* index) {
    int retval = -1;

    amxrt_new();
    amxc_var_t* config = amxrt_get_config();

    wait_for_set_cmd_line_options();

    retval = amxrt_config_init(argc, argv, index, NULL);
    when_failed(retval, leave);

    // Turn off creation of the pid file
    amxc_var_set(bool, GET_ARG(config, AMXRT_COPT_PID_FILE), false);

    retval = wait_for_check_args(argc, *index);
    when_failed(retval, leave);

    retval = amxrt_connect();

leave:
    return retval;
}

int WAIT_FOR_MAIN(int argc, char* argv[]) {
    int index = 0;
    int retval = wait_for_init(argc, argv, &index);
    when_failed(retval, leave);

    retval = amxrt_el_create();
    when_failed(retval, leave);

    retval = wait_for(argv[index]);
    when_failed(retval, leave);

    amxrt_el_start(); // Will block until amxrt_el_stop is called
    amxp_slot_disconnect_all(wait_for_done);

leave:
    amxrt_stop();
    amxrt_delete();
    return retval;
}