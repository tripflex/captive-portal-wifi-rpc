# Mongoose OS Captive Portal WiFi RPC

[![Gitter](https://badges.gitter.im/cesanta/mongoose-os.svg)](https://gitter.im/cesanta/mongoose-os?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

- [Mongoose OS Captive Portal WiFi RPC](#mongoose-os-captive-portal-wifi-rpc)
  - [Captive Portal Stack](#captive-portal-stack)
  - [Author](#author)
  - [Features](#features)
  - [Settings](#settings)
    - [`cportal.rpc.apsta` Setting](#cportalrpcapsta-setting)
  - [Installation/Usage](#installationusage)
    - [Full Captive Portal Stack](#full-captive-portal-stack)
    - [Only this library](#only-this-library)
    - [Use specific branch of library](#use-specific-branch-of-library)
  - [Required Libraries](#required-libraries)
  - [Available RPC EndPoints](#available-rpc-endpoints)
    - [`WiFi.PortalScan`](#wifiportalscan)
    - [`WiFi.PortalTest`](#wifiportaltest)
  - [Available Functions/Methods](#available-functionsmethods)
    - [C Functions](#c-functions)
  - [Changelog](#changelog)
  - [License](#license)

This library adds RPC endpoints for Mongoose OS to Scan for WiFi networks (same as `rpc-service-wifi` lib), and test WiFi credentials (from the `captive-portal-wifi-setup` lib).

## Captive Portal Stack

This is the **WiFi RPC** library from the [Captive Portal WiFi Full Stack](https://github.com/tripflex/captive-portal-wifi-stack), a full stack (frontend web ui & backend handling) library for implementing a full Captive Portal WiFi with Mongoose OS

## Author
Myles McNamara ( https://smyl.es )

## Features
- RPC Endpoint to Scan for Wireless Networks
- RPC Endpoint to Test WiFi connection and credentials
- Disable RPC endpoints after successful wifi test
- Support for Enterprise Networks

## Settings
Check the `mos.yml` file for latest settings, all settings listed below are defaults

```yaml
  - [ "cportal.rpc.enable", "b", true, {title: "Enable WiFi captive portal RPC endpoints on device boot"}]
  - [ "cportal.rpc.disable", "b", false, {title: "Disable RPC endpoints after successful WiFi test"}]
  - [ "cportal.rpc.apsta", "b", false, {title: "(ESP32 only) When RPC is enabled, make sure device is in AP+STA mode to prevent client disconnect when Scan called to switch"}]
```

### `cportal.rpc.apsta` Setting
**This only applies to ESP32 devices**
When this setting is enabled (default is disabled), and RPC is enabled on device boot, this library will for the ESP32 device into AP+STA mode instead of just AP mode.

The reason behind this is because ESP32 devices will boot into `AP` mode initially, but when RPC call is made to Scan for WiFi networks, it will force the device to switch into `AP+STA` mode, disconnecting any clients that may be connected.  In my tests I found that some devices, like iPhones, will not reconnect to that WiFi network after the disconnect ... resulting in a bad user experience.

By forcing the device into `AP+STA` mode on boot, this resolves that issue.  When RPC is disabled, this setting will be ignored (and the device will not be forced into any specific mode).

You can also call the `C` function to do this from your own code, `mgos_captive_portal_wifi_rpc_force_apsta` (see below for details)

## Installation/Usage

### Full Captive Portal Stack
If you want all of the features this library was built for, you should install the [Captive Portal WiFi Stack](https://github.com/tripflex/captive-portal-wifi-stack) library instead of just this one:

Add this lib your `mos.yml` file under `libs:`

```yaml
  - origin: https://github.com/tripflex/captive-portal-wifi-stack
```

### Only this library
Add this lib your `mos.yml` file under `libs:`

```yaml
  - origin: https://github.com/tripflex/captive-portal-wifi-rpc
```

### Use specific branch of library
To use a specific branch of this library (as example, `dev`), you need to specify the `version` below the library

```yaml
  - origin: https://github.com/tripflex/captive-portal-wifi-rpc
   version: dev
```

## Required Libraries
*These libraries are already defined as dependencies of this library, and is just here for reference (you're probably already using these anyways)*
- [wifi](https://github.com/mongoose-os-libs/wifi)
- [rpc-common](https://github.com/mongoose-os-libs/rpc-common)
- [captive-portal-wifi-setup](https://github.com/tripflex/captive-portal-wifi-setup)
  
## Available RPC EndPoints

### `WiFi.PortalScan`
Scan and return available wireless networks.  This is the exact same as the `rpc-service-wifi` library, but was included in this library to allow disabling this endpoint with the test endpoint below as well (if you want -- after done using -- or when in specific "configuration" or other mode) -- as well as having one less lib on your site.

Response will be returned after scan completes.

**Response**
```
[
  {
    "ssid": "my_essid",
    "bssid": "12:34:56:78:90:ab",
    "auth": 0,
    "channel": 1,
    "rssi": -25
  },
  ...
]
```

### `WiFi.PortalTest`
Start WiFi connection and credential test.  Response is returned immediately, and does not wait for test to complete.

**Arguments (required)**
```
{
  ssid: "your ssid",
  pass: "yourpassword",
  user: "EnterpriseUser"
}
```

`user` and `pass` are optional -- if `user` is passed, it is assumed the network attempting to connect to is a WPA2 Enterprise Network.

**Response**
`result` will be a boolean, `true` or `false` if the test was started successfully or not.
```
{
  ssid: "Passed SSID",
  pass: "Passed Password",
  result: true
}
```
`user` will be returned in response above as well if passed in original call

## Available Functions/Methods

### C Functions
```C
/**
 * @brief Force ESP32 Device into AP+STA Mode
 * 
 * @return true 
 */
bool mgos_captive_portal_wifi_rpc_force_apsta(void);
```

**mJS Example:**
```javascript
let FORCE_AP_STA_MODE = ffi('bool mgos_captive_portal_wifi_rpc_force_apsta()');
FORCE_AP_STA_MODE();
```

```C
bool mgos_captive_portal_wifi_rpc_start(void);
```

## Changelog

**1.0.1** (March 10, 2019)
- Added support for Enterprise Networks
- Added `cportal.rpc.apsta` setting to force ESP32 device into AP+STA mode when RPC enabled

**1.0.0** (March 9, 2019)
- Initial release

## License
Apache 2.0