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
#ifndef _TEST_STATUS_REPORT_H_
#define _TEST_STATUS_REPORT_H_

#include "esp_err.h"

#define MAX_MSG_SIZE  255U

typedef struct test_status_report_handle_t test_status_report_handle_t;

struct test_status_report_handle_t {
    esp_err_t (*report_status)(test_status_report_handle_t*, char*);
    esp_err_t (*wait_for_start)(test_status_report_handle_t*);
    esp_err_t (*wait_for_stop)(test_status_report_handle_t*);
};

esp_err_t new_test_status_report_instance(test_status_report_handle_t** return_handle, int port);

#endif //_TEST_STATUS_REPORT_H_
