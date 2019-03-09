# Mongoose OS Captive Portal WiFi RPC

[![Gitter](https://badges.gitter.im/cesanta/mongoose-os.svg)](https://gitter.im/cesanta/mongoose-os?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

- [Mongoose OS Captive Portal WiFi RPC](#mongoose-os-captive-portal-wifi-rpc)
  - [Captive Portal Stack](#captive-portal-stack)
  - [Author](#author)
  - [Features](#features)
  - [Settings](#settings)
  - [Installation/Usage](#installationusage)
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

## Settings
Check the `mos.yml` file for latest settings, all settings listed below are defaults

```yaml
  - [ "cportal.rpc.enable", "b", true, {title: "Enable WiFi captive portal RPC endpoints on device boot"}]
  - [ "cportal.rpc.disable", "b", false, {title: "Disable RPC endpoints after successful WiFi test"}]
```

## Installation/Usage
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
  pass: "yourpassword"
}
```

**Response**
`result` will be a boolean, `true` or `false` if the test was started successfully or not.
```
{
  ssid: "Passed SSID",
  pass: "Passed Password",
  result: true
}
```
## Available Functions/Methods

### C Functions
```C
bool mgos_captive_portal_wifi_rpc_start(void);
```

## Changelog

**1.0.0** TBD - Initial release

## License
Apache 2.0