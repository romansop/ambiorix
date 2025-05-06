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

#if !defined(__AMXC_INTEGER_H__)
#define __AMXC_INTEGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <amxc/amxc_common.h>
#include <stdint.h>

/**
   @file
   @brief
   Ambiorix integer API header file
 */

/**
   @ingroup amxc_containers
   @defgroup amxc_integer Integer

   @brief
   The Ambiorix integer is a collection of integer-related algorithms.

 */

/**
   @ingroup amxc_integer
   @name Integer decimal representation constants
   @brief
   The maximum number of characters required to represent the various fixed-width
   integer types using decimal notation.

   The optional null terminal is not included in these counts. For signed numbers,
   the optional '-' character is included.

   @internal
   Ideally, these would be defined based on the standard INT<N>_MIN and
   (U?)INT<N>_MAX macros. However, these macros do not have a standardized
   implementation style. Compiler implementations are allowed to define them in
   any way they see fit. For example, UINT8_MAX could be defined in decimal
   notation, hexadecimal, with extra type suffixes, with brackets...
   @{
 */

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a uint8_t in decimal,
   excluding null terminal.
 */
#define AMXC_INTEGER_UINT8_MAX_DIGITS (sizeof("255") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a uint16_t in decimal,
   excluding null terminal.
 */
#define AMXC_INTEGER_UINT16_MAX_DIGITS (sizeof("65535") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a uint32_t in decimal,
   excluding null terminal.
 */
#define AMXC_INTEGER_UINT32_MAX_DIGITS (sizeof("4294967295") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a uint64_t in decimal,
   excluding null terminal.
 */
#define AMXC_INTEGER_UINT64_MAX_DIGITS (sizeof("18446744073709551615") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a int8_t in decimal,
   including optional minus sign, excluding null terminal.
 */
#define AMXC_INTEGER_INT8_MAX_DIGITS (sizeof("-128") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a int16_t in decimal,
   including optional minus sign, excluding null terminal.
 */
#define AMXC_INTEGER_INT16_MAX_DIGITS (sizeof("-32768") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a int32_t in decimal,
   including optional minus sign, excluding null terminal.
 */
#define AMXC_INTEGER_INT32_MAX_DIGITS (sizeof("-2147483648") - 1)

/**
   @ingroup amxc_integer
   @brief
   The maximum number of charaters required to represent a int64_t in decimal,
   including optional minus sign, excluding null terminal.
 */
#define AMXC_INTEGER_INT64_MAX_DIGITS (sizeof("-9223372036854775808") - 1)
/** @} */


/**
   @ingroup amxc_integer
   @name Integer to buffer
   @brief
   These functions write the string representation of a given integer value to a
   pre-allocated buffer.

   The buffer is assumed to be large enough to contain the string representation.
   No space checking is done.

   @note
   These functions do not write a null terminal to the buffer.
   @{
 */
/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given unsigned integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_UINT8_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_uint8_to_buf(uint8_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given unsigned integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_UINT16_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_uint16_to_buf(uint16_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given unsigned integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_UINT32_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_uint32_to_buf(uint32_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given unsigned integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_UINT64_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_uint64_to_buf(uint64_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given unsigned integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   How long this is, depends on the size of int on your platform.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_uint_to_buf(unsigned int value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given signed integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_INT8_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_int8_to_buf(int8_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given signed integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_INT16_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_int16_to_buf(int16_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given signed integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_INT32_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_int32_to_buf(int32_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given signed integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   Typically, this means that the buffer must be at least
   @ref AMXC_INTEGER_INT64_MAX_DIGITS characters long.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_int64_to_buf(int64_t value, char* buf);

/**
   @ingroup amxc_integer
   @brief
   Writes the decimal representation of the given signed integer to a buffer.

   Fills the given buffer with the decimal representation of the given integer
   and returns a pointer to the first buffer location after the last writen character.

   @attention
   The buffer is assumed to be long enough to contain the entire string value.
   How long this is, depends on the size of int on your platform.

   @attention
   The written string is not null-terminated.

   @param value Integer value that must be converted.
   @param buf Character buffer where the result will be written.

   @return
   A pointer to the first location in the buffer after the written characters, or
   NULL on error.
 */
char* amxc_int_to_buf(int value, char* buf);
/** @} */


/**
   @ingroup amxc_integer
   @name Integer to string
   @brief
   These functions write the string representation of a given integer value to a
   newly-allocated null-terminated string.

   @note
   The returned string must be freed.

   @{
 */
/**
   @ingroup amxc_integer
   @brief
   Converts the given unsigned integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_uint8_to_str(uint8_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given unsigned integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_uint16_to_str(uint16_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given unsigned integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_uint32_to_str(uint32_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given unsigned integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_uint64_to_str(uint64_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given unsigned integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_uint_to_str(unsigned int value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given signed integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_int8_to_str(int8_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given signed integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_int16_to_str(int16_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given signed integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_int32_to_str(int32_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given signed integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_int64_to_str(int64_t value);

/**
   @ingroup amxc_integer
   @brief
   Converts the given signed integer to its decimal string representation.

   Allocates a new character buffer and fills it with the decimal string
   representation of the given integer value.

   @attention
   The returned string will be null-terminated.

   @attention
   The returned string must be freed.

   @param value Integer value that must be converted.

   @return
   A pointer to a null-terminated string containing the decimal representation of value,
   or NULL on error.
 */
char* amxc_int_to_str(int value);
/** @} */

#ifdef __cplusplus
}
#endif

#endif // __AMXC_INTEGER_H__
