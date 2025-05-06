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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <amxc/amxc_integer.h>
#include <amxc/amxc_macros.h>

#if !defined(__cplusplus) && !defined(static_assert)
#define static_assert _Static_assert
#endif

/**
   @file
   @brief
   Ambiorix integer API implementation
 */

/**
   @brief
   Internal type-agnostic loop implementation for integer to string conversion
   routines.

   @param a variable name of type uint<N>_t, temporary value
   @param u variable name of type uint<N>_t, value to convert, is 0 when loop finishes
   @param p variable name of type char*, pointer to location where next character must be printed
 */
#define AMXC_INT_TO_STR_DECIMAL_LOOP(t, a, u, p) \
    do {                                         \
        a = u % (t) (10);                        \
        u /= (t) (10);                           \
        *p = "0123456789"[a];                    \
        ++p;                                     \
    } while(u > 0)

#define AMXC_INT_WRITE_LITERAL(outp, literal) \
    do {                                            \
        memcpy(outp, literal, sizeof(literal) - 1); \
        outp += sizeof(literal) - 1;                \
    } while(0)

/**
   @brief
   Reverses the characters in the inclusive range [start, end]
 */
static inline void s_reverse(char* start, char* end) {
    char tmp;
    do {
        tmp = *end;
        *end = *start;
        *start = tmp;
        --end;
        ++start;
    } while (start < end);
}

char* amxc_uint8_to_buf(uint8_t value, char* buf) {
    char* p = NULL;
    char* b = NULL;
    uint8_t a = 0;
    uint8_t u;

    when_null(buf, exit);

    b = buf;
    u = value;
    p = b;

    AMXC_INT_TO_STR_DECIMAL_LOOP(uint8_t, a, u, p);

    s_reverse(b, p - 1);

exit:
    return p;
}

char* amxc_uint16_to_buf(uint16_t value, char* buf) {
    char* p = NULL;
    char* b = NULL;
    uint16_t a = 0;
    uint16_t u;

    when_null(buf, exit);

    b = buf;
    u = value;
    p = b;

    AMXC_INT_TO_STR_DECIMAL_LOOP(uint16_t, a, u, p);

    s_reverse(b, p - 1);

exit:
    return p;
}

char* amxc_uint32_to_buf(uint32_t value, char* buf) {
    char* p = NULL;
    char* b = NULL;
    uint32_t a = 0;
    uint32_t u;

    when_null(buf, exit);

    b = buf;
    u = value;
    p = b;

    AMXC_INT_TO_STR_DECIMAL_LOOP(uint32_t, a, u, p);

    s_reverse(b, p - 1);

exit:
    return p;
}

char* amxc_uint64_to_buf(uint64_t value, char* buf) {
    char* p = NULL;
    char* b = NULL;
    uint64_t a = 0;
    uint64_t u;

    when_null(buf, exit);

    b = buf;
    u = value;
    p = b;

    AMXC_INT_TO_STR_DECIMAL_LOOP(uint64_t, a, u, p);

    s_reverse(b, p - 1);

exit:
    return p;
}

char* amxc_uint_to_buf(unsigned int value, char* buf) {
    char* result;

    static_assert((sizeof(unsigned int) == sizeof(uint8_t)) ||
                  (sizeof(unsigned int) == sizeof(uint16_t)) ||
                  (sizeof(unsigned int) == sizeof(uint32_t)) ||
                  (sizeof(unsigned int) == sizeof(uint64_t)),
                  "unsigned int is not one of the supported fixed-width unsigned integer types.");

    switch(sizeof(unsigned int)) {
    case sizeof(uint8_t):
        result = amxc_uint8_to_buf(value, buf);
        break;
    case sizeof(uint16_t):
        result = amxc_uint16_to_buf(value, buf);
        break;
    case sizeof(uint32_t):
        result = amxc_uint32_to_buf(value, buf);
        break;
    case sizeof(uint64_t):
        result = amxc_uint64_to_buf(value, buf);
        break;
    default:
        result = NULL;
        break;
    }

    return result;
}

char* amxc_int8_to_buf(int8_t value, char* buf) {
    char* p = NULL;

    when_null(buf, exit);

    if(value < 0) {
        *buf = '-';
        ++buf;
        /* The absolute value of INT8_MIN does not fit in an int8_t.
         * Therefore, negating and then casting leads to integer overflow.
         * All values do fit in an int16_t, so upcasting to 16 bits, negating
         * and then casting to uint8_t is valid.
         */
        p = amxc_uint8_to_buf((uint8_t) (-(int16_t) value), buf);
    } else {
        p = amxc_uint8_to_buf((uint8_t) value, buf);
    }

exit:
    return p;
}

char* amxc_int16_to_buf(int16_t value, char* buf) {
    char* p = NULL;

    when_null(buf, exit);

    if(value < 0) {
        *buf = '-';
        ++buf;
        /* The absolute value of INT16_MIN does not fit in an int16_t.
         * Therefore, negating and then casting leads to integer overflow.
         * All values do fit in an int32_t, so upcasting to 32 bits, negating
         * and then casting to uint16_t is valid.
         */
        p = amxc_uint16_to_buf((uint16_t) (-(int32_t) value), buf);
    } else {
        p = amxc_uint16_to_buf((uint16_t) value, buf);
    }

exit:
    return p;
}

