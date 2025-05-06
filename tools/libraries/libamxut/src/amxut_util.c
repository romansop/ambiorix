/****************************************************************************
**
** SPDX-License-Identifier: BSD-2-Clause-Patent
**
** SPDX-FileCopyrightText: Copyright (c) 2024 SoftAtHome
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
** 1. Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above
** copyright notice, this list of conditions and the following
** disclaimer in the documentation and/or other materials provided
** with the distribution.
**
** Subject to the terms and conditions of this license, each
** copyright holder and contributor hereby grants to those receiving
** rights under this license a perpetual, worldwide, non-exclusive,
** no-charge, royalty-free, irrevocable (except for failure to
** satisfy the conditions of this license) patent license to make,
** have made, use, offer to sell, sell, import, and otherwise
** transfer this software, where such license applies only to those
** patent claims, already acquired or hereafter acquired, licensable
** by such copyright holder or contributor that are necessarily
** infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright
** holders and non-copyrightable additions of contributors, in
** source or binary form) alone; or
**
** (b) combination of their Contribution(s) with the work of
** authorship to which such Contribution(s) was added by such
** copyright holder or contributor, if, at the time the Contribution
** is added, such addition causes such combination to be necessarily
** infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any
** copyright holder or contributor is granted under this license,
** whether expressly, by implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
** CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
** USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
** AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
** ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "amxut/amxut_util.h"
#include <amxc/amxc.h>
#include <stdarg.h> // needed for cmocka
#include <setjmp.h> // needed for cmocka
#include <unistd.h> // needed for cmocka
#include <cmocka.h>
#include <yajl/yajl_gen.h>
#include <amxp/amxp.h>
#include <amxj/amxj_variant.h>
#include <errno.h>
#include <fcntl.h>


amxc_var_t* amxut_util_read_json_from_file(const char* filename) {
    int fd = -1;
    variant_json_t* reader = NULL;
    amxc_var_t* data = NULL;

    // create a json reader
    if(amxj_reader_new(&reader) != 0) {
        fail_msg("Failed to create json file reader");
    }

    // open the json file
    fd = open(filename, O_RDONLY);
    if(fd == -1) {
        fail_msg("File open file %s - error 0x%8.8X", filename, errno);
    }

    // read the json file and parse the json text
    amxj_read(reader, fd);

    // get the variant
    data = amxj_reader_result(reader);

    // delete the reader and close the file
    amxj_reader_delete(&reader);
    close(fd);

    if(data == NULL) {
        fail_msg("Invalid JSON in file %s", filename);
    }

    return data;
}

void amxut_util_write_to_json_file(const amxc_var_t* const var, const char* filename) {
    variant_json_t* writer = NULL;
    int retval = -1;
    int fd = -1;

    retval = amxj_writer_new(&writer, var);
    if(retval != 0) {
        fail_msg("Failed to create json file writer, retval = %d", retval);
    }

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd == -1) {
        fail_msg("File open file %s - error 0x%8.8X", filename, errno);
    }

    retval = amxj_write(writer, fd);
    if(retval <= 0) {
        fail_msg("Failed to write to file, retval = %d", retval);
    }

    amxj_writer_delete(&writer);
    close(fd);
}
