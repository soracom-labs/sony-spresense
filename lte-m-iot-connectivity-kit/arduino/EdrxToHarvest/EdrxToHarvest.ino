// SORACOM LTE-M Spresense eDRX to Harvest
// v0.99 - 07/30/2023
// code snips are taken from:
// https://ja.stackoverflow.com/questions/74425/spresense-lte拡張ボードでlteとadを使うとlte-shutdownでエラーになる
// SPRESENSE beta release (2023/03/18)
// was used to get rid of above mentioned crashdump which didn't help :-(
// therefore fallen back to official release
//
// 
/*
// ########################################################################################################################
// # This demo code is used to show eDRX value push from Soracom API to a Sony Spresense board with LTE-M extension board
// # according to soracom/private-beta-docs/docs/eDRX_and_PSM_timer-privatebeta-en  eDRX and PSM timer configuration
// #
// # Used Soracom SIM should be assigned to a group-id e.g.: 695f3ea3-41c9-4cd8-9f71-d45bcc624afe - 'eDRX'
// # Soracom Harvest should be enabled for the SIM
// # eDRX and PTW window values can be adapted as mentioned above in eDRX_and_PSM_timer-privatebeta-en
// # This ino file code should be flashed to Sony Spresense with LTE-M extension via Arduino SDK
// # It will do the following:
// # - an initial eDRX value is set for the device                                                                          mark: // Initial eDRX value setting  &  // enable eDRX
// # - device attaches to the LTE-M network and receives the eDRX values defined in used group-id in attach accept message  mark: // Attach
// # - device will fill a data buffer with current active eDRX parameters                                                   mark: // get eDRX value  & // get PTW value
// # - device will send these values together with timestamp to Soracom Harvest via UDP                                     mark: // send buffer to Soracom Harvest
// # - if device is connected to a power measurement device like Qoitech Otii Arc a 300sec 
// #   waitting time will happen to show effect of eDRX value change                                                        mark: // do nothing for 300s to show eDRX value took effect
// # - device will be set to low power deep sleep mode for 1 min. and will then restart                                     mark: // sleep(60) in low power mode and start again
*/


#include <LTE.h>
#include <LowPower.h> 
#include "lte/lte_api.h"

#define LTE_APN       "soracom.io"  // replace your APN
#define LTE_USER_NAME "sora"        // replace with your username
#define LTE_PASSWORD  "sora"        // replace with your password
#define LTE_IP_TYPE (LTE_NET_IPTYPE_V4)
#define LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_CHAP)
#define LTE_RAT (LTE_NET_RAT_CATM)

char sendBuffer[] = "Hello Soracom!";                   // The string to send...will be overwritten

char  host[] = "harvest.soracom.io";
int   port = 8514;
LTE           lteAccess;
LTEUDP        lteUdp;
int   ad_value = 0;


/* eDRX-Cycle-Length-Value is defined in TS 24.008 clause 10.5.5.32.
bit 4-1	  Dec	 Duration
0 0 0 0	   0	     5.12 seconds  LTE_EDRX_CYC_512
0 0 0 1	   1	    10.24 seconds  LTE_EDRX_CYC_1024
0 0 1 0	   2	    20.48 seconds  LTE_EDRX_CYC_2048
0 0 1 1	   3	    40.96 seconds  LTE_EDRX_CYC_4096
0 1 0 0	   4	    61.44 seconds  LTE_EDRX_CYC_6144
0 1 0 1	   5	    81.92 seconds  LTE_EDRX_CYC_8192
0 1 1 0	   6	   102.4  seconds  LTE_EDRX_CYC_10240
0 1 1 1	   7	   122.88 seconds  LTE_EDRX_CYC_12288
1 0 0 0	   8	   143.36 seconds  LTE_EDRX_CYC_14336
1 0 0 1	   9	   163.84 seconds  LTE_EDRX_CYC_16384
1 0 1 0	  10	   327.68 seconds  LTE_EDRX_CYC_32768
1 0 1 1	  11	   655.36 seconds  LTE_EDRX_CYC_65536
1 1 0 0	  12	  1310.72 seconds  LTE_EDRX_CYC_131072
1 1 0 1	  13	  2621.44 seconds  LTE_EDRX_CYC_262144
1 1 1 0	  14	  5242.88 seconds   only in NB1 mode
1 1 1 1	  15	 10485.76 seconds   only in NB1 mode
*/
//.ptw_val = LTE_EDRX_PTW_256 /* Modem起床時2.56秒起き続ける */
/*
bit 8-5	  Dec	Duration for WB-S1/WB-N1 mode
0 0 0 0  	 0	   1.28 seconds - LTE_EDRX_PTW_128
0 0 0 1	   1	   2.56 seconds - LTE_EDRX_PTW_256
0 0 1 0	   2	   3.84 seconds - LTE_EDRX_PTW_384
0 0 1 1	   3	   5.12 seconds - LTE_EDRX_PTW_512
0 1 0 0	   4	   6.4 seconds  - LTE_EDRX_PTW_640
0 1 0 1	   5	   7.68 seconds - LTE_EDRX_PTW_768
0 1 1 0	   6	   8.96 seconds - LTE_EDRX_PTW_896
0 1 1 1	   7	  10.24 seconds - LTE_EDRX_PTW_1024
1 0 0 0	   8	  11.52 seconds - LTE_EDRX_PTW_1152
1 0 0 1	   9	  12.8 seconds  - LTE_EDRX_PTW_1280
1 0 1 0	  10  	14.08 seconds - LTE_EDRX_PTW_1408
1 0 1 1	  11  	15.36 seconds - LTE_EDRX_PTW_1536
1 1 0 0	  12  	16.64 seconds - LTE_EDRX_PTW_1664
1 1 0 1	  13  	17.92 seconds - LTE_EDRX_PTW_1792
1 1 1 0	  14  	19.20 seconds - LTE_EDRX_PTW_1920
1 1 1 1	  15  	20.48 seconds - LTE_EDRX_PTW_2048
*/

