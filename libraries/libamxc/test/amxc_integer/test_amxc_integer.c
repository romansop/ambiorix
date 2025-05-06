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

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <cmocka.h>

#include <amxc/amxc_integer.h>

#include "test_amxc_integer.h"

#include <amxc/amxc_macros.h>

/* Kept as a macro to preserve source line location in case of error. */
#define ASSERT_FROM_TYPE(type, value, expected) \
    do {                                                    \
        char* strvalue = amxc_ ## type ## _to_str(value);   \
        assert_non_null(expected);                          \
        assert_non_null(strvalue);                          \
        assert_string_equal(strvalue, expected);            \
        free(strvalue);                                     \
    } while(0)

static void s_test_uint_buf_part(int bits, uint64_t value, const char* expected, int fill) {
    size_t offset = 10;
    char buf[24 + AMXC_INTEGER_UINT64_MAX_DIGITS];
    char cmp[sizeof(buf)];
    char* out;
    size_t len;

    assert_non_null(expected);
    len = strlen(expected);

    assert_true(len < sizeof(buf) - offset * 2);

    memset(buf, fill, sizeof(buf));
    memset(cmp, fill, sizeof(cmp));

    // WHEN a valid buffer is given
    switch(bits) {
    case 8:
        out = amxc_uint8_to_buf(value, buf + offset);
        break;
    case 16:
        out = amxc_uint16_to_buf(value, buf + offset);
        break;
    case 32:
        out = amxc_uint32_to_buf(value, buf + offset);
        break;
    case 64:
        out = amxc_uint64_to_buf(value, buf + offset);
        break;
    default:
        out = NULL;
        print_error("Invalid number of bits %d set, expected 8, 16, 32 or 64", bits);
        fail();
    }

    // THEN the output is written to that buffer
    assert_non_null(out);
    assert_true(out >= buf + offset);
    assert_true(out <= buf + sizeof(buf));
    assert_true(out == buf + offset + len);
    assert_true(strncmp(expected, buf + offset, len) == 0);

    // AND no memory before or after the written text is touched
    assert_memory_equal(buf, cmp, offset);
    assert_memory_equal(out, cmp + (out - buf), sizeof(buf) - (out - buf));
}

static void s_test_uint_buf(int bits, uint64_t value, const char* expected) {
    s_test_uint_buf_part(bits, value, expected, 0x00);
    s_test_uint_buf_part(bits, value, expected, 0xff);
}

static void s_test_int_buf_part(int bits, int64_t value, const char* expected, int fill) {
    size_t offset = 10;
    char buf[24 + AMXC_INTEGER_INT64_MAX_DIGITS];
    char cmp[sizeof(buf)];
    char* out;
    size_t len;

    assert_non_null(expected);
    len = strlen(expected);

    assert_true(len < sizeof(buf) - offset * 2);

    memset(buf, fill, sizeof(buf));
    memset(cmp, fill, sizeof(cmp));

    // WHEN a valid buffer is given
    switch(bits) {
    case 8:
        out = amxc_int8_to_buf(value, buf + offset);
        break;
    case 16:
        out = amxc_int16_to_buf(value, buf + offset);
        break;
    case 32:
        out = amxc_int32_to_buf(value, buf + offset);
        break;
    case 64:
        out = amxc_int64_to_buf(value, buf + offset);
        break;
    default:
        out = NULL;
        print_error("Invalid number of bits %d set, expected 8, 16, 32 or 64", bits);
        fail();
    }

    // THEN the output is written to that buffer
    assert_non_null(out);
    assert_true(out >= buf + offset);
    assert_true(out <= buf + sizeof(buf));
    assert_true(out == buf + offset + len);
    assert_true(strncmp(expected, buf + offset, len) == 0);

    // AND no memory before or after the written text is touched
    assert_memory_equal(buf, cmp, offset);
    assert_memory_equal(out, cmp + (out - buf), sizeof(buf) - (out - buf));
}

static void s_test_int_buf(int bits, int64_t value, const char* expected) {
    s_test_int_buf_part(bits, value, expected, 0x00);
    s_test_int_buf_part(bits, value, expected, 0xff);
}

void test_amxc_integer_digit_count(UNUSED void** state) {
    /* Subtract 1 for the positive values because the MAX_DIGITS accounts for the '-' sign. */
    assert_true(snprintf(NULL, 0, "%" PRId8, INT8_MIN) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId8, INT8_MIN), AMXC_INTEGER_INT8_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRId8, INT8_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId8, INT8_MAX), AMXC_INTEGER_INT8_MAX_DIGITS - 1);
    assert_true(snprintf(NULL, 0, "%" PRId16, INT16_MIN) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId16, INT16_MIN), AMXC_INTEGER_INT16_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRId16, INT16_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId16, INT16_MAX), AMXC_INTEGER_INT16_MAX_DIGITS - 1);
    assert_true(snprintf(NULL, 0, "%" PRId32, INT32_MIN) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId32, INT32_MIN), AMXC_INTEGER_INT32_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRId32, INT32_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId32, INT32_MAX), AMXC_INTEGER_INT32_MAX_DIGITS - 1);
    assert_true(snprintf(NULL, 0, "%" PRId64, INT64_MIN) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId64, INT64_MIN), AMXC_INTEGER_INT64_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRId64, INT64_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRId64, INT64_MAX), AMXC_INTEGER_INT64_MAX_DIGITS - 1);

    assert_true(snprintf(NULL, 0, "%" PRIu8, UINT8_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRIu8, UINT8_MAX), AMXC_INTEGER_UINT8_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRIu16, UINT16_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRIu16, UINT16_MAX), AMXC_INTEGER_UINT16_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRIu32, UINT32_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRIu32, UINT32_MAX), AMXC_INTEGER_UINT32_MAX_DIGITS);
    assert_true(snprintf(NULL, 0, "%" PRIu64, UINT64_MAX) > 0);
    assert_int_equal(snprintf(NULL, 0, "%" PRIu64, UINT64_MAX), AMXC_INTEGER_UINT64_MAX_DIGITS);
}

void test_amxc_integer_uint_to_buf(UNUSED void** state) {
    char buf[AMXC_INTEGER_INT64_MAX_DIGITS + 1];

    s_test_uint_buf(8, UINT64_C(0), "0");
    s_test_uint_buf(16, UINT64_C(0), "0");
    s_test_uint_buf(32, UINT64_C(0), "0");
    s_test_uint_buf(64, UINT64_C(0), "0");
    s_test_uint_buf(8, UINT64_C(1), "1");
    s_test_uint_buf(16, UINT64_C(1), "1");
    s_test_uint_buf(32, UINT64_C(1), "1");
    s_test_uint_buf(64, UINT64_C(1), "1");
    s_test_uint_buf(8, UINT64_C(9), "9");
    s_test_uint_buf(16, UINT64_C(9), "9");
    s_test_uint_buf(32, UINT64_C(9), "9");
    s_test_uint_buf(64, UINT64_C(9), "9");
    s_test_uint_buf(8, UINT64_C(9), "9");
    s_test_uint_buf(8, UINT64_C(10), "10");
    s_test_uint_buf(8, UINT64_C(11), "11");
    s_test_uint_buf(16, UINT64_C(9), "9");
    s_test_uint_buf(16, UINT64_C(10), "10");
    s_test_uint_buf(16, UINT64_C(11), "11");
    s_test_uint_buf(32, UINT64_C(9), "9");
    s_test_uint_buf(32, UINT64_C(10), "10");
    s_test_uint_buf(32, UINT64_C(11), "11");
    s_test_uint_buf(64, UINT64_C(9), "9");
    s_test_uint_buf(64, UINT64_C(10), "10");
    s_test_uint_buf(64, UINT64_C(11), "11");
    s_test_uint_buf(8, UINT64_C(99), "99");
    s_test_uint_buf(8, UINT64_C(100), "100");
    s_test_uint_buf(8, UINT64_C(101), "101");
    s_test_uint_buf(16, UINT64_C(99), "99");
    s_test_uint_buf(16, UINT64_C(100), "100");
    s_test_uint_buf(16, UINT64_C(101), "101");
    s_test_uint_buf(32, UINT64_C(99), "99");
    s_test_uint_buf(32, UINT64_C(100), "100");
    s_test_uint_buf(32, UINT64_C(101), "101");
    s_test_uint_buf(64, UINT64_C(99), "99");
    s_test_uint_buf(64, UINT64_C(100), "100");
    s_test_uint_buf(64, UINT64_C(101), "101");

    assert_int_equal(255, UINT8_MAX);
    s_test_uint_buf(8, UINT64_C(255), "255");
    s_test_uint_buf(16, UINT64_C(255), "255");
    s_test_uint_buf(32, UINT64_C(255), "255");
    s_test_uint_buf(64, UINT64_C(255), "255");

    s_test_uint_buf(16, UINT64_C(999), "999");
    s_test_uint_buf(16, UINT64_C(1000), "1000");
    s_test_uint_buf(16, UINT64_C(1001), "1001");
    s_test_uint_buf(32, UINT64_C(999), "999");
    s_test_uint_buf(32, UINT64_C(1000), "1000");
    s_test_uint_buf(32, UINT64_C(1001), "1001");
    s_test_uint_buf(64, UINT64_C(999), "999");
    s_test_uint_buf(64, UINT64_C(1000), "1000");
    s_test_uint_buf(64, UINT64_C(1001), "1001");
    s_test_uint_buf(16, UINT64_C(9999), "9999");
    s_test_uint_buf(16, UINT64_C(10000), "10000");
    s_test_uint_buf(16, UINT64_C(10001), "10001");
    s_test_uint_buf(32, UINT64_C(9999), "9999");
    s_test_uint_buf(32, UINT64_C(10000), "10000");
    s_test_uint_buf(32, UINT64_C(10001), "10001");
    s_test_uint_buf(64, UINT64_C(9999), "9999");
    s_test_uint_buf(64, UINT64_C(10000), "10000");
    s_test_uint_buf(64, UINT64_C(10001), "10001");

    assert_int_equal(65535, UINT16_MAX);
    s_test_uint_buf(16, UINT64_C(65535), "65535");
    s_test_uint_buf(32, UINT64_C(65535), "65535");
    s_test_uint_buf(64, UINT64_C(65535), "65535");

    s_test_uint_buf(32, UINT64_C(99999), "99999");
    s_test_uint_buf(32, UINT64_C(100000), "100000");
    s_test_uint_buf(32, UINT64_C(100001), "100001");
    s_test_uint_buf(64, UINT64_C(99999), "99999");
    s_test_uint_buf(64, UINT64_C(100000), "100000");
    s_test_uint_buf(64, UINT64_C(100001), "100001");
    s_test_uint_buf(32, UINT64_C(999999), "999999");
    s_test_uint_buf(32, UINT64_C(1000000), "1000000");
    s_test_uint_buf(32, UINT64_C(1000001), "1000001");
    s_test_uint_buf(64, UINT64_C(999999), "999999");
    s_test_uint_buf(64, UINT64_C(1000000), "1000000");
    s_test_uint_buf(64, UINT64_C(1000001), "1000001");
    s_test_uint_buf(32, UINT64_C(9999999), "9999999");
    s_test_uint_buf(32, UINT64_C(10000000), "10000000");
    s_test_uint_buf(32, UINT64_C(10000001), "10000001");
    s_test_uint_buf(64, UINT64_C(9999999), "9999999");
    s_test_uint_buf(64, UINT64_C(10000000), "10000000");
    s_test_uint_buf(64, UINT64_C(10000001), "10000001");
    s_test_uint_buf(32, UINT64_C(99999999), "99999999");
    s_test_uint_buf(32, UINT64_C(100000000), "100000000");
    s_test_uint_buf(32, UINT64_C(100000001), "100000001");
    s_test_uint_buf(64, UINT64_C(99999999), "99999999");
    s_test_uint_buf(64, UINT64_C(100000000), "100000000");
    s_test_uint_buf(64, UINT64_C(100000001), "100000001");
    s_test_uint_buf(32, UINT64_C(999999999), "999999999");
    s_test_uint_buf(32, UINT64_C(1000000000), "1000000000");
    s_test_uint_buf(32, UINT64_C(1000000001), "1000000001");
    s_test_uint_buf(64, UINT64_C(999999999), "999999999");
    s_test_uint_buf(64, UINT64_C(1000000000), "1000000000");
    s_test_uint_buf(64, UINT64_C(1000000001), "1000000001");

    assert_int_equal(4294967295, UINT32_MAX);
    s_test_uint_buf(32, UINT64_C(4294967295), "4294967295");
    s_test_uint_buf(64, UINT64_C(4294967295), "4294967295");

    s_test_uint_buf(64, UINT64_C(9999999999), "9999999999");
    s_test_uint_buf(64, UINT64_C(10000000000), "10000000000");
    s_test_uint_buf(64, UINT64_C(10000000001), "10000000001");
    s_test_uint_buf(64, UINT64_C(99999999999), "99999999999");
    s_test_uint_buf(64, UINT64_C(100000000000), "100000000000");
    s_test_uint_buf(64, UINT64_C(100000000001), "100000000001");
    s_test_uint_buf(64, UINT64_C(999999999999), "999999999999");
    s_test_uint_buf(64, UINT64_C(1000000000000), "1000000000000");
    s_test_uint_buf(64, UINT64_C(1000000000001), "1000000000001");
    s_test_uint_buf(64, UINT64_C(9999999999999), "9999999999999");
    s_test_uint_buf(64, UINT64_C(10000000000000), "10000000000000");
    s_test_uint_buf(64, UINT64_C(10000000000001), "10000000000001");
    s_test_uint_buf(64, UINT64_C(99999999999999), "99999999999999");
    s_test_uint_buf(64, UINT64_C(100000000000000), "100000000000000");
    s_test_uint_buf(64, UINT64_C(100000000000001), "100000000000001");
    s_test_uint_buf(64, UINT64_C(999999999999999), "999999999999999");
    s_test_uint_buf(64, UINT64_C(1000000000000000), "1000000000000000");
    s_test_uint_buf(64, UINT64_C(1000000000000001), "1000000000000001");
    s_test_uint_buf(64, UINT64_C(9999999999999999), "9999999999999999");
    s_test_uint_buf(64, UINT64_C(10000000000000000), "10000000000000000");
    s_test_uint_buf(64, UINT64_C(10000000000000001), "10000000000000001");
    s_test_uint_buf(64, UINT64_C(99999999999999999), "99999999999999999");
    s_test_uint_buf(64, UINT64_C(100000000000000000), "100000000000000000");
    s_test_uint_buf(64, UINT64_C(100000000000000001), "100000000000000001");
    s_test_uint_buf(64, UINT64_C(999999999999999999), "999999999999999999");
    s_test_uint_buf(64, UINT64_C(1000000000000000000), "1000000000000000000");
    s_test_uint_buf(64, UINT64_C(1000000000000000001), "1000000000000000001");
    s_test_uint_buf(64, UINT64_C(9999999999999999999), "9999999999999999999");
    s_test_uint_buf(64, UINT64_C(10000000000000000000), "10000000000000000000");
    s_test_uint_buf(64, UINT64_C(10000000000000000001), "10000000000000000001");

    assert_int_equal(UINT64_C(18446744073709551615), UINT64_MAX);
    s_test_uint_buf(64, UINT64_C(18446744073709551615), "18446744073709551615");

    /* Only verify a couple of values for unsigned int to test that the function
     * returns well-defined values.
     */
    assert_ptr_not_equal(buf, amxc_uint_to_buf(10u, buf));
    assert_int_equal(0, strncmp(buf, "10", 2));
    assert_ptr_not_equal(buf, amxc_uint_to_buf(0xffu, buf));
    assert_int_equal(0, strncmp(buf, "255", 3));
    assert_ptr_not_equal(buf, amxc_uint_to_buf(651u, buf));
    assert_int_equal(0, strncmp(buf, "651", 3));
    assert_ptr_not_equal(buf, amxc_uint_to_buf(6548u, buf));
    assert_int_equal(0, strncmp(buf, "6548", 4));
}

void test_amxc_integer_int_to_buf(UNUSED void** state) {
    char buf[AMXC_INTEGER_INT64_MAX_DIGITS + 1];

    s_test_int_buf(8, INT64_C(0), "0");
    s_test_int_buf(16, INT64_C(0), "0");
    s_test_int_buf(32, INT64_C(0), "0");
    s_test_int_buf(64, INT64_C(0), "0");
    s_test_int_buf(8, INT64_C(1), "1");
    s_test_int_buf(16, INT64_C(1), "1");
    s_test_int_buf(32, INT64_C(1), "1");
    s_test_int_buf(64, INT64_C(1), "1");
    s_test_int_buf(8, INT64_C(-1), "-1");
    s_test_int_buf(16, INT64_C(-1), "-1");
    s_test_int_buf(32, INT64_C(-1), "-1");
    s_test_int_buf(64, INT64_C(-1), "-1");
    s_test_int_buf(8, INT64_C(9), "9");
    s_test_int_buf(16, INT64_C(9), "9");
    s_test_int_buf(32, INT64_C(9), "9");
    s_test_int_buf(64, INT64_C(9), "9");
    s_test_int_buf(8, INT64_C(-9), "-9");
    s_test_int_buf(16, INT64_C(-9), "-9");
    s_test_int_buf(32, INT64_C(-9), "-9");
    s_test_int_buf(64, INT64_C(-9), "-9");
    s_test_int_buf(8, INT64_C(10), "10");
    s_test_int_buf(16, INT64_C(10), "10");
    s_test_int_buf(32, INT64_C(10), "10");
    s_test_int_buf(64, INT64_C(10), "10");
    s_test_int_buf(8, INT64_C(-10), "-10");
    s_test_int_buf(16, INT64_C(-10), "-10");
    s_test_int_buf(32, INT64_C(-10), "-10");
    s_test_int_buf(64, INT64_C(-10), "-10");
    s_test_int_buf(8, INT64_C(100), "100");
    s_test_int_buf(16, INT64_C(100), "100");
    s_test_int_buf(32, INT64_C(100), "100");
    s_test_int_buf(64, INT64_C(100), "100");
    s_test_int_buf(8, INT64_C(-100), "-100");
    s_test_int_buf(16, INT64_C(-100), "-100");
    s_test_int_buf(32, INT64_C(-100), "-100");
    s_test_int_buf(64, INT64_C(-100), "-100");

    assert_int_equal(127, INT8_MAX);
    s_test_int_buf(8, INT64_C(127), "127");
    s_test_int_buf(8, INT64_C(-127), "-127");
    s_test_int_buf(16, INT64_C(127), "127");
    s_test_int_buf(16, INT64_C(-127), "-127");
    s_test_int_buf(32, INT64_C(127), "127");
    s_test_int_buf(32, INT64_C(-127), "-127");
    s_test_int_buf(64, INT64_C(127), "127");
    s_test_int_buf(64, INT64_C(-127), "-127");

    assert_int_equal(-128, INT8_MIN);
    s_test_int_buf(8, INT64_C(-128), "-128");
    s_test_int_buf(16, INT64_C(128), "128");
    s_test_int_buf(16, INT64_C(-128), "-128");
    s_test_int_buf(32, INT64_C(128), "128");
    s_test_int_buf(32, INT64_C(-128), "-128");
    s_test_int_buf(64, INT64_C(128), "128");
    s_test_int_buf(64, INT64_C(-128), "-128");

    s_test_int_buf(16, INT64_C(999), "999");
    s_test_int_buf(16, INT64_C(-999), "-999");
    s_test_int_buf(16, INT64_C(1000), "1000");
    s_test_int_buf(16, INT64_C(-1000), "-1000");
    s_test_int_buf(16, INT64_C(1001), "1001");
    s_test_int_buf(16, INT64_C(-1001), "-1001");
    s_test_int_buf(32, INT64_C(999), "999");
    s_test_int_buf(32, INT64_C(-999), "-999");
    s_test_int_buf(32, INT64_C(1000), "1000");
    s_test_int_buf(32, INT64_C(-1000), "-1000");
    s_test_int_buf(32, INT64_C(1001), "1001");
    s_test_int_buf(32, INT64_C(-1001), "-1001");
    s_test_int_buf(64, INT64_C(999), "999");
    s_test_int_buf(64, INT64_C(-999), "-999");
    s_test_int_buf(64, INT64_C(1000), "1000");
    s_test_int_buf(64, INT64_C(-1000), "-1000");
    s_test_int_buf(64, INT64_C(1001), "1001");
    s_test_int_buf(64, INT64_C(-1001), "-1001");
    s_test_int_buf(16, INT64_C(9999), "9999");
    s_test_int_buf(16, INT64_C(-9999), "-9999");
    s_test_int_buf(16, INT64_C(10000), "10000");
    s_test_int_buf(16, INT64_C(-10000), "-10000");
    s_test_int_buf(16, INT64_C(10001), "10001");
    s_test_int_buf(16, INT64_C(-10001), "-10001");
    s_test_int_buf(32, INT64_C(9999), "9999");
    s_test_int_buf(32, INT64_C(-9999), "-9999");
    s_test_int_buf(32, INT64_C(10000), "10000");
    s_test_int_buf(32, INT64_C(-10000), "-10000");
    s_test_int_buf(32, INT64_C(10001), "10001");
    s_test_int_buf(32, INT64_C(-10001), "-10001");
    s_test_int_buf(64, INT64_C(9999), "9999");
    s_test_int_buf(64, INT64_C(-9999), "-9999");
    s_test_int_buf(64, INT64_C(10000), "10000");
    s_test_int_buf(64, INT64_C(-10000), "-10000");
    s_test_int_buf(64, INT64_C(10001), "10001");
    s_test_int_buf(64, INT64_C(-10001), "-10001");

    assert_int_equal(32767, INT16_MAX);
    s_test_int_buf(16, INT64_C(32767), "32767");
    s_test_int_buf(16, INT64_C(-32767), "-32767");
    s_test_int_buf(32, INT64_C(32767), "32767");
    s_test_int_buf(32, INT64_C(-32767), "-32767");
    s_test_int_buf(64, INT64_C(32767), "32767");
    s_test_int_buf(64, INT64_C(-32767), "-32767");

    assert_int_equal(-32768, INT16_MIN);
    s_test_int_buf(16, INT64_C(-32768), "-32768");
    s_test_int_buf(32, INT64_C(32768), "32768");
    s_test_int_buf(32, INT64_C(-32768), "-32768");
    s_test_int_buf(64, INT64_C(32768), "32768");
    s_test_int_buf(64, INT64_C(-32768), "-32768");

    s_test_int_buf(32, INT64_C(99999), "99999");
    s_test_int_buf(32, INT64_C(-99999), "-99999");
    s_test_int_buf(32, INT64_C(100000), "100000");
    s_test_int_buf(32, INT64_C(-100000), "-100000");
    s_test_int_buf(32, INT64_C(100001), "100001");
    s_test_int_buf(32, INT64_C(-100001), "-100001");
    s_test_int_buf(64, INT64_C(99999), "99999");
    s_test_int_buf(64, INT64_C(-99999), "-99999");
    s_test_int_buf(64, INT64_C(100000), "100000");
    s_test_int_buf(64, INT64_C(-100000), "-100000");
    s_test_int_buf(64, INT64_C(100001), "100001");
    s_test_int_buf(64, INT64_C(-100001), "-100001");
    s_test_int_buf(32, INT64_C(999999), "999999");
    s_test_int_buf(32, INT64_C(-999999), "-999999");
    s_test_int_buf(32, INT64_C(1000000), "1000000");
    s_test_int_buf(32, INT64_C(-1000000), "-1000000");
    s_test_int_buf(32, INT64_C(1000001), "1000001");
    s_test_int_buf(32, INT64_C(-1000001), "-1000001");
    s_test_int_buf(64, INT64_C(999999), "999999");
    s_test_int_buf(64, INT64_C(-999999), "-999999");
    s_test_int_buf(64, INT64_C(1000000), "1000000");
    s_test_int_buf(64, INT64_C(-1000000), "-1000000");
    s_test_int_buf(64, INT64_C(1000001), "1000001");
    s_test_int_buf(64, INT64_C(-1000001), "-1000001");
    s_test_int_buf(32, INT64_C(9999999), "9999999");
    s_test_int_buf(32, INT64_C(-9999999), "-9999999");
    s_test_int_buf(32, INT64_C(10000000), "10000000");
    s_test_int_buf(32, INT64_C(-10000000), "-10000000");
    s_test_int_buf(32, INT64_C(10000001), "10000001");
    s_test_int_buf(32, INT64_C(-10000001), "-10000001");
    s_test_int_buf(64, INT64_C(9999999), "9999999");
    s_test_int_buf(64, INT64_C(-9999999), "-9999999");
    s_test_int_buf(64, INT64_C(10000000), "10000000");
    s_test_int_buf(64, INT64_C(-10000000), "-10000000");
    s_test_int_buf(64, INT64_C(10000001), "10000001");
    s_test_int_buf(64, INT64_C(-10000001), "-10000001");
    s_test_int_buf(32, INT64_C(99999999), "99999999");
    s_test_int_buf(32, INT64_C(-99999999), "-99999999");
    s_test_int_buf(32, INT64_C(100000000), "100000000");
    s_test_int_buf(32, INT64_C(-100000000), "-100000000");
    s_test_int_buf(32, INT64_C(100000001), "100000001");
    s_test_int_buf(32, INT64_C(-100000001), "-100000001");
    s_test_int_buf(64, INT64_C(99999999), "99999999");
    s_test_int_buf(64, INT64_C(-99999999), "-99999999");
    s_test_int_buf(64, INT64_C(100000000), "100000000");
    s_test_int_buf(64, INT64_C(-100000000), "-100000000");
    s_test_int_buf(64, INT64_C(100000001), "100000001");
    s_test_int_buf(64, INT64_C(-100000001), "-100000001");
    s_test_int_buf(32, INT64_C(999999999), "999999999");
    s_test_int_buf(32, INT64_C(-999999999), "-999999999");
    s_test_int_buf(32, INT64_C(1000000000), "1000000000");
    s_test_int_buf(32, INT64_C(-1000000000), "-1000000000");
    s_test_int_buf(32, INT64_C(1000000001), "1000000001");
    s_test_int_buf(32, INT64_C(-1000000001), "-1000000001");
    s_test_int_buf(64, INT64_C(999999999), "999999999");
    s_test_int_buf(64, INT64_C(-999999999), "-999999999");
    s_test_int_buf(64, INT64_C(1000000000), "1000000000");
    s_test_int_buf(64, INT64_C(-1000000000), "-1000000000");
    s_test_int_buf(64, INT64_C(1000000001), "1000000001");
    s_test_int_buf(64, INT64_C(-1000000001), "-1000000001");

    assert_int_equal(2147483647, INT32_MAX);
    s_test_int_buf(32, INT64_C(2147483647), "2147483647");
    s_test_int_buf(32, INT64_C(-2147483647), "-2147483647");
    s_test_int_buf(64, INT64_C(2147483647), "2147483647");
    s_test_int_buf(64, INT64_C(-2147483647), "-2147483647");

    assert_int_equal(-2147483648, INT32_MIN);
    s_test_int_buf(32, INT64_C(-2147483648), "-2147483648");
    s_test_int_buf(64, INT64_C(2147483648), "2147483648");
    s_test_int_buf(64, INT64_C(-2147483648), "-2147483648");

    s_test_int_buf(64, INT64_C(9999999999), "9999999999");
    s_test_int_buf(64, INT64_C(-9999999999), "-9999999999");
    s_test_int_buf(64, INT64_C(10000000000), "10000000000");
    s_test_int_buf(64, INT64_C(-10000000000), "-10000000000");
    s_test_int_buf(64, INT64_C(10000000001), "10000000001");
    s_test_int_buf(64, INT64_C(-10000000001), "-10000000001");
    s_test_int_buf(64, INT64_C(99999999999), "99999999999");
    s_test_int_buf(64, INT64_C(-99999999999), "-99999999999");
    s_test_int_buf(64, INT64_C(100000000000), "100000000000");
    s_test_int_buf(64, INT64_C(-100000000000), "-100000000000");
    s_test_int_buf(64, INT64_C(100000000001), "100000000001");
    s_test_int_buf(64, INT64_C(-100000000001), "-100000000001");
    s_test_int_buf(64, INT64_C(999999999999), "999999999999");
    s_test_int_buf(64, INT64_C(-999999999999), "-999999999999");
    s_test_int_buf(64, INT64_C(1000000000000), "1000000000000");
    s_test_int_buf(64, INT64_C(-1000000000000), "-1000000000000");
    s_test_int_buf(64, INT64_C(1000000000001), "1000000000001");
    s_test_int_buf(64, INT64_C(-1000000000001), "-1000000000001");
    s_test_int_buf(64, INT64_C(9999999999999), "9999999999999");
    s_test_int_buf(64, INT64_C(-9999999999999), "-9999999999999");
    s_test_int_buf(64, INT64_C(10000000000000), "10000000000000");
    s_test_int_buf(64, INT64_C(-10000000000000), "-10000000000000");
    s_test_int_buf(64, INT64_C(10000000000001), "10000000000001");
    s_test_int_buf(64, INT64_C(-10000000000001), "-10000000000001");
    s_test_int_buf(64, INT64_C(99999999999999), "99999999999999");
    s_test_int_buf(64, INT64_C(-99999999999999), "-99999999999999");
    s_test_int_buf(64, INT64_C(100000000000000), "100000000000000");
    s_test_int_buf(64, INT64_C(-100000000000000), "-100000000000000");
    s_test_int_buf(64, INT64_C(100000000000001), "100000000000001");
    s_test_int_buf(64, INT64_C(-100000000000001), "-100000000000001");
    s_test_int_buf(64, INT64_C(999999999999999), "999999999999999");
    s_test_int_buf(64, INT64_C(-999999999999999), "-999999999999999");
    s_test_int_buf(64, INT64_C(1000000000000000), "1000000000000000");
    s_test_int_buf(64, INT64_C(-1000000000000000), "-1000000000000000");
    s_test_int_buf(64, INT64_C(1000000000000001), "1000000000000001");
    s_test_int_buf(64, INT64_C(-1000000000000001), "-1000000000000001");
    s_test_int_buf(64, INT64_C(9999999999999999), "9999999999999999");
    s_test_int_buf(64, INT64_C(-9999999999999999), "-9999999999999999");
    s_test_int_buf(64, INT64_C(10000000000000000), "10000000000000000");
    s_test_int_buf(64, INT64_C(-10000000000000000), "-10000000000000000");
    s_test_int_buf(64, INT64_C(10000000000000001), "10000000000000001");
    s_test_int_buf(64, INT64_C(-10000000000000001), "-10000000000000001");
    s_test_int_buf(64, INT64_C(99999999999999999), "99999999999999999");
    s_test_int_buf(64, INT64_C(-99999999999999999), "-99999999999999999");
    s_test_int_buf(64, INT64_C(100000000000000000), "100000000000000000");
    s_test_int_buf(64, INT64_C(-100000000000000000), "-100000000000000000");
    s_test_int_buf(64, INT64_C(100000000000000001), "100000000000000001");
    s_test_int_buf(64, INT64_C(-100000000000000001), "-100000000000000001");
    s_test_int_buf(64, INT64_C(999999999999999999), "999999999999999999");
    s_test_int_buf(64, INT64_C(-999999999999999999), "-999999999999999999");
    s_test_int_buf(64, INT64_C(1000000000000000000), "1000000000000000000");
    s_test_int_buf(64, INT64_C(-1000000000000000000), "-1000000000000000000");
    s_test_int_buf(64, INT64_C(1000000000000000001), "1000000000000000001");
    s_test_int_buf(64, INT64_C(-1000000000000000001), "-1000000000000000001");

    assert_int_equal(INT64_C(9223372036854775807), INT64_MAX);
    s_test_int_buf(64, INT64_C(9223372036854775807), "9223372036854775807");
    s_test_int_buf(64, INT64_C(-9223372036854775807), "-9223372036854775807");

    /* Can't use a literal -9223372036854775808 because the '-' is an unary
     * operator and not part of the literal. Therefore, the max value is bounded
     * by the largest positive value and not the actual type constraints.
     */
    assert_int_equal(INT64_C(-9223372036854775807 - 1), INT64_MIN);
    s_test_int_buf(64, INT64_C(-9223372036854775807 - 1), "-9223372036854775808");

    /* Only verify a couple of values for int to test that the function returns
     * well-defined values.
     */
    ASSERT_FROM_TYPE(int, 10, "10");
    ASSERT_FROM_TYPE(int, 0xff, "255");
    ASSERT_FROM_TYPE(int, 651, "651");
    ASSERT_FROM_TYPE(int, -461, "-461");
    ASSERT_FROM_TYPE(int, -461525, "-461525");
    assert_ptr_not_equal(buf, amxc_int_to_buf(10, buf));
    assert_int_equal(0, strncmp(buf, "10", 2));
    assert_ptr_not_equal(buf, amxc_int_to_buf(0xff, buf));
    assert_int_equal(0, strncmp(buf, "255", 3));
    assert_ptr_not_equal(buf, amxc_int_to_buf(651, buf));
    assert_int_equal(0, strncmp(buf, "651", 3));
    assert_ptr_not_equal(buf, amxc_int_to_buf(-461, buf));
    assert_int_equal(0, strncmp(buf, "-461", 4));
    assert_ptr_not_equal(buf, amxc_int_to_buf(-461525, buf));
    assert_int_equal(0, strncmp(buf, "-461525", 7));
}

void test_amxc_integer_to_buf_invalid(UNUSED void** state) {
    // WHEN no buffer is given, the methods should not attempt to write to it.
    assert_null(amxc_uint8_to_buf(0, NULL));
    assert_null(amxc_uint16_to_buf(0, NULL));
    assert_null(amxc_uint32_to_buf(0, NULL));
    assert_null(amxc_uint64_to_buf(0, NULL));
    assert_null(amxc_uint_to_buf(0, NULL));

    assert_null(amxc_int8_to_buf(0, NULL));
    assert_null(amxc_int16_to_buf(0, NULL));
    assert_null(amxc_int32_to_buf(0, NULL));
    assert_null(amxc_int64_to_buf(0, NULL));
    assert_null(amxc_int_to_buf(0, NULL));
}

void test_amxc_integer_int_to_str(UNUSED void** state) {
    // GIVEN the limits for the various fixed-size signed integer types, test
    // * all powers of 10
    // * edge cases around INT<N>_MIN and INT<N>_MAX
    // * edge cases around 0
    // * at least a couple of numbers somewhere in between these cases
    ASSERT_FROM_TYPE(int8, INT8_C(0), "0");
    ASSERT_FROM_TYPE(int16, INT16_C(0), "0");
    ASSERT_FROM_TYPE(int32, INT32_C(0), "0");
    ASSERT_FROM_TYPE(int64, INT64_C(0), "0");

    ASSERT_FROM_TYPE(int8, INT8_C(1), "1");
    ASSERT_FROM_TYPE(int16, INT16_C(1), "1");
    ASSERT_FROM_TYPE(int32, INT32_C(1), "1");
    ASSERT_FROM_TYPE(int64, INT64_C(1), "1");

    ASSERT_FROM_TYPE(int8, INT8_C(-1), "-1");
    ASSERT_FROM_TYPE(int16, INT16_C(-1), "-1");
    ASSERT_FROM_TYPE(int32, INT32_C(-1), "-1");
    ASSERT_FROM_TYPE(int64, INT64_C(-1), "-1");

    ASSERT_FROM_TYPE(int8, INT8_C(2), "2");
    ASSERT_FROM_TYPE(int16, INT16_C(2), "2");
    ASSERT_FROM_TYPE(int32, INT32_C(2), "2");
    ASSERT_FROM_TYPE(int64, INT64_C(2), "2");

    ASSERT_FROM_TYPE(int8, INT8_C(-2), "-2");
    ASSERT_FROM_TYPE(int16, INT16_C(-2), "-2");
    ASSERT_FROM_TYPE(int32, INT32_C(-2), "-2");
    ASSERT_FROM_TYPE(int64, INT64_C(-2), "-2");
    ASSERT_FROM_TYPE(int8, INT8_C(3), "3");
    ASSERT_FROM_TYPE(int16, INT16_C(3), "3");
    ASSERT_FROM_TYPE(int32, INT32_C(3), "3");
    ASSERT_FROM_TYPE(int64, INT64_C(3), "3");

    ASSERT_FROM_TYPE(int8, INT8_C(-3), "-3");
    ASSERT_FROM_TYPE(int16, INT16_C(-3), "-3");
    ASSERT_FROM_TYPE(int32, INT32_C(-3), "-3");
    ASSERT_FROM_TYPE(int64, INT64_C(-3), "-3");

    ASSERT_FROM_TYPE(int8, INT8_C(4), "4");
    ASSERT_FROM_TYPE(int16, INT16_C(4), "4");
    ASSERT_FROM_TYPE(int32, INT32_C(4), "4");
    ASSERT_FROM_TYPE(int64, INT64_C(4), "4");

    ASSERT_FROM_TYPE(int8, INT8_C(-4), "-4");
    ASSERT_FROM_TYPE(int16, INT16_C(-4), "-4");
    ASSERT_FROM_TYPE(int32, INT32_C(-4), "-4");
    ASSERT_FROM_TYPE(int64, INT64_C(-4), "-4");

    ASSERT_FROM_TYPE(int8, INT8_C(5), "5");
    ASSERT_FROM_TYPE(int16, INT16_C(5), "5");
    ASSERT_FROM_TYPE(int32, INT32_C(5), "5");
    ASSERT_FROM_TYPE(int64, INT64_C(5), "5");

    ASSERT_FROM_TYPE(int8, INT8_C(-5), "-5");
    ASSERT_FROM_TYPE(int16, INT16_C(-5), "-5");
    ASSERT_FROM_TYPE(int32, INT32_C(-5), "-5");
    ASSERT_FROM_TYPE(int64, INT64_C(-5), "-5");

    ASSERT_FROM_TYPE(int8, INT8_C(6), "6");
    ASSERT_FROM_TYPE(int16, INT16_C(6), "6");
    ASSERT_FROM_TYPE(int32, INT32_C(6), "6");
    ASSERT_FROM_TYPE(int64, INT64_C(6), "6");

    ASSERT_FROM_TYPE(int8, INT8_C(-6), "-6");
    ASSERT_FROM_TYPE(int16, INT16_C(-6), "-6");
    ASSERT_FROM_TYPE(int32, INT32_C(-6), "-6");
    ASSERT_FROM_TYPE(int64, INT64_C(-6), "-6");

    ASSERT_FROM_TYPE(int8, INT8_C(7), "7");
    ASSERT_FROM_TYPE(int16, INT16_C(7), "7");
    ASSERT_FROM_TYPE(int32, INT32_C(7), "7");
    ASSERT_FROM_TYPE(int64, INT64_C(7), "7");

    ASSERT_FROM_TYPE(int8, INT8_C(-7), "-7");
    ASSERT_FROM_TYPE(int16, INT16_C(-7), "-7");
    ASSERT_FROM_TYPE(int32, INT32_C(-7), "-7");
    ASSERT_FROM_TYPE(int64, INT64_C(-7), "-7");

    ASSERT_FROM_TYPE(int8, INT8_C(8), "8");
    ASSERT_FROM_TYPE(int16, INT16_C(8), "8");
    ASSERT_FROM_TYPE(int32, INT32_C(8), "8");
    ASSERT_FROM_TYPE(int64, INT64_C(8), "8");

    ASSERT_FROM_TYPE(int8, INT8_C(-8), "-8");
    ASSERT_FROM_TYPE(int16, INT16_C(-8), "-8");
    ASSERT_FROM_TYPE(int32, INT32_C(-8), "-8");
    ASSERT_FROM_TYPE(int64, INT64_C(-8), "-8");

    ASSERT_FROM_TYPE(int8, INT8_C(9), "9");
    ASSERT_FROM_TYPE(int16, INT16_C(9), "9");
    ASSERT_FROM_TYPE(int32, INT32_C(9), "9");
    ASSERT_FROM_TYPE(int64, INT64_C(9), "9");

    ASSERT_FROM_TYPE(int8, INT8_C(-9), "-9");
    ASSERT_FROM_TYPE(int16, INT16_C(-9), "-9");
    ASSERT_FROM_TYPE(int32, INT32_C(-9), "-9");
    ASSERT_FROM_TYPE(int64, INT64_C(-9), "-9");

    ASSERT_FROM_TYPE(int8, INT8_C(10), "10");
    ASSERT_FROM_TYPE(int16, INT16_C(10), "10");
    ASSERT_FROM_TYPE(int32, INT32_C(10), "10");
    ASSERT_FROM_TYPE(int64, INT64_C(10), "10");

    ASSERT_FROM_TYPE(int8, INT8_C(-10), "-10");
    ASSERT_FROM_TYPE(int16, INT16_C(-10), "-10");
    ASSERT_FROM_TYPE(int32, INT32_C(-10), "-10");
    ASSERT_FROM_TYPE(int64, INT64_C(-10), "-10");

    ASSERT_FROM_TYPE(int8, INT8_C(11), "11");
    ASSERT_FROM_TYPE(int16, INT16_C(11), "11");
    ASSERT_FROM_TYPE(int32, INT32_C(11), "11");
    ASSERT_FROM_TYPE(int64, INT64_C(11), "11");

    ASSERT_FROM_TYPE(int8, INT8_C(-11), "-11");
    ASSERT_FROM_TYPE(int16, INT16_C(-11), "-11");
    ASSERT_FROM_TYPE(int32, INT32_C(-11), "-11");
    ASSERT_FROM_TYPE(int64, INT64_C(-11), "-11");

    ASSERT_FROM_TYPE(int8, INT8_C(57), "57");
    ASSERT_FROM_TYPE(int16, INT16_C(57), "57");
    ASSERT_FROM_TYPE(int32, INT32_C(57), "57");
    ASSERT_FROM_TYPE(int64, INT64_C(57), "57");

    ASSERT_FROM_TYPE(int8, INT8_C(-57), "-57");
    ASSERT_FROM_TYPE(int16, INT16_C(-57), "-57");
    ASSERT_FROM_TYPE(int32, INT32_C(-57), "-57");
    ASSERT_FROM_TYPE(int64, INT64_C(-57), "-57");

    ASSERT_FROM_TYPE(int8, INT8_C(99), "99");
    ASSERT_FROM_TYPE(int16, INT16_C(99), "99");
    ASSERT_FROM_TYPE(int32, INT32_C(99), "99");
    ASSERT_FROM_TYPE(int64, INT64_C(99), "99");

    ASSERT_FROM_TYPE(int8, INT8_C(-99), "-99");
    ASSERT_FROM_TYPE(int16, INT16_C(-99), "-99");
    ASSERT_FROM_TYPE(int32, INT32_C(-99), "-99");
    ASSERT_FROM_TYPE(int64, INT64_C(-99), "-99");

    ASSERT_FROM_TYPE(int8, INT8_C(100), "100");
    ASSERT_FROM_TYPE(int16, INT16_C(100), "100");
    ASSERT_FROM_TYPE(int32, INT32_C(100), "100");
    ASSERT_FROM_TYPE(int64, INT64_C(100), "100");

    ASSERT_FROM_TYPE(int8, INT8_C(-100), "-100");
    ASSERT_FROM_TYPE(int16, INT16_C(-100), "-100");
    ASSERT_FROM_TYPE(int32, INT32_C(-100), "-100");
    ASSERT_FROM_TYPE(int64, INT64_C(-100), "-100");

    ASSERT_FROM_TYPE(int8, INT8_C(101), "101");
    ASSERT_FROM_TYPE(int16, INT16_C(101), "101");
    ASSERT_FROM_TYPE(int32, INT32_C(101), "101");
    ASSERT_FROM_TYPE(int64, INT64_C(101), "101");

    ASSERT_FROM_TYPE(int8, INT8_C(-101), "-101");
    ASSERT_FROM_TYPE(int16, INT16_C(-101), "-101");
    ASSERT_FROM_TYPE(int32, INT32_C(-101), "-101");
    ASSERT_FROM_TYPE(int64, INT64_C(-101), "-101");

    ASSERT_FROM_TYPE(int8, INT8_C(126), "126");
    ASSERT_FROM_TYPE(int16, INT16_C(126), "126");
    ASSERT_FROM_TYPE(int32, INT32_C(126), "126");
    ASSERT_FROM_TYPE(int64, INT64_C(126), "126");

    ASSERT_FROM_TYPE(int8, INT8_C(-126), "-126");
    ASSERT_FROM_TYPE(int16, INT16_C(-126), "-126");
    ASSERT_FROM_TYPE(int32, INT32_C(-126), "-126");
    ASSERT_FROM_TYPE(int64, INT64_C(-126), "-126");

    assert_int_equal(127, INT8_MAX);
    ASSERT_FROM_TYPE(int8, INT8_C(127), "127");
    ASSERT_FROM_TYPE(int16, INT16_C(127), "127");
    ASSERT_FROM_TYPE(int32, INT32_C(127), "127");
    ASSERT_FROM_TYPE(int64, INT64_C(127), "127");

    ASSERT_FROM_TYPE(int8, INT8_C(-127), "-127");
    ASSERT_FROM_TYPE(int16, INT16_C(-127), "-127");
    ASSERT_FROM_TYPE(int32, INT32_C(-127), "-127");
    ASSERT_FROM_TYPE(int64, INT64_C(-127), "-127");

    ASSERT_FROM_TYPE(int16, INT16_C(128), "128");
    ASSERT_FROM_TYPE(int32, INT32_C(128), "128");
    ASSERT_FROM_TYPE(int64, INT64_C(128), "128");

    assert_int_equal(-128, INT8_MIN);
    ASSERT_FROM_TYPE(int8, INT8_C(-128), "-128");
    ASSERT_FROM_TYPE(int16, INT16_C(-128), "-128");
    ASSERT_FROM_TYPE(int32, INT32_C(-128), "-128");
    ASSERT_FROM_TYPE(int64, INT64_C(-128), "-128");

    ASSERT_FROM_TYPE(int16, INT16_C(129), "129");
    ASSERT_FROM_TYPE(int32, INT32_C(129), "129");
    ASSERT_FROM_TYPE(int64, INT64_C(129), "129");

    ASSERT_FROM_TYPE(int16, INT16_C(-129), "-129");
    ASSERT_FROM_TYPE(int32, INT32_C(-129), "-129");
    ASSERT_FROM_TYPE(int64, INT64_C(-129), "-129");

    ASSERT_FROM_TYPE(int16, INT16_C(457), "457");
    ASSERT_FROM_TYPE(int32, INT32_C(457), "457");
    ASSERT_FROM_TYPE(int64, INT64_C(457), "457");

    ASSERT_FROM_TYPE(int16, INT16_C(-457), "-457");
    ASSERT_FROM_TYPE(int32, INT32_C(-457), "-457");
    ASSERT_FROM_TYPE(int64, INT64_C(-457), "-457");

    ASSERT_FROM_TYPE(int16, INT16_C(1000), "1000");
    ASSERT_FROM_TYPE(int32, INT32_C(1000), "1000");
    ASSERT_FROM_TYPE(int64, INT64_C(1000), "1000");
    ASSERT_FROM_TYPE(int16, INT16_C(1001), "1001");
    ASSERT_FROM_TYPE(int32, INT32_C(1001), "1001");
    ASSERT_FROM_TYPE(int64, INT64_C(1001), "1001");

    ASSERT_FROM_TYPE(int16, INT16_C(-1000), "-1000");
    ASSERT_FROM_TYPE(int32, INT32_C(-1000), "-1000");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000), "-1000");
    ASSERT_FROM_TYPE(int16, INT16_C(-1001), "-1001");
    ASSERT_FROM_TYPE(int32, INT32_C(-1001), "-1001");
    ASSERT_FROM_TYPE(int64, INT64_C(-1001), "-1001");

    ASSERT_FROM_TYPE(int16, INT16_C(8571), "8571");
    ASSERT_FROM_TYPE(int32, INT32_C(8571), "8571");
    ASSERT_FROM_TYPE(int64, INT64_C(8571), "8571");

    ASSERT_FROM_TYPE(int16, INT16_C(-8571), "-8571");
    ASSERT_FROM_TYPE(int32, INT32_C(-8571), "-8571");
    ASSERT_FROM_TYPE(int64, INT64_C(-8571), "-8571");

    ASSERT_FROM_TYPE(int16, INT16_C(10000), "10000");
    ASSERT_FROM_TYPE(int32, INT32_C(10000), "10000");
    ASSERT_FROM_TYPE(int64, INT64_C(10000), "10000");
    ASSERT_FROM_TYPE(int16, INT16_C(10001), "10001");
    ASSERT_FROM_TYPE(int32, INT32_C(10001), "10001");
    ASSERT_FROM_TYPE(int64, INT64_C(10001), "10001");

    ASSERT_FROM_TYPE(int16, INT16_C(-10000), "-10000");
    ASSERT_FROM_TYPE(int32, INT32_C(-10000), "-10000");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000), "-10000");
    ASSERT_FROM_TYPE(int16, INT16_C(-10001), "-10001");
    ASSERT_FROM_TYPE(int32, INT32_C(-10001), "-10001");
    ASSERT_FROM_TYPE(int64, INT64_C(-10001), "-10001");

    ASSERT_FROM_TYPE(int16, INT16_C(24152), "24152");
    ASSERT_FROM_TYPE(int32, INT32_C(24152), "24152");
    ASSERT_FROM_TYPE(int64, INT64_C(24152), "24152");

    ASSERT_FROM_TYPE(int16, INT16_C(-24152), "-24152");
    ASSERT_FROM_TYPE(int32, INT32_C(-24152), "-24152");
    ASSERT_FROM_TYPE(int64, INT64_C(-24152), "-24152");

    ASSERT_FROM_TYPE(int16, INT16_C(32766), "32766");
    ASSERT_FROM_TYPE(int32, INT32_C(32766), "32766");
    ASSERT_FROM_TYPE(int64, INT64_C(32766), "32766");

    ASSERT_FROM_TYPE(int16, INT16_C(-32766), "-32766");
    ASSERT_FROM_TYPE(int32, INT32_C(-32766), "-32766");
    ASSERT_FROM_TYPE(int64, INT64_C(-32766), "-32766");

    assert_int_equal(32767, INT16_MAX);
    ASSERT_FROM_TYPE(int16, INT16_C(32767), "32767");
    ASSERT_FROM_TYPE(int32, INT32_C(32767), "32767");
    ASSERT_FROM_TYPE(int64, INT64_C(32767), "32767");

    ASSERT_FROM_TYPE(int16, INT16_C(-32767), "-32767");
    ASSERT_FROM_TYPE(int32, INT32_C(-32767), "-32767");
    ASSERT_FROM_TYPE(int64, INT64_C(-32767), "-32767");

    assert_int_equal(-32768, INT16_MIN);
    ASSERT_FROM_TYPE(int32, INT32_C(32768), "32768");
    ASSERT_FROM_TYPE(int64, INT64_C(32768), "32768");

    ASSERT_FROM_TYPE(int16, INT16_C(-32768), "-32768");
    ASSERT_FROM_TYPE(int32, INT32_C(-32768), "-32768");
    ASSERT_FROM_TYPE(int64, INT64_C(-32768), "-32768");

    ASSERT_FROM_TYPE(int32, INT32_C(32769), "32769");
    ASSERT_FROM_TYPE(int64, INT64_C(32769), "32769");

    ASSERT_FROM_TYPE(int32, INT32_C(-32769), "-32769");
    ASSERT_FROM_TYPE(int64, INT64_C(-32769), "-32769");

    ASSERT_FROM_TYPE(int32, INT32_C(99999), "99999");
    ASSERT_FROM_TYPE(int64, INT64_C(99999), "99999");
    ASSERT_FROM_TYPE(int32, INT32_C(100000), "100000");
    ASSERT_FROM_TYPE(int64, INT64_C(100000), "100000");
    ASSERT_FROM_TYPE(int32, INT32_C(100001), "100001");
    ASSERT_FROM_TYPE(int64, INT64_C(100001), "100001");

    ASSERT_FROM_TYPE(int32, INT32_C(-99999), "-99999");
    ASSERT_FROM_TYPE(int64, INT64_C(-99999), "-99999");
    ASSERT_FROM_TYPE(int32, INT32_C(-100000), "-100000");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000), "-100000");
    ASSERT_FROM_TYPE(int32, INT32_C(-100001), "-100001");
    ASSERT_FROM_TYPE(int64, INT64_C(-100001), "-100001");

    ASSERT_FROM_TYPE(int32, INT32_C(999999), "999999");
    ASSERT_FROM_TYPE(int64, INT64_C(999999), "999999");
    ASSERT_FROM_TYPE(int32, INT32_C(1000000), "1000000");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000), "1000000");
    ASSERT_FROM_TYPE(int32, INT32_C(1000001), "1000001");
    ASSERT_FROM_TYPE(int64, INT64_C(1000001), "1000001");

    ASSERT_FROM_TYPE(int32, INT32_C(-999999), "-999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-999999), "-999999");
    ASSERT_FROM_TYPE(int32, INT32_C(-1000000), "-1000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000), "-1000000");
    ASSERT_FROM_TYPE(int32, INT32_C(-1000001), "-1000001");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000001), "-1000001");

    ASSERT_FROM_TYPE(int32, INT32_C(9999999), "9999999");
    ASSERT_FROM_TYPE(int64, INT64_C(9999999), "9999999");
    ASSERT_FROM_TYPE(int32, INT32_C(10000000), "10000000");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000), "10000000");
    ASSERT_FROM_TYPE(int32, INT32_C(10000001), "10000001");
    ASSERT_FROM_TYPE(int64, INT64_C(10000001), "10000001");

    ASSERT_FROM_TYPE(int32, INT32_C(-9999999), "-9999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-9999999), "-9999999");
    ASSERT_FROM_TYPE(int32, INT32_C(-10000000), "-10000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000), "-10000000");
    ASSERT_FROM_TYPE(int32, INT32_C(-10000001), "-10000001");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000001), "-10000001");

    ASSERT_FROM_TYPE(int32, INT32_C(99999999), "99999999");
    ASSERT_FROM_TYPE(int64, INT64_C(99999999), "99999999");
    ASSERT_FROM_TYPE(int32, INT32_C(100000000), "100000000");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000), "100000000");
    ASSERT_FROM_TYPE(int32, INT32_C(100000001), "100000001");
    ASSERT_FROM_TYPE(int64, INT64_C(100000001), "100000001");

    ASSERT_FROM_TYPE(int32, INT32_C(-99999999), "-99999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-99999999), "-99999999");
    ASSERT_FROM_TYPE(int32, INT32_C(-100000000), "-100000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000), "-100000000");
    ASSERT_FROM_TYPE(int32, INT32_C(-100000001), "-100000001");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000001), "-100000001");

    ASSERT_FROM_TYPE(int32, INT32_C(999999999), "999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(999999999), "999999999");
    ASSERT_FROM_TYPE(int32, INT32_C(1000000000), "1000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000), "1000000000");
    ASSERT_FROM_TYPE(int32, INT32_C(1000000001), "1000000001");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000001), "1000000001");

    ASSERT_FROM_TYPE(int32, INT32_C(-999999999), "-999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-999999999), "-999999999");
    ASSERT_FROM_TYPE(int32, INT32_C(-1000000000), "-1000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000), "-1000000000");
    ASSERT_FROM_TYPE(int32, INT32_C(-1000000001), "-1000000001");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000001), "-1000000001");

    ASSERT_FROM_TYPE(int32, INT32_C(2147483646), "2147483646");
    ASSERT_FROM_TYPE(int64, INT64_C(2147483646), "2147483646");

    ASSERT_FROM_TYPE(int32, INT32_C(-2147483646), "-2147483646");
    ASSERT_FROM_TYPE(int64, INT64_C(-2147483646), "-2147483646");

    assert_int_equal(2147483647, INT32_MAX);
    ASSERT_FROM_TYPE(int32, INT32_C(2147483647), "2147483647");
    ASSERT_FROM_TYPE(int64, INT64_C(2147483647), "2147483647");

    ASSERT_FROM_TYPE(int32, INT32_C(-2147483647), "-2147483647");
    ASSERT_FROM_TYPE(int64, INT64_C(-2147483647), "-2147483647");

    assert_int_equal(-2147483648, INT32_MIN);
    ASSERT_FROM_TYPE(int64, INT64_C(2147483648), "2147483648");

    ASSERT_FROM_TYPE(int32, INT32_C(-2147483648), "-2147483648");
    ASSERT_FROM_TYPE(int64, INT64_C(-2147483648), "-2147483648");

    ASSERT_FROM_TYPE(int64, INT64_C(2147483649), "2147483649");

    ASSERT_FROM_TYPE(int64, INT64_C(-2147483649), "-2147483649");

    ASSERT_FROM_TYPE(int64, INT64_C(9999999999), "9999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000000), "10000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000001), "10000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-9999999999), "-9999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000000), "-10000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000001), "-10000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(99999999999), "99999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000000), "100000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000001), "100000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-99999999999), "-99999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000000), "-100000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000001), "-100000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(999999999999), "999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000000), "1000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000001), "1000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-999999999999), "-999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000000), "-1000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000001), "-1000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(9999999999999), "9999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000000000), "10000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000000001), "10000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-9999999999999), "-9999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000000000), "-10000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000000001), "-10000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(99999999999999), "99999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000000000), "100000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000000001), "100000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-99999999999999), "-99999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000000000), "-100000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000000001), "-100000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(999999999999999), "999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000000000), "1000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000000001), "1000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-999999999999999), "-999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000000000), "-1000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000000001), "-1000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(9999999999999999), "9999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000000000000), "10000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(10000000000000001), "10000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-9999999999999999), "-9999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000000000000), "-10000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-10000000000000001), "-10000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(99999999999999999), "99999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000000000000), "100000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(100000000000000001), "100000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-99999999999999999), "-99999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000000000000), "-100000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-100000000000000001), "-100000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(999999999999999999), "999999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000000000000), "1000000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(1000000000000000001), "1000000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(-999999999999999999), "-999999999999999999");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000000000000), "-1000000000000000000");
    ASSERT_FROM_TYPE(int64, INT64_C(-1000000000000000001), "-1000000000000000001");

    ASSERT_FROM_TYPE(int64, INT64_C(9223372036854775806), "9223372036854775806");

    ASSERT_FROM_TYPE(int64, INT64_C(-9223372036854775806), "-9223372036854775806");

    assert_int_equal(INT64_C(9223372036854775807), INT64_MAX);
    ASSERT_FROM_TYPE(int64, INT64_C(9223372036854775807), "9223372036854775807");

    ASSERT_FROM_TYPE(int64, INT64_C(-9223372036854775807), "-9223372036854775807");

    /* Can't use a literal -9223372036854775808 because the '-' is an unary
     * operator and not part of the literal. Therefore, the max value is bounded
     * by the largest positive value and not the actual type constraints.
     */
    assert_int_equal(INT64_C(-9223372036854775807 - 1), INT64_MIN);
    ASSERT_FROM_TYPE(int64, INT64_C(-9223372036854775807 - 1), "-9223372036854775808");

    /* Only verify a couple of values for int to test that the function returns
     * well-defined values.
     */
    ASSERT_FROM_TYPE(int, 10, "10");
    ASSERT_FROM_TYPE(int, 0xff, "255");
    ASSERT_FROM_TYPE(int, 651, "651");
    ASSERT_FROM_TYPE(int, -461, "-461");
    ASSERT_FROM_TYPE(int, -461525, "-461525");
}

void test_amxc_integer_uint_to_str(UNUSED void** state) {
    // GIVEN the limits for the various fixed-size unsigned integer types, test
    // * all powers of 10
    // * edge cases around UINT<N>_MAX
    // * edge cases around 0
    // * at least a couple of numbers somewhere in between these cases
    ASSERT_FROM_TYPE(uint8, UINT8_C(0), "0");
    ASSERT_FROM_TYPE(uint16, UINT16_C(0), "0");
    ASSERT_FROM_TYPE(uint32, UINT32_C(0), "0");
    ASSERT_FROM_TYPE(uint64, UINT64_C(0), "0");

    ASSERT_FROM_TYPE(uint8, UINT8_C(1), "1");
    ASSERT_FROM_TYPE(uint16, UINT16_C(1), "1");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1), "1");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1), "1");

    ASSERT_FROM_TYPE(uint8, UINT8_C(2), "2");
    ASSERT_FROM_TYPE(uint16, UINT16_C(2), "2");
    ASSERT_FROM_TYPE(uint32, UINT32_C(2), "2");
    ASSERT_FROM_TYPE(uint64, UINT64_C(2), "2");

    ASSERT_FROM_TYPE(uint8, UINT8_C(3), "3");
    ASSERT_FROM_TYPE(uint16, UINT16_C(3), "3");
    ASSERT_FROM_TYPE(uint32, UINT32_C(3), "3");
    ASSERT_FROM_TYPE(uint64, UINT64_C(3), "3");

    ASSERT_FROM_TYPE(uint8, UINT8_C(4), "4");
    ASSERT_FROM_TYPE(uint16, UINT16_C(4), "4");
    ASSERT_FROM_TYPE(uint32, UINT32_C(4), "4");
    ASSERT_FROM_TYPE(uint64, UINT64_C(4), "4");

    ASSERT_FROM_TYPE(uint8, UINT8_C(5), "5");
    ASSERT_FROM_TYPE(uint16, UINT16_C(5), "5");
    ASSERT_FROM_TYPE(uint32, UINT32_C(5), "5");
    ASSERT_FROM_TYPE(uint64, UINT64_C(5), "5");

    ASSERT_FROM_TYPE(uint8, UINT8_C(6), "6");
    ASSERT_FROM_TYPE(uint16, UINT16_C(6), "6");
    ASSERT_FROM_TYPE(uint32, UINT32_C(6), "6");
    ASSERT_FROM_TYPE(uint64, UINT64_C(6), "6");

    ASSERT_FROM_TYPE(uint8, UINT8_C(7), "7");
    ASSERT_FROM_TYPE(uint16, UINT16_C(7), "7");
    ASSERT_FROM_TYPE(uint32, UINT32_C(7), "7");
    ASSERT_FROM_TYPE(uint64, UINT64_C(7), "7");

    ASSERT_FROM_TYPE(uint8, UINT8_C(8), "8");
    ASSERT_FROM_TYPE(uint16, UINT16_C(8), "8");
    ASSERT_FROM_TYPE(uint32, UINT32_C(8), "8");
    ASSERT_FROM_TYPE(uint64, UINT64_C(8), "8");

    ASSERT_FROM_TYPE(uint8, UINT8_C(9), "9");
    ASSERT_FROM_TYPE(uint16, UINT16_C(9), "9");
    ASSERT_FROM_TYPE(uint32, UINT32_C(9), "9");
    ASSERT_FROM_TYPE(uint64, UINT64_C(9), "9");

    ASSERT_FROM_TYPE(uint8, UINT8_C(10), "10");
    ASSERT_FROM_TYPE(uint16, UINT16_C(10), "10");
    ASSERT_FROM_TYPE(uint32, UINT32_C(10), "10");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10), "10");

    ASSERT_FROM_TYPE(uint8, UINT8_C(11), "11");
    ASSERT_FROM_TYPE(uint16, UINT16_C(11), "11");
    ASSERT_FROM_TYPE(uint32, UINT32_C(11), "11");
    ASSERT_FROM_TYPE(uint64, UINT64_C(11), "11");

    ASSERT_FROM_TYPE(uint8, UINT8_C(57), "57");
    ASSERT_FROM_TYPE(uint16, UINT16_C(57), "57");
    ASSERT_FROM_TYPE(uint32, UINT32_C(57), "57");
    ASSERT_FROM_TYPE(uint64, UINT64_C(57), "57");

    ASSERT_FROM_TYPE(uint8, UINT8_C(99), "99");
    ASSERT_FROM_TYPE(uint16, UINT16_C(99), "99");
    ASSERT_FROM_TYPE(uint32, UINT32_C(99), "99");
    ASSERT_FROM_TYPE(uint64, UINT64_C(99), "99");

    ASSERT_FROM_TYPE(uint8, UINT8_C(100), "100");
    ASSERT_FROM_TYPE(uint16, UINT16_C(100), "100");
    ASSERT_FROM_TYPE(uint32, UINT32_C(100), "100");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100), "100");

    ASSERT_FROM_TYPE(uint8, UINT8_C(101), "101");
    ASSERT_FROM_TYPE(uint16, UINT16_C(101), "101");
    ASSERT_FROM_TYPE(uint32, UINT32_C(101), "101");
    ASSERT_FROM_TYPE(uint64, UINT64_C(101), "101");

    ASSERT_FROM_TYPE(uint8, UINT8_C(126), "126");
    ASSERT_FROM_TYPE(uint16, UINT16_C(126), "126");
    ASSERT_FROM_TYPE(uint32, UINT32_C(126), "126");
    ASSERT_FROM_TYPE(uint64, UINT64_C(126), "126");

    ASSERT_FROM_TYPE(uint8, UINT8_C(254), "254");
    ASSERT_FROM_TYPE(uint16, UINT16_C(254), "254");
    ASSERT_FROM_TYPE(uint32, UINT32_C(254), "254");
    ASSERT_FROM_TYPE(uint64, UINT64_C(254), "254");

    assert_int_equal(255, UINT8_MAX);
    ASSERT_FROM_TYPE(uint8, UINT8_C(255), "255");
    ASSERT_FROM_TYPE(uint16, UINT16_C(255), "255");
    ASSERT_FROM_TYPE(uint32, UINT32_C(255), "255");
    ASSERT_FROM_TYPE(uint64, UINT64_C(255), "255");

    ASSERT_FROM_TYPE(uint16, UINT16_C(256), "256");
    ASSERT_FROM_TYPE(uint32, UINT32_C(256), "256");
    ASSERT_FROM_TYPE(uint64, UINT64_C(256), "256");

    ASSERT_FROM_TYPE(uint16, UINT16_C(457), "457");
    ASSERT_FROM_TYPE(uint32, UINT32_C(457), "457");
    ASSERT_FROM_TYPE(uint64, UINT64_C(457), "457");

    ASSERT_FROM_TYPE(uint16, UINT16_C(1000), "1000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1000), "1000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000), "1000");
    ASSERT_FROM_TYPE(uint16, UINT16_C(1001), "1001");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1001), "1001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1001), "1001");

    ASSERT_FROM_TYPE(uint16, UINT16_C(8571), "8571");
    ASSERT_FROM_TYPE(uint32, UINT32_C(8571), "8571");
    ASSERT_FROM_TYPE(uint64, UINT64_C(8571), "8571");

    ASSERT_FROM_TYPE(uint16, UINT16_C(10000), "10000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(10000), "10000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000), "10000");
    ASSERT_FROM_TYPE(uint16, UINT16_C(10001), "10001");
    ASSERT_FROM_TYPE(uint32, UINT32_C(10001), "10001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10001), "10001");

    ASSERT_FROM_TYPE(uint16, UINT16_C(24152), "24152");
    ASSERT_FROM_TYPE(uint32, UINT32_C(24152), "24152");
    ASSERT_FROM_TYPE(uint64, UINT64_C(24152), "24152");

    ASSERT_FROM_TYPE(uint16, UINT16_C(32766), "32766");
    ASSERT_FROM_TYPE(uint32, UINT32_C(32766), "32766");
    ASSERT_FROM_TYPE(uint64, UINT64_C(32766), "32766");

    ASSERT_FROM_TYPE(uint16, UINT16_C(65534), "65534");
    ASSERT_FROM_TYPE(uint32, UINT32_C(65534), "65534");
    ASSERT_FROM_TYPE(uint64, UINT64_C(65534), "65534");

    assert_int_equal(65535, UINT16_MAX);
    ASSERT_FROM_TYPE(uint16, UINT16_C(65535), "65535");
    ASSERT_FROM_TYPE(uint32, UINT32_C(65535), "65535");
    ASSERT_FROM_TYPE(uint64, UINT64_C(65535), "65535");

    ASSERT_FROM_TYPE(uint32, UINT32_C(65536), "65536");
    ASSERT_FROM_TYPE(uint64, UINT64_C(65536), "65536");

    ASSERT_FROM_TYPE(uint32, UINT32_C(99999), "99999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(99999), "99999");
    ASSERT_FROM_TYPE(uint32, UINT32_C(100000), "100000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000), "100000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(100001), "100001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100001), "100001");

    ASSERT_FROM_TYPE(uint32, UINT32_C(999999), "999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(999999), "999999");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1000000), "1000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000), "1000000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1000001), "1000001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000001), "1000001");

    ASSERT_FROM_TYPE(uint32, UINT32_C(9999999), "9999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(9999999), "9999999");
    ASSERT_FROM_TYPE(uint32, UINT32_C(10000000), "10000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000), "10000000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(10000001), "10000001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000001), "10000001");

    ASSERT_FROM_TYPE(uint32, UINT32_C(99999999), "99999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(99999999), "99999999");
    ASSERT_FROM_TYPE(uint32, UINT32_C(100000000), "100000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000), "100000000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(100000001), "100000001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000001), "100000001");

    ASSERT_FROM_TYPE(uint32, UINT32_C(999999999), "999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(999999999), "999999999");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1000000000), "1000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000), "1000000000");
    ASSERT_FROM_TYPE(uint32, UINT32_C(1000000001), "1000000001");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000001), "1000000001");

    ASSERT_FROM_TYPE(uint32, UINT32_C(2147483646), "2147483646");
    ASSERT_FROM_TYPE(uint64, UINT64_C(2147483646), "2147483646");

    ASSERT_FROM_TYPE(uint32, UINT32_C(4294967294), "4294967294");
    ASSERT_FROM_TYPE(uint64, UINT64_C(4294967294), "4294967294");

    assert_int_equal(4294967295, UINT32_MAX);
    ASSERT_FROM_TYPE(uint32, UINT32_C(4294967295), "4294967295");
    ASSERT_FROM_TYPE(uint64, UINT64_C(4294967295), "4294967295");

    ASSERT_FROM_TYPE(uint64, UINT64_C(4294967296), "4294967296");

    ASSERT_FROM_TYPE(uint64, UINT64_C(4294967297), "4294967297");

    ASSERT_FROM_TYPE(uint64, UINT64_C(9999999999), "9999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000), "10000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000001), "10000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(99999999999), "99999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000000), "100000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000001), "100000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(999999999999), "999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000000), "1000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000001), "1000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(9999999999999), "9999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000000), "10000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000001), "10000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(99999999999999), "99999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000000000), "100000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000000001), "100000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(999999999999999), "999999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000000000), "1000000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000000001), "1000000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(9999999999999999), "9999999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000000000), "10000000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000000001), "10000000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(99999999999999999), "99999999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000000000000), "100000000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(100000000000000001), "100000000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(999999999999999999), "999999999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000000000000), "1000000000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(1000000000000000001), "1000000000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(9223372036854775806), "9223372036854775806");

    ASSERT_FROM_TYPE(uint64, UINT64_C(9999999999999999999), "9999999999999999999");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000000000000), "10000000000000000000");
    ASSERT_FROM_TYPE(uint64, UINT64_C(10000000000000000001), "10000000000000000001");

    ASSERT_FROM_TYPE(uint64, UINT64_C(18446744073709551614), "18446744073709551614");
    assert_int_equal(UINT64_C(18446744073709551615), UINT64_MAX);
    ASSERT_FROM_TYPE(uint64, UINT64_C(18446744073709551615), "18446744073709551615");

    /* Only verify a couple of values for int to test that the function returns
     * well-defined values. */
    ASSERT_FROM_TYPE(uint, 10u, "10");
    ASSERT_FROM_TYPE(uint, 0xffu, "255");
    ASSERT_FROM_TYPE(uint, 651u, "651");
}
