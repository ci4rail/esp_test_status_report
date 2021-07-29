/*
Copyright © 2021 Ci4Rail GmbH
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef _STATUS_REPORT_PUBLIC_H_
#define _STATUS_REPORT_PUBLIC_H_

#include "esp_err.h"

#define MAX_MSG_SIZE  255U

typedef struct status_report_public_handle status_report_public_handle;

struct status_report_public_handle {
    /* todo return error */
    esp_err_t (*report_status)(status_report_public_handle*, char*);
    esp_err_t (*wait_for_start)(status_report_public_handle*);
    esp_err_t (*wait_for_stop)(status_report_public_handle*);
};

void host_report_create_instance(status_report_public_handle** return_handle, int port);

#endif //_STATUS_REPORT_PUBLIC_H_