/* Initial eDRX value setting */
lte_edrx_setting_t edrxSettings = {
      .act_type = LTE_EDRX_ACTTYPE_WBS1, /* Cat.M */
      .enable = LTE_ENABLE,              /* eDRX 有効化 */
      .edrx_cycle = LTE_EDRX_CYC_6144,   /* 655.36秒間隔でModem起床 - value 4*/
      .ptw_val = LTE_EDRX_PTW_512        /* Modem起床時2.56秒起き続ける - value 3 */
};

void setup()
{
  // Initialize LEDs on main board
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  /* Initialize Serial */
  Serial.begin(115200);
  LowPower.begin();
}

void loop()
{
  //************************************************************
  // LTE setting
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  // Serial.println("LTE Start");
  while (true) {
    if (lteAccess.begin() == LTE_SEARCHING) {
      int ret = lte_set_edrx_sync(&edrxSettings);   // enable eDRX
          if (ret) {
                    Serial.println("Error to set a eDRX parameter. ret" + String(ret));
                   }
      if (lteAccess.attach(LTE_RAT, LTE_APN, LTE_USER_NAME, LTE_PASSWORD, LTE_AUTH_TYPE, LTE_IP_TYPE, false) == LTE_CONNECTING) {     // Attach
        Serial.println("Attempting to connect to network.");
        break;
      }
      Serial.println("An error occurred, shutdown and try again.");
      lteAccess.shutdown();
      sleep(1);
    }
  }

  while (LTE_READY != lteAccess.getStatus()) {
    sleep(1);
  }

  ad_value = analogRead(PIN_A4);     // don't know why I need this, but without we crash


  Serial.println("UDP start");
  if (lteUdp.begin(port)  == 1) {
    if (lteUdp.beginPacket(host, port) == 1) {
      char ad_str[10];
      sprintf(ad_str, "ad=%04x", ad_value); 
      long time = lteAccess.getTime();       // get time value to be sent with eDRX data
  
   Serial.println("TIME: " + String(time));
   char *a = itoa(time,sendBuffer,10);
   // get eDRX values finally pushed from network
   int drx = lte_get_current_edrx_sync(&edrxSettings);
   int derxcyc = edrxSettings.edrx_cycle;     // get eDRX value
   int ptwval = edrxSettings.ptw_val;         // get PTW value

   // format buffer to send data
   char b[] = " eDRX: "; 
   String c = String(derxcyc);  // add eDRX value

   char e[] = " PTW: "; 
   String f = String(ptwval);   // add PTW value
  
   String d = b + c + e + f;    //put all together
   
   d.toCharArray(b,20);        //push to array
   strcat(sendBuffer, b);      
   
   Serial.println("BUFFER to send:");
   Serial.println(sendBuffer);

   // send buffer to Soracom Harvest
      lteUdp.write(sendBuffer, strlen(sendBuffer)); 

      //lteUdp.write(ad_str, 7);
      if (lteUdp.endPacket() == 1) {
        Serial.println("UDP Data Send OK");
        delay(100);
      } else {
        Serial.println("UDP Data Send NG(endPacket)");
      }
    } else {
      Serial.println("UDP Data make NG(beginPacket)");
    }
    lteUdp.stop();         // end of UDP session
    Serial.println("UDP end");
  }
  
  //lteAccess.shutdown(); // optional shtdown

  //********************* let device adapt to network pushed eDRX value  ***************************************
  Serial.println("Check if eDRX is applied");
  sleep(300);               // do nothing for 300s to show eDRX value took effect
  Serial.println("LTE End - low power mode");
    //********************* low power mode ***********************************************************************
  LowPower.deepSleep(60); // ★sleep(60) in low power mode and start again
}

