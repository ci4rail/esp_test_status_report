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
#include "status_report_public.h"

#include "lwip/sockets.h"
#include "tusb_cdc_ecm.h"
#include "tinyusb.h"

static const char *TAG = "status-report-example";

/* configure cdc-ecm driver ip */
static const ip_addr_t ipaddr  = IPADDR4_INIT_BYTES(192, 168, 7, 1);
static const ip_addr_t netmask = IPADDR4_INIT_BYTES(255, 255, 255, 0);
static const ip_addr_t gateway = IPADDR4_INIT_BYTES(0, 0, 0, 0);

static void initialize_cdc_ecm_interface(void)
{
    /* setup CDC-ECM interface*/
    tinyusb_config_t tusb_cfg = {
        .descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_ethernet_over_usb_t tusb_ethernet_over_usb_cfg = {
        .ipaddr = ipaddr,
        .netmask = netmask,
        .gateway = gateway,
        .mac_address = {}//MAC address is defined later
    };

    /* get pre-programmed ethernet MAC address */
    esp_read_mac(tusb_ethernet_over_usb_cfg.mac_address, ESP_MAC_ETH);

    ESP_ERROR_CHECK(tusb_ethernet_over_usb_init(tusb_ethernet_over_usb_cfg));
}

void app_main(void)
{
    status_report_public_handle *handle;
    int port = 10000;

    /* setup cdc ecm interface */
    initialize_cdc_ecm_interface();

    host_report_create_instance(&handle, port);
    handle->wait_for_start(handle);
    handle->report_status(handle, "Test String");
    handle->wait_for_stop(handle);
    ESP_LOGI(TAG, "Test stopped");
}
