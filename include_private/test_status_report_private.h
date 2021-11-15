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
#ifndef _TEST_STATUS_REPORT_PRIVATE_H_
#define _TEST_STATUS_REPORT_PRIVATE_H_

#include "test_status_report.h"

#define STATUS_REPORT_THREAD_STACK_SIZE 4096

typedef struct {
    int port;
    QueueHandle_t send_report_task_queue;
    SemaphoreHandle_t start_sem;
    SemaphoreHandle_t stop_sem;
} status_report_data_t;

typedef struct {
    test_status_report_handle_t handle_pub;
    status_report_data_t handle_data;
} test_status_report_handle_priv_t;

#endif  //_TEST_STATUS_REPORT_PRIVATE_H_
