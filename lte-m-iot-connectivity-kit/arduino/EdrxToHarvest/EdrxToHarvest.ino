/**
 * SORACOM LTE-M Spresense eDRX to Soracom Harvest
 * v0.99 - 07/30/2023
 *
 * This code demonstrates eDRX values pushed from Soracom to a Sony Spresense device with LTE Extension board.
 * Typically, eDRX values will be requested by a device when connecting to a network. This demo shows how
 * eDRX values can also be pushed from Soracom to the device, allowing for network-side control of eDRX.
 *
 * Requirements:
 * - A Soracom SIM installed in the Sony Spresense LTE Extension board
 * - A Soracom group with the following settings:
 *    - User-configured eDRX and PTW values
 *    - Soracom Harvest service enabled
 *    - The above Soracom SIM added to the group
 *
 * With this Arduino sketch, the Sony Spresense device will:
 * - Enable eDRX and set the initial eDRX and PTW values to be requested by the device
 * - Attach to a network and receive eDRX and PTW values from Soracom as configured in the Soracom group
 * - Report the current eDRX and PTW values in the serial output and also send them to Soracom Harvest via UDP
 * - Idle for 300 seconds so a power measurement device like Qoitech Otii Arc can be used to measure power
 *   consumption during the eDRX Cycle and confirm eDRX behavior
 * - Restart to re-attach to network and receive new eDRX and PTW values from Soracom (if changed)
 *
 * This ino file code should be flashed to Sony Spresense with LTE-M extension via Arduino SDK.
 *
 * For information on how to configure eDRX and PTW values in a Soracom group, please submit a support request
 * to the Soracom support team, or contact Soracom at: https://www.soracom.io/contact
 *
 * ====================================================================================================================
 *
 * Code snips in this demo are from:
 * https://ja.stackoverflow.com/questions/74425/spresense-lte拡張ボードでlteとadを使うとlte-shutdownでエラーになる
 *
 * TODO: There is no need to use analogRead for this demo, so I removed it, therefore official non-beta branch might be okay to use.
 * SPRESENSE beta release (2023/03/18)
 * was used to get rid of above mentioned crashdump which didn't help :-(
 * therefore fallen back to official release
 *
 * ====================================================================================================================
 *
 * Additional Resources
 *
 * LTE connection and eDRX settings:
 * - Spresense Hardware Documentation - LTE Extension Board: https://developer.sony.com/spresense/development-guides/hw_docs_lte_en.html
 * - LTE Library API: https://developer.sony.com/spresense/spresense-api-references-sdk/group__lte.html
 * - LTEAccessProvider Class Reference: https://developer.sony.com/spresense/spresense-api-references-arduino/classLTEAccessProvider.html
 * - lte_edrx_setting Struct Reference: https://developer.sony.com/spresense/spresense-api-references-sdk/structlte__edrx__setting.html
 *
 * UDP Client:
 * - LTEUDP Class Reference: https://developer.sony.com/spresense/spresense-api-references-arduino/classLTEUDP.html
 *
 * Power Management:
 * - LowPowerClass Class Reference: https://developer.sony.com/spresense/spresense-api-references-arduino/classLowPowerClass.html
 * - Power Management and Power Domain Reference: https://developer.sony.com/spresense/development-guides/sdk_developer_guide_en.html#_power_management
 * - lte_hibernation example: https://github.com/sonydevworld/spresense/blob/master/examples/lte_hibernation/lte_hibernation_main.c
*/



#include <LTE.h>
#include <LowPower.h>
#include "lte/lte_api.h"

#define LTE_APN       "soracom.io"            // Replace your APN
#define LTE_USER_NAME "sora"                  // Replace with your username
#define LTE_PASSWORD  "sora"                  // Replace with your password
#define LTE_IP_TYPE   (LTE_NET_IPTYPE_V4)
#define LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_CHAP)
#define LTE_RAT       (LTE_NET_RAT_CATM)
LTE lteAccess;
LTEUDP lteUdp;

char host[] = "harvest.soracom.io";           // Your server hostname
int port = 8514;                              // Your server port
int ad_value = 0;

/**
 * eDRX Cycle (T-eDRX) lengths for S1 mode
 *
 * Devices are required to periodically wake up and listen to the network
 * for any incoming data. Setting a higher eDRX Cycle increases the duration
 * between waking up, allowing for extended time where the device is not
 * listening to the network, and therefore reduces power consumption.
 *
 * Reference: TS 24.008 clause 10.5.5.32
 *
 * Bits 4-1   Duration          Parameter to use      Value
 * ========================================================
 * 0000       5.12 seconds      LTE_EDRX_CYC_512      0
 * 0001       10.24 seconds     LTE_EDRX_CYC_1024     1
 * 0010       20.48 seconds     LTE_EDRX_CYC_2048     2
 * 0011       40.96 seconds     LTE_EDRX_CYC_4096     3
 * 0100       61.44 seconds     LTE_EDRX_CYC_6144     4
 * 0101       81.92 seconds     LTE_EDRX_CYC_8192     5
 * 0110       102.4  seconds    LTE_EDRX_CYC_10240    6
 * 0111       122.88 seconds    LTE_EDRX_CYC_12288    7
 * 1000       143.36 seconds    LTE_EDRX_CYC_14336    8
 * 1001       163.84 seconds    LTE_EDRX_CYC_16384    9
 * 1010       327.68 seconds    LTE_EDRX_CYC_32768    10
 * 1011       655.36 seconds    LTE_EDRX_CYC_65536    11
 * 1100       1310.72 seconds   LTE_EDRX_CYC_131072   12
 * 1101       2621.44 seconds   LTE_EDRX_CYC_262144   13
 * 1110       5242.88 seconds   LTE_EDRX_CYC_524288   14    ** Only valid for NB1 mode
 * 1111       10485.76 seconds  LTE_EDRX_CYC_1048576  15    ** Only valid for NB1 mode
 */

/**
 * Paging Time Window (T-PTW) lengths for WB-S1 mode
 *
 * This parameter controls how long the device continues to listen to the
 * network for any incoming data each time it wakes up. Setting a lower
 * Paging Time Window reduces the time the device is awake and therefore
 * reduces power consumption, but affects device reachability.
 *
 * Reference: TS 24.008 clause 10.5.5.32
 *
 * Bits 8-5   Duration          Parameter to use      Value
 * ========================================================
 * 0000       1.28 seconds      LTE_EDRX_PTW_128      0
 * 0001       2.56 seconds      LTE_EDRX_PTW_256      1
 * 0010       3.84 seconds      LTE_EDRX_PTW_384      2
 * 0011       5.12 seconds      LTE_EDRX_PTW_512      3
 * 0100       6.4 seconds       LTE_EDRX_PTW_640      4
 * 0101       7.68 seconds      LTE_EDRX_PTW_768      5
 * 0110       8.96 seconds      LTE_EDRX_PTW_896      6
 * 0111       10.24 seconds     LTE_EDRX_PTW_1024     7
 * 1000       11.52 seconds     LTE_EDRX_PTW_1152     8
 * 1001       12.8 seconds      LTE_EDRX_PTW_1280     9
 * 1010       14.08 seconds     LTE_EDRX_PTW_1408     10
 * 1011       15.36 seconds     LTE_EDRX_PTW_1536     11
 * 1100       16.64 seconds     LTE_EDRX_PTW_1664     12
 * 1101       17.92 seconds     LTE_EDRX_PTW_1792     13
 * 1110       19.20 seconds     LTE_EDRX_PTW_1920     14
 * 1111       20.48 seconds     LTE_EDRX_PTW_2048     15
 */



/**
 * Initial eDRX values
 * Reference: https://developer.sony.com/spresense/spresense-api-references-sdk/structlte__edrx__setting.html
 */
lte_edrx_setting_t edrxSettings = {
  .act_type = LTE_EDRX_ACTTYPE_WBS1, // Set Cat-M mode
  .enable = LTE_ENABLE,              // Enable eDRX
  .edrx_cycle = LTE_EDRX_CYC_6144,   // Set T-eDRX length to 61.44 seconds
  .ptw_val = LTE_EDRX_PTW_512        // Set T-PTW length to 5.12 seconds
};

void setup()
{
  // Initialize LEDs on main board
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Initialize serial
  Serial.begin(115200);

  // Initialize low power mode
  // This does not yet enable low power mode but just initializes RTC needed for low power mode
  LowPower.begin();

  // TODO: Consider if initial edrxSettings should be set once in setup() rather than every time in loop()
}

void loop()
{
  /**
   * Initialize LTE connection
   *
   * This block will run if the device is not attached and registered to a network.
   * If your SIM has a PIN, use lteAccess.begin("1234") with your PIN.
   */
  while (true) {
    // Power on the modem
    if (lteAccess.begin() == LTE_SEARCHING) {
      Serial.println("Modem start");

      char buffer[40];
      sprintf(buffer, "Setting eDRX values: eDRX=%u, PTW=%u...", edrxSettings.edrx_cycle, edrxSettings.ptw_val);
      Serial.println(buffer);

      // Set eDRX values
      // Reference: https://developer.sony.com/spresense/spresense-api-references-sdk/group__lte.html#ga34a4e45f4951a93dfcb680068fdcff03
      int err = lte_set_edrx_sync(&edrxSettings);
      if (err) {
        Serial.println("Error setting eDRX values (error number: " + String(err) + ")");
      }

      // Attach to network
      // Note that the `false` parameter disables synchronous wait, so the method will immediately
      // return LTE_CONNECTING even though the device has not finished registering to a network.
      // The next while loop below will check that the device has registered to the network.
      // Reference: https://developer.sony.com/spresense/spresense-api-references-arduino/classLTEAccessProvider.html#a20e30738a266dda9e0031ddaa5e1e495
      if (lteAccess.attach(LTE_RAT, LTE_APN, LTE_USER_NAME, LTE_PASSWORD, LTE_AUTH_TYPE, LTE_IP_TYPE, false) == LTE_CONNECTING) {
        Serial.println("Attaching to network...");
        break;
      }

      Serial.println("An error occurred. Shutting down modem and trying again...");
      lteAccess.shutdown();
      sleep(1);
    }
  }

  // Check network registration status
  while (lteAccess.getStatus() != LTE_READY) {
    sleep(1);
  }
  Serial.println("Registered to network!");



  /**
   * Get configured eDRX values
   *
   * Although device will request the initial eDRX values when initializing
   * the LTE connection, the values can be overridden from the network side.
   * After the device registered to the network, we will check the eDRX values
   * to see what values were configured.
   */
  // Get network time
  unsigned long time = lteAccess.getTime();

  // Get eDRX values
  int err = lte_get_current_edrx_sync(&edrxSettings);
  if (err) {
    Serial.println("Error getting eDRX values (error number: " + String(err) + ")");
    // TODO: Handle whether to continue if eDRX values could not be retrieved
  }

  // Format a string for local serial output and also for UDP packet
  char buffer[33];
  sprintf(buffer, "TIME=%lu, eDRX=%u, PTW=%u", time, edrxSettings.edrx_cycle, edrxSettings.ptw_val);

  // Output to serial
  Serial.println(buffer);



  /**
   * Send UDP packet to server with the current eDRX configuration
   * Reference: https://developer.sony.com/spresense/spresense-api-references-arduino/classLTEUDP.html#a2045981e41c65d07426bfa31facbba1f
   */
  Serial.println("UDP start");
  if (lteUdp.begin(port) == 1) {
    if (lteUdp.beginPacket(host, port) == 1) {
      lteUdp.write(buffer, strlen(buffer));

      if (lteUdp.endPacket() == 1) {
        Serial.println("UDP send OK!");
        delay(100);
      } else {
        Serial.println("Error sending UDP packet");
      }
    } else {
      Serial.println("Error resolving host or port");
    }

    lteUdp.stop();
  }



  /**
   * Sleep for some time
   *
   * In normal usage, if there is no further data to send or receive, shutting
   * down the modem with this line will help reduce power consumption:
   *
   * lteAccess.shutdown();
   *
   * However, for this demo we want to observe that eDRX values have been applied
   * and that the changes in power consumption match the eDRX and PTW lengths.
   * Therefore, we will keep the modem powered on and let the device sit idle.
   *
   * During this time, we should be able to see that spikes in power consumption,
   * which correspond to the modem listening to the network, last for the PTW
   * length, and afterwards power consumption stays low for the remainder of the
   * eDRX Cycle length.
   */
  Serial.println("Sleeping for 300 seconds. Check your power measurement device to see if eDRX is applied.");
  sleep(300);

  Serial.println("Now going to deep sleep for 60 seconds before starting again...");
  LowPower.deepSleep(60);
}