char* amxc_int32_to_buf(int32_t value, char* buf) {
    char* p = NULL;

    when_null(buf, exit);

    if(value < 0) {
        if(value == INT32_MIN) {
            /* The absolute value of INT32_MIN does not fit in an int32_t.
             * Therefore, negating and then casting leads to integer overflow.
             */
            p = buf;
            AMXC_INT_WRITE_LITERAL(p, "-2147483648");
        } else {
            *buf = '-';
            ++buf;
            p = amxc_uint32_to_buf((uint32_t) (-value), buf);
        }
    } else {
        p = amxc_uint32_to_buf((uint32_t) value, buf);
    }

exit:
    return p;
}

char* amxc_int64_to_buf(int64_t value, char* buf) {
    char* p = NULL;

    when_null(buf, exit);

    if(value < 0) {
        if(value == INT64_MIN) {
            /* The absolute value of INT64_MIN does not fit in an int64_t.
             * Therefore, negating and then casting leads to integer overflow.
             */
            p = buf;
            AMXC_INT_WRITE_LITERAL(p, "-9223372036854775808");
        } else {
            *buf = '-';
            ++buf;
            p = amxc_uint64_to_buf((uint64_t) (-value), buf);
        }
    } else {
        p = amxc_uint64_to_buf((uint64_t) value, buf);
    }

exit:
    return p;
}

char* amxc_int_to_buf(int value, char* buf) {
    char* result;

    static_assert((sizeof(int) == sizeof(int8_t)) ||
                  (sizeof(int) == sizeof(int16_t)) ||
                  (sizeof(int) == sizeof(int32_t)) ||
                  (sizeof(int) == sizeof(int64_t)),
                  "int is not one of the supported fixed-width signed integer types.");

    switch(sizeof(int)) {
    case sizeof(int8_t):
        result = amxc_int8_to_buf(value, buf);
        break;
    case sizeof(int16_t):
        result = amxc_int16_to_buf(value, buf);
        break;
    case sizeof(int32_t):
        result = amxc_int32_to_buf(value, buf);
        break;
    case sizeof(int64_t):
        result = amxc_int64_to_buf(value, buf);
        break;
    default:
        result = NULL;
        break;
    }

    return result;
}

char* amxc_uint8_to_str(uint8_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_UINT8_MAX_DIGITS);
    char* end = amxc_uint8_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_uint16_to_str(uint16_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_UINT16_MAX_DIGITS);
    char* end = amxc_uint16_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_uint32_to_str(uint32_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_UINT32_MAX_DIGITS);
    char* end = amxc_uint32_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_uint64_to_str(uint64_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_UINT64_MAX_DIGITS);
    char* end = amxc_uint64_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_uint_to_str(unsigned int value) {
    char* result;

    static_assert((sizeof(unsigned int) == sizeof(uint8_t)) ||
                  (sizeof(unsigned int) == sizeof(uint16_t)) ||
                  (sizeof(unsigned int) == sizeof(uint32_t)) ||
                  (sizeof(unsigned int) == sizeof(uint64_t)),
                  "unsigned int is not one of the supported fixed-width unsigned integer types.");

    switch(sizeof(unsigned int)) {
    case sizeof(uint8_t):
        result = amxc_uint8_to_str(value);
        break;
    case sizeof(uint16_t):
        result = amxc_uint16_to_str(value);
        break;
    case sizeof(uint32_t):
        result = amxc_uint32_to_str(value);
        break;
    case sizeof(uint64_t):
        result = amxc_uint64_to_str(value);
        break;
    default:
        result = NULL;
        break;
    }

    return result;
}

char* amxc_int8_to_str(int8_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_INT8_MAX_DIGITS);
    char* end = amxc_int8_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_int16_to_str(int16_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_INT16_MAX_DIGITS);
    char* end = amxc_int16_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_int32_to_str(int32_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_INT32_MAX_DIGITS);
    char* end = amxc_int32_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_int64_to_str(int64_t value) {
    char* buf = (char*) malloc(1 + AMXC_INTEGER_INT64_MAX_DIGITS);
    char* end = amxc_int64_to_buf(value, buf);
    if(end == NULL) {
        free(buf);
        buf = NULL;
    } else {
        *end = '\0';
    }
    return buf;
}

char* amxc_int_to_str(int value) {
    char* result;

    static_assert((sizeof(int) == sizeof(int8_t)) ||
                  (sizeof(int) == sizeof(int16_t)) ||
                  (sizeof(int) == sizeof(int32_t)) ||
                  (sizeof(int) == sizeof(int64_t)),
                  "int is not one of the supported fixed-width signed integer types.");

    switch(sizeof(int)) {
    case sizeof(int8_t):
        result = amxc_int8_to_str(value);
        break;
    case sizeof(int16_t):
        result = amxc_int16_to_str(value);
        break;
    case sizeof(int32_t):
        result = amxc_int32_to_str(value);
        break;
    case sizeof(int64_t):
        result = amxc_int64_to_str(value);
        break;
    default:
        result = NULL;
        break;
    }

    return result;
}
