# lte_at_cmd AT Command application for Sony Spresense with LTE-M Extension Board

## Overview

This Spresense `lte_at_cmd` application provides a simple interface for sending AT commands to the Sony Spresense LTE-M Extension Board. It implements a simple AT command interface using `lte_send_atcmd_sync` function.

- `AT+` commands are defined in 3GPP standard [3GPP 27.007 commands](https://www.etsi.org/deliver/etsi_ts/127000_127099/127007/13.03.00_60/ts_127007v130300p.pdf).
- `AT%` commands are proprietary commands for the Murata 1SC-DM module.

---

## Usage

This application is meant to be compiled using the [Sony Spresense SDK](https://github.com/sonydevworld/spresense). Alternatively, a pre-compiled application is provided and can be flashed to the Spresense board directly.

### Prerequisites

Follow the [Spresense SDK Getting Started Guide (CLI): Development environment](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_development_environment) instructions to set up the Spresense SDK for your development environment.

At a minimum, you should complete the following:
- **For Linux:** Follow step 2.1 to configure your serial port, install Spresense development tools, and download the Spresense SDK.
- **For Windows:** Follow step 2.2 to install MSYS2 and Spresense development tools, download the Spresense SDK, and install the USB serial driver.
- **For macOS:** Follow step 2.3 to install Apple development tools, Python3, wget, and Spresense development tools, download the Spresense SDK, and install the USB serial driver.

When flashing an application to your Spresense board, the Spresense development tools will check if your Spresense board's bootloader is up to date. If your bootloader needs to be updated, a warning will be displayed. Follow the [Spresense SDK Getting Started Guide (CLI): Flashing bootloader](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_flashing_bootloader) instructions (step 7) to download and install the necessary bootloader to your Spresense board, and retry flashing.

Once you have flash this application to your Spresense board, you will also need to connect to the serial port using a terminal application such as **minicom**, **TeraTerm**, or [Chrome Browser Serial Terminal](https://googlechromelabs.github.io/serial-terminal).

---

### Installing the pre-compiled application

A pre-compiled `nuttx.spk` application is provided in this repository and can be flashed directly to your Spresense board. **Note:** This repository, including the pre-compiled application, is provided for reference only and is not guaranteed to work. No warranty is provided. Please use at your own risk.

1. Clone or download and extract this repository to your local machine.

2. Connect your Spresense board to your machine, and check the port that corresponds to the serial connection. Refer to [USB connection](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_usb_connection) for instructions on how to find your Spresense board's serial port.

3. In your terminal, navigate to the `spresense/sdk` directory where your Spresense SDK was downloaded:

    ```bash
    cd /<path-to>/spresense/sdk
    ```

4. Run one of the following commands to flash the `nuttx.spk` application, making sure to modify the port number to match your port:

    - On Linux:

        ```bash
        ./tools/flash.sh -c /dev/ttyUSB0 -b 500000 /<path-to>/nuttx.spk
        ```

    - On Windows (using MSYS2):

        ```bash
        ./tools/flash.sh -c COM9 -b 500000 /<path-to>/nuttx.spk
        ```

    - On macOS:

        ```bash
        ./tools/flash.sh -c /dev/cu.SLAB_USBtoUART -b 500000 /<path-to>/nuttx.spk
        ```

Once the application is flashed, go to the **Connecting to the serial port and running AT commands** below.

---

### Compiling the application manually

If you prefer to compile the application yourself, you can follow the steps below. For additional information, refer to the [Spresense SDK Getting Started Guide (CLI): Build Method](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_build_method) documentation.

1. Clone or download and extract this repository to your local machine. This repository includes the following files:

     - `./spresense/examples/lte_at_cmd/*`
     - `./spresense/sdk/configs/examples/lte_at_cmd/*`

2. Copy the above files to the `spresense` directory where your Sprsense SDK was downloaded. Your `spresense` directory should look like this:

    ```text
    spresense/
      ├ examples/
      │   ├ lte_at_cmd/
      │   │   ├ Kconfig
      │   │   ├ lte_at_cmd_main.c
      │   │   ├ Make.defs
      │   │   └ Makefile
      │   └ ...
      ├ sdk/
      │   ├ configs/
      │   │   ├ examples/
      │   │   │   ├ lte_at_cmd/
      │   │   │   │   ├ defconfig
      │   │   │   │   └ README.txt
      │   │   │   └ ...
      │   │   └ ...
      │   └ ...
      └ ...
    ```

3. In your terminal, navigate to the `spresense/sdk` directory where your Spresense SDK was downloaded:

    ```bash
    cd /<path-to>/spresense/sdk
    ```

4. Configure the build method following the [Build Method](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_build_method) instructions:

    ```bash
    ./tools/config.py default
    ```

    If you see a warning that your bootloader needs to be updated, follow the [Flashing bootloader](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_flashing_bootloader) instructions (step 7) to download and install the necessary bootloader to your Spresense board.

5. Prepare the build:

    ```bash
    make clean
    ```

7. Check if files are in correct folders:

    ```bash
    ./tools/config.py -i examples/lte_at_cmd
    ```

8. Start the configure script:

    ```bash
    ./tools/config.py examples/lte_at_cmd
    ```

9. Start the make script:

    ```bash
    make
    ```

Finally, follow the steps in the **Installing the pre-compiled application** section above to flash the application to your Spresense board.

Then go to the **Connecting to the serial port and running AT commands** below.

---

### Connecting to the serial port and running AT commands

1. Connect your Spresense board to your machine, and check the port that corresponds to the serial connection. Refer to [USB connection](https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_usb_connection) for instructions on how to find your Spresense board's serial port.

2. Using a terminal application such as **minicom**, **TeraTerm**, or [Chrome Browser Serial Terminal](https://googlechromelabs.github.io/serial-terminal), connect to the serial port. You should see output like this:

    ```
    <CONNECTED>

    NuttShell (NSH) NuttX-10.2.0
    nsh>
    ```

3. Enter `lte_sysctl start` to start the LTE-M Extension Board module:

    ```
    nsh> lte_sysctl start
    ```

4. Enter `lte_at_cmd` followed by a suitable AT command to pass the AT command to the module. The response from the module will be returned. For example:

    - Output module information using `ATI`:

        ```
        nsh> lte_at_cmd ATI
        ATI

        Manufacturer: Murata Manufacturing Co., Ltd.
        Model: LBAD0XX1SC-DM
        Revision: RK_03_00_00_00_04121_001

        OK
        ```

    - Output PDP context configuration using `AT+CGDCONT?`:

        ```
        nsh> lte_at_cmd AT+CGDCONT?
        AT+CGDCONT?

        +CGDCONT: 1,"IP","soracom.io",,0,0,0,0,0,,0,,,,,

        OK
        ```

    - Enable EPS network registration and location information URC:

        ```
        nsh> lte_at_cmd AT+CEREG=2
        AT+CEREG=2

        OK
        ```

    - Check EPS network registration status:

        ```
        nsh> lte_at_cmd AT+CEREG?
        AT+CEREG?

        +CEREG: 2,0

        OK
        ```

    - Enable the internal `eth0` interface:

        ```
        nsh> ifup eth0
        ifup eth0...OK
        ```

    - Check EPS nework registration status again:

        ```
        nsh> lte_at_cmd AT+CEREG?
        AT+CEREG?

        +CEREG: 5,5,"05D4","019B6403",7

        OK
        ```

---

## Disclaimers

The code in this repository are examples only and are not guaranteed to work. No warranty is provided. In addition, the content of this code example is not intended for commercial use. Please use them at your own risk.