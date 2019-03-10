/*
 * Copyright (c) 2018 Myles McNamara <https://smyl.es>
 * Copyright (c) 2014-2018 Cesanta Software Limited (Scan)
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdlib.h>

#include "common/json_utils.h"
#include "mgos.h"
#include "mgos_rpc.h"
#include "mgos_event.h"
#include "mgos_config.h"
#include "mgos_captive_portal_wifi_setup.h"

#if CS_PLATFORM == CS_P_ESP32
#include "esp_wifi.h"
#endif

char *s_test_ssid = NULL;
char *s_test_pass = NULL;
bool s_captive_portal_rpc_init = false;

static int wifi_scan_result_printer(struct json_out *out, va_list *ap) {
  int len = 0;
  int num_res = va_arg(*ap, int);

  const struct mgos_wifi_scan_result *res =
      va_arg(*ap, const struct mgos_wifi_scan_result *);

  for (int i = 0; i < num_res; i++) {
    const struct mgos_wifi_scan_result *r = &res[i];
    if (i > 0) len += json_printf(out, ", ");

    len += json_printf(out,
                       "{ssid: %Q, bssid: \"%02x:%02x:%02x:%02x:%02x:%02x\", "
                       "auth: %d, channel: %d,"
                       " rssi: %d}",
                       r->ssid, r->bssid[0], r->bssid[1], r->bssid[2],
                       r->bssid[3], r->bssid[4], r->bssid[5], r->auth_mode,
                       r->channel, r->rssi);
  }

  return len;
}

static void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg) {
    struct mg_rpc_request_info *ri = (struct mg_rpc_request_info *) arg;

    if (n < 0) {
        mg_rpc_send_errorf(ri, n, "wifi scan failed");
        return;
    }
    mg_rpc_send_responsef(ri, "[%M]", wifi_scan_result_printer, n, res);
}

static void mgos_captive_portal_wifi_scan_rpc_handler(struct mg_rpc_request_info *ri,
                                       void *cb_arg,
                                       struct mg_rpc_frame_info *fi,
                                       struct mg_str args) {
    mgos_wifi_scan(wifi_scan_cb, ri);

    (void) args;
    (void) cb_arg;
    (void) fi;
}

static void mgos_captive_portal_wifi_test_rpc_handler(struct mg_rpc_request_info *ri, void *cb_arg,
                                                      struct mg_rpc_frame_info *fi,
                                                      struct mg_str args){

    LOG(LL_INFO, ("WiFi.PortalTest RPC Handler Parsing JSON: %.*s\n", args.len, args.p) );
    char *ssid;
    char *pass;
    char *user;

    json_scanf(args.p, args.len, ri->args_fmt, &ssid, &pass, &user );

    if (mgos_conf_str_empty(ssid)){
        mg_rpc_send_errorf(ri, 400, "SSID is required!" );
        return;
    }

    if (!mgos_conf_str_empty(user)){
        LOG(LL_INFO, ("WiFi.PortalTest RPC Handler ssid: %s pass: %s user: %s", ssid, pass, user));
        bool result = mgos_captive_portal_wifi_setup_test_ent( ssid, pass, user, NULL, NULL );
        mg_rpc_send_responsef(ri, "{ ssid: %Q, pass: %Q, user: %Q, result: %B }", ssid, pass, user, result);
    } else {
        LOG(LL_INFO, ("WiFi.PortalTest RPC Handler ssid: %s pass: %s", ssid, pass));
        bool result = mgos_captive_portal_wifi_setup_test( ssid, pass, NULL, NULL );
        mg_rpc_send_responsef(ri, "{ ssid: %Q, pass: %Q, result: %B }", ssid, pass, result);
    }

    (void)cb_arg;
    (void)fi;
}

static void test_success_cb(int ev, void *ev_data, void *userdata){
    mgos_sys_config_set_cportal_rpc_enable(false);
    mgos_event_remove_handler(MGOS_WIFI_EV_STA_IP_ACQUIRED, test_success_cb, NULL);

    char *err = NULL;
    if (!save_cfg(&mgos_sys_config, &err)){
        LOG(LL_ERROR, ("Disable RPC Save Config Error: %s", err));
        free(err);
    }

    (void)ev;
    (void)ev_data;
    (void)userdata;
}

bool mgos_captive_portal_wifi_rpc_force_apsta(void){
#if CS_PLATFORM == CS_P_ESP32
        // We have to call this to set device in AP+STA mode, as when the Scan is called, it will force the device into
        // AP+STA mode, causing a disconnect to the client if it's not already in this mode.
        // By adding this we can set the device in that mode immediately, to prevent that disconnect of the client device
        // Only relevant on ESP32 devices
        esp_wifi_set_mode(WIFI_MODE_APSTA);
#endif
    return true;
}

bool mgos_captive_portal_wifi_rpc_start(void){

    if( ! s_captive_portal_rpc_init ){

        if( mgos_sys_config_get_cportal_rpc_apsta() ){
            // Will only do something if device is ESP32
            mgos_captive_portal_wifi_rpc_force_apsta();
        }

        struct mg_rpc *c = mgos_rpc_get_global();
        mg_rpc_add_handler(c, "WiFi.PortalTest", "{ssid: %Q, pass: %Q, user: %Q}", mgos_captive_portal_wifi_test_rpc_handler, NULL);
        mg_rpc_add_handler(c, "Wifi.PortalScan", "", mgos_captive_portal_wifi_scan_rpc_handler, NULL);
        s_captive_portal_rpc_init = true;
        return true;
    }

    return false;
}

bool mgos_captive_portal_wifi_rpc_init(void){
    if( mgos_sys_config_get_cportal_rpc_enable() ){
        mgos_captive_portal_wifi_rpc_start();
    }
    if( mgos_sys_config_get_cportal_rpc_disable() ){
        mgos_event_add_handler(MGOS_CAPTIVE_PORTAL_WIFI_SETUP_TEST_SUCCESS, test_success_cb, NULL);
    }
    
    return true;
}