/**
 * LteUdpSoracom.ino - Example for UDP client using LTE-M
 * Soracom Spresense LTE extension board
 * based on:
 * Copyright 2019, 2021, 2022 Sony Semiconductor Solutions Corporation
 *
 * Copyright SORACOM
 * This software is released under the MIT License, and libraries used by these sketches 
 * are subject to their respective licenses.
 * See also: https://github.com/soracom-labs/soracom-spresense-lte-m-iot-connectivity-kit/blob/main/README.md
 */

// Libraries
#include <LTE.h>
#include <LTEUDP.h>


/**
 * Cellular network settings
 */
#define APP_LTE_APN "soracom.io"
#define APP_LTE_USERNAME "sora"
#define APP_LTE_PASSWORD "sora"

// Soracom provides IPv4 connectivity. You can also change to one of
// the following if your provider requires different settings:
// - IPv4: LTE_NET_IPTYPE_V4
// - IPv4v6: LTE_NET_IPTYPE_V4V6
// - IPv6: LTE_NET_IPTYPE_V6
#define APP_LTE_IP_TYPE (LTE_NET_IPTYPE_V4)

// Soracom accepts both CHAP and PAP authentication methods by default.
// You can also change to one of the following if your provider requires
// different settings:
// - CHAP: LTE_NET_AUTHTYPE_CHAP
// - PAP: LTE_NET_AUTHTYPE_PAP
// - NONE: LTE_NET_AUTHTYPE_NONE
#define APP_LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_CHAP)

// Soracom provides LTE-M (LTE Cat-M1) coverage. You can also change to
// one of the following if your provider requires different settings:
// - LTE-M: LTE_NET_RAT_CATM
// - NB-IoT: LTE_NET_RAT_NBIOT
#define APP_LTE_RAT (LTE_NET_RAT_CATM)



/**
 * Server settings
 *
 * In this example, we will send a simple string to Soracom Harvest
 * using UDP to test the cellular connection.
 * It is possible to change the destination to Soracom Unified Endpoint
 * by moving the comment slash from server and port to the Harvest server and port 
 */
char server[] = "harvest.soracom.io"; // Soracom Harvest address
int port = 8514;                      // Soracom Harvest UDP port

//char server[] = "unified.soracom.io"; // Soracom Unified endpoint address
//int port = 23080;                      // Soracom Unified endpoint UDP port

char sendBuffer[] = "Hello Soracom!"; // The string to send


/**
 * Initialize the library instance
 */
LTE lteAccess;
LTEClient client;
LTEUDP udp;
LTEScanner scannerNetworks;
LTEModem modem;
LTENetworkRatType RAT = LTE_NET_RAT_UNKNOWN;



/**
 * Setup
 *
 * This section initializes the LTE connection, attaches to the data
 * network service, shows network connection details, then starts a
 * UDP client and sends the sample data.
 */
void setup()
{
  // Initialize LEDs on main board
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only.
  }
  Serial.println("Starting...");

  // Turn on LED #1 (LED0) to indicate that the program is running
  digitalWrite(LED0, HIGH);

  Serial.println("=========== APN information ===========");
  Serial.print("Access Point Name  : ");
  Serial.println(APP_LTE_APN);
  if (APP_LTE_AUTH_TYPE != LTE_NET_AUTHTYPE_NONE) {
    Serial.print("Username           : ");
    Serial.println(APP_LTE_USERNAME);
    Serial.print("Password           : ");
    Serial.println(APP_LTE_PASSWORD);
  }
  Serial.print("Authentication Type: ");
  Serial.println(APP_LTE_AUTH_TYPE == LTE_NET_AUTHTYPE_CHAP ? "CHAP" :
                 APP_LTE_AUTH_TYPE == LTE_NET_AUTHTYPE_PAP ? "PAP" : "NONE");

  // Connect to network
  while (true) {
    // Power on the modem and enable the radio function
    if (lteAccess.begin() != LTE_SEARCHING) {
      Serial.println("Could not transition to LTE_SEARCHING.");
      Serial.println("Please check the status of the LTE board.");
      for (;;) {
        sleep(1); // Do nothing forevermore
      }
    }

    // Start the connection process to the data network
    if (lteAccess.attach(APP_LTE_RAT,
                         APP_LTE_APN,
                         APP_LTE_USERNAME,
                         APP_LTE_PASSWORD,
                         APP_LTE_AUTH_TYPE,
                         APP_LTE_IP_TYPE) == LTE_READY) {
      Serial.println();
      Serial.println("Successfully attached to network!");
      Serial.println();

      // Turn on LED #2 (LED1) to indicate cellular network connection
      digitalWrite(LED1, HIGH);

      // Print out connection information
      Serial.println("=========== Connection info ===========");
      Serial.print("Current carrier: ");
      Serial.println(scannerNetworks.getCurrentCarrier());

      Serial.print("IP address: ");
      Serial.println(lteAccess.getIPAddress());

      Serial.print("Current RAT: ");
      RAT = modem.getRAT();
      Serial.println(RAT == LTE_NET_RAT_CATM ? "LTE-M (LTE Cat-M1)" :
                     RAT == LTE_NET_RAT_NBIOT ? "NB-IoT" : "Unknown type [" + String(RAT) + "]");

      Serial.print("Signal Strength: ");
      Serial.print(scannerNetworks.getSignalStrength());
      Serial.println(" [dBm]");

      // Exit this while loop
      break;
    }

    /* If the following logs occur frequently, one of the following might be a cause:
     * - APN settings are incorrect
     * - SIM is not inserted correctly
     * - RAT type is not supported
     * - Rejected from LTE network
     */
    Serial.println("An error has occurred. Shutting down and retrying the network attach process after 1 second...");
    
    // Turn on LED #2 (LED1) to indicate cellular network connection try
    digitalWrite(LED1, HIGH);

    lteAccess.shutdown();
    sleep(1);
    
    // Turn off LED #2 (LED1) to indicate shutdown cellular network connection
    digitalWrite(LED1, LOW);

  }

  // Start a UDP client with local port 8000 to send and receive data
  Serial.println("=========== UDP client ===========");
  if (udp.begin(8000)) {
    // Turn on LED #3 (LED2) to indicate that the UDP client is ready
    digitalWrite(LED2, HIGH);

    Serial.println();
    Serial.println("UDP client listening on port 8000");
  }

  // Send sample data to server
  if (udp.beginPacket(server, port)) {
    // Turn on LED #4 (LED3) to indicate that device has connected to server
    digitalWrite(LED3, HIGH);

    Serial.println();
    Serial.print("Connected to ");
    Serial.println(server);

    udp.write(sendBuffer, 14); // TODO: Buffer length is hardcoded to match the "Hello Soracom!" string length. This should be changed to use a length function.
    udp.endPacket();

    Serial.println("Data sent to Soracom Harvest!");
    Serial.println();

    // Turn off LED #4 (LED3) to indicate that data has been sent
    digitalWrite(LED3, LOW);
  }
}



/**
 * Loop
 *
 * This section loops until a response is received from the server,
 * then outputs the response, stops the UDP client, and ends the program.
 */
void loop()
{
  // Check if there are incoming bytes available from the server.
  // Soracom Harvest will send a 201 response to confirm that the data
  // was successfully received.
  int pSize = udp.parsePacket();

  if (pSize) {
    Serial.print("Received packet! Size: ");
    Serial.println(pSize);

    char receiveBuffer[255];
    int len = udp.read(receiveBuffer, 255);
    if (len > 0) {
      receiveBuffer[len] = '\0';
    }
    Serial.print("Server response: ");
    Serial.println(receiveBuffer);
    Serial.println();
    Serial.println("Now check Harvest Data to see the sample data.");

    // Stop the UDP client
    if (!udp.available()) {
      Serial.println();
      Serial.println("Disconnecting.");
      udp.stop();

      // Turn off LED #3 (LED2) to indicate that the UDP client has stopped
      digitalWrite(LED2, LOW);

        sleep(1); // Do nothing forevermore

        lteAccess.shutdown();
        // Turn off LED #2 (LED1) to indicate that LTE connection shutdown happened
        digitalWrite(LED1, LOW);

        // Turn off LED #1 (LED0) to indicate that the program is ended
        digitalWrite(LED0, LOW);

    }
  }
}
