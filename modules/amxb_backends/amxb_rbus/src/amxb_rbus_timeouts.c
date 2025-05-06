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

#include "amxb_rbus.h"

// AS THE RBUS CONFIG IS NOT PUBLIC ACCESSIBLE
// BUT IT WOULD BE NICE THAT THE TIMEOUTS ARE
// CONFIGURABLE OR AT LEAST CAN BE CHANGED
// THE FOLLOWING SECTION IS A COPY FROM
// rbus_config.h WHICH IS NOT INSTALLED IN THE
// RBUS INCLUDE DIR

////////////////////////////////////////////////////////////////////////////////
typedef struct _rbusConfig_t {
    char* tmpDir;                       /* temp directory where rbus can persist data*/
    int subscribeTimeout;               /* max time to attempt subscribe retries in milisecond*/
    int subscribeMaxWait;               /* max time to wait between subscribe retries in miliseconds*/
    int valueChangePeriod;              /* polling period for valuechange detector in miliseconds*/
    int getTimeout;                     /* default timeout in miliseconds for GET API*/
    int setTimeout;                     /* default timeout in miliseconds for SET API*/
} rbusConfig_t;

// LUCKELY THIS SYMBOL IS EXPORTED BY LIBRBUS, SO IT CAN BE CALLED
// AND THE TIMEOUT CONFIG CAN BE RETRIEVED
rbusConfig_t* rbusConfig_Get(void);

// END OF COPY
////////////////////////////////////////////////////////////////////////////////

void amxb_rbus_set_timeout(int timeout) {
    rbusConfig_t* rbus_config = rbusConfig_Get();

    when_null(rbus_config, exit);

    rbus_config->getTimeout = timeout * 1000;
    rbus_config->setTimeout = timeout * 1000;

exit:
    return;
}