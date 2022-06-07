# Spresense LTE-M IoT Connectivity Kit Example Code

![02-kit-top-cropped](https://user-images.githubusercontent.com/33822072/172305316-51a5b8c2-378c-45b4-8aed-684f4004f04f.jpg)

## Overview
This repository provides example sketches using Spresense LTE-M IoT Connectivity Kit.

## How to use

Please refer to our Developer document: https://developers.soracom.io/en/start/connect/sony-spresense

## About the sketch
The sketch will show you how to send some sample data to Soracom Harvest Data service.
It mainly uses public code parts from the following Sony Spresense LTE Tutorials:

[Spresense Modem Status from LTE Tutorial](https://developer.sony.com/develop/spresense/docs/arduino_tutorials_en.html#_tutorial_lte)

[Spresense LteScanNetworks from LTE Tutorial](https://developer.sony.com/develop/spresense/docs/arduino_tutorials_en.html#_get_network_status)

#### Prerequisites:

- This sketch uses [libraries of Spresense](https://github.com/sonydevworld/spresense-arduino-compatible/tree/master/Arduino15/packages/SPRESENSE/hardware/spresense/1.0.0/libraries). Please setup your Arduino IDE to include the libraries following [Soracom Spresense Setup Guide](https://developers.soracom.io/en/start/connect/sony-spresense)
- You need to activate Soracom Harvest Data service in a Group which is assigned to your SIM. Please check [Soracom Spresense Setup Guide](https://developers.soracom.io/en/start/connect/sony-spresense) to find out how to activate the service.

### Definitions:
Soracom APN, Username and Password definition is done in the Cellular network settings section:
```c
/**
 * Cellular network settings
 */
#define APP_LTE_APN "soracom.io"
#define APP_LTE_USERNAME "sora"
#define APP_LTE_PASSWORD "sora" 
```

Soracom provides IPv4 connectivity, therefore LTE_NET_IPTYPE is set to IP-Version 4:
```c
#define APP_LTE_IP_TYPE (LTE_NET_IPTYPE_V4)
```
Soracom accepts both CHAP and PAP authentication methods by default. Here CHAP authentication is set:
```c
#define APP_LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_CHAP)
```
Soracom provides LTE-M (LTE Cat-M1) coverage in several countries. Please check our [Supported Carriers](https://developers.soracom.io/en/docs/reference/carriers/) list for more details or [contact us](https://www.soracom.io/contact).
```c
#define APP_LTE_RAT (LTE_NET_RAT_CATM)
```
Soracom Harvest Data service endpoint definition is done here according to the [Soracom Harvest developer documentation](https://developers.soracom.io/en/docs/harvest/).
Destination endpoint can easily be changed to the Soracom Unified endpoint. 
```c
char server[] = "harvest.soracom.io"; // Soracom Harvest address
int port = 8514;                      // Soracom Harvest UDP port

//char server[] = "unified.soracom.io"; // Soracom Unified endpoint address
//int port = 23080;                      // Soracom Unified endpoint UDP port
```
The data to be send is defined in this buffer:
```c
char sendBuffer[] = "Hello Soracom!"; // The string to send
```

In the setup section the initialization of the LTE connection and the attach to the data network service takes place.
The network connection details will be shown according to the console output section below. 
During this sequence the Spresense Main board LEDs will indicate the progress according to the LED section below.

The device will start start a UDP client on local ``` port 8000 ``` to be able to receive UDP data.
There will be the need for a DNS query to get the IP of Soracom Harvest data endpoint.
A UDP packet is then sent to Soracom Harvest data endpoint on ``` port 8514 ```.

Soracom Harvest data will acknowledge the receiption of the data and return a UDP packet containing 3 Bytes (see Received packet section for details).

A disconnection of all instances will take place at the end of this example.

### Console output at the Serial port
The sketch will show the current status of the connection in the console output. The Spresense Main board LEDs are used to show progress through the run.
Details about the LED status are listed in the next section.

```text
Starting...
=========== APN information ===========
Access Point Name  : soracom.io
Username           : sora
Password           : sora
Authentication Type: CHAP

Successfully attached to network!

=========== Connection info ===========
Current carrier: SORACOM
IP address: 10.216.xxx.xxx
Current RAT: LTE-M (LTE Cat-M1)
Signal Strength: -105 [dBm]
=========== UDP client ===========

UDP client listening on port 8000

Connected to harvest.soracom.io
Data sent to Soracom Harvest!

Received packet! Size: 3
Server response: 201

Now check Harvest Data to see the sample data.

Disconnecting.
```

### LED0,LED1,LED2,LED3 on Spresense Main board
- LED0 ON; LED #1 (LED0) to indicate that the program is running
- LED1 ON; LED #2 (LED1) to indicate cellular network connection
- LED2 ON; LED #3 (LED2) to indicate that the UDP client is ready
- LED2 ON; LED #4 (LED3) to indicate that device has connected to server
- ALL  OFF; to indicate end of program and all connections closed

### Received packet
- Size=3 Soracom Harvest Data service will return a packet to the device to indicate whether or not data was successfully sent (always 3 Bytes) 

### Server response
- 201 : Data was successfully received by Harvest.
- 400 : Data was rejected by Harvest, most likely because Harvest is not enabled or device does not belong to a group with Harvest enabled.

## License

The sketches in this repository are released under [The MIT License](./LICENSE), and libraries used by these sketches are subject to their respective licenses.    
For example, the library of [Arduino Core codes for Spresense reference board](https://github.com/sonydevworld/spresense-arduino-compatible/tree/master/Arduino15/packages/SPRESENSE/hardware/spresense/1.0.0) used for cellular communications is released under the LGPL v2.1 license.

## Disclaimers

The sketches in this repository are examples only and are not guaranteed to work. In addition, the content of this sketch is not intended for commercial use.
Please use them at your own risk.
