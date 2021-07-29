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
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "lwip/sockets.h"

#include "status_report_private.h"

static const char *TAG = "status-report";

static void status_report_task(void *arg)
{
    status_report_private_handle* handle_priv = (status_report_private_handle*)arg;
    char client_addr_str[128];
    int addr_family = AF_INET;
    int port = 1234;
    int ip_protocol = 0;
    struct sockaddr_storage dest_addr;
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);

    ESP_LOGI(TAG, "Start Test Status Report Server...");

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(port);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP_LISTEN_SOCKET;
    }

    ESP_LOGI(TAG, "Socket bound, port %d", port);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP_LISTEN_SOCKET;
    }

    ESP_LOGI(TAG, "Socket listening");

    int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
        goto CLEAN_UP_LISTEN_SOCKET;
    }

    // Convert ip address to string
    if (source_addr.ss_family == PF_INET) {
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, client_addr_str, sizeof(client_addr_str) - 1);
    }

    ESP_LOGI(TAG, "Socket accepted ip address: %s", client_addr_str);
    /* host connected -> start */
    xSemaphoreGive(handle_priv->handle_data.start_sem);

    char msg[MAX_MSG_SIZE];

    while(1) {
        if(xQueueReceive(handle_priv->handle_data.send_report_task_queue, &(msg[0]), pdMS_TO_TICKS(1000)) == pdTRUE) {
            int written = send(sock, &msg, strlen(msg), 0);
            if (written < 0) {
                ESP_LOGE(TAG, "Error occurred during sending report: errno %d", errno);
                /* signal stop */
                xSemaphoreGive(handle_priv->handle_data.stop_sem);
                break;
            }
        }
        else if(recv(sock, NULL, 1, MSG_PEEK | MSG_DONTWAIT) == 0) {
            /* signal stop */
            xSemaphoreGive(handle_priv->handle_data.stop_sem);
            break;
        }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    shutdown(sock, 0);
    close(sock);
CLEAN_UP_LISTEN_SOCKET:
    close(listen_sock);
    vTaskDelete(NULL);
}

static esp_err_t wait_for_host_to_start(status_report_public_handle *handle)
{
    status_report_private_handle* handle_priv = (status_report_private_handle*)handle;
    /* wait for start */
    xSemaphoreTake(handle_priv->handle_data.start_sem, portMAX_DELAY);
    ESP_LOGI(TAG, "Start");
    return ESP_OK;
}

static esp_err_t wait_for_host_to_stop(status_report_public_handle *handle)
{
    status_report_private_handle* handle_priv = (status_report_private_handle*)handle;
    /* wait for start */
    xSemaphoreTake(handle_priv->handle_data.stop_sem, portMAX_DELAY);
    ESP_LOGI(TAG, "Stop");
    return ESP_OK;
}

static esp_err_t report_status_to_host(status_report_public_handle *handle, char* msg)
{
    status_report_private_handle* handle_priv = (status_report_private_handle*)handle;
    ESP_LOGI(TAG, "Report to host: %s", msg);
    /* send queue */
    xQueueSend(handle_priv->handle_data.send_report_task_queue, &(msg[0]), portMAX_DELAY);
    return ESP_OK;
}

void host_report_create_instance(status_report_public_handle** return_handle, int port)
{
    /* create handle */
    status_report_private_handle* handle = malloc(sizeof(status_report_private_handle));
    /* set methods */
    handle->handle_pub.wait_for_start = &wait_for_host_to_start;
    handle->handle_pub.wait_for_stop = &wait_for_host_to_stop;
    handle->handle_pub.report_status = &report_status_to_host;
    /* create queues and semaphores */
    handle->handle_data.send_report_task_queue = xQueueCreate(10, sizeof(char)*MAX_MSG_SIZE);
    handle->handle_data.start_sem = xSemaphoreCreateBinary();
    handle->handle_data.stop_sem = xSemaphoreCreateBinary();
    /* set parameter */
    handle->handle_data.port = port;

    /* give task name and set prio */
    xTaskCreate(&status_report_task, "test_status_report", STATUS_REPORT_THREAD_STACK_SIZE, (void*)handle, 5, NULL);
    /* return handle to caller */
    *return_handle = (status_report_public_handle*)handle;
}
