/****************************************************************************
 * lte_at_cmd
 * Implements simple at-cmd parser using lte_send_atcmd_sync
 *
 * Prerequisite:
 * - Installation and setup of Sony Spresense SDK
 *   https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html
 * - inclsuive flashing bootloader
 *   https://developer.sony.com/develop/spresense/docs/sdk_set_up_en.html#_flashing_bootloader
 *
 * Either flash nuttx.spk directly:
 * - flash nuttx.spk to device
 *   >tools/flash.sh -c /dev/ttyUSB0 -b 500000 ./<yourpath>/nuttx.spk
 *
 * Or setup example applications according to Sony Spresense website and add lte_at_cmd example
 * - try to get lte_http_get_sample working first
 *   https://developer.sony.com/develop/spresense/docs/sdk_tutorials_en.html#_lte_http_get_sample_application
 * - then copy the lte_at_cmd files to the example directories
 *   .../spresense/examples/lte_at_cmd
 *   .../spresense/sdk/configs/examples/lte_at_cmd
 * - add directory to example/Kconfig file: source "<yourpath>/spresense/examples/lte_at_cmd/Kconfig"
 * - follow build steps:
 *   >cd spresense/sdk
 *   >source tools/build-env.sh
 *   >tools/config.py examples/lte_at_cmd
 *   >make
 * - flash nuttx.spk to device
 *   >tools/flash.sh -c /dev/ttyUSB0 -b 500000 nuttx.spk
 * - open shell or minicom
 *   > minicom -D /dev/ttyUSB0 -b 115200
 *   > nsh> lte_sysctl start
 *   > nsh> lte_at_cmd ATI
 *
 * Usage: lte_at_cmd <AT-CMD>
 * Example: lte_at_cmd ATI
 * ATI
 *
 * Manufacturer: Murata Manufacturing Co., Ltd.
 * Model: LBAD0XX1SC-DM
 * Revision: RK_03_00_00_00_04121_001
 *
 * OK
 ****************************************************************************/
#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <lte/lte_api.h>

/****************************************************************************
 * app_show_errinfo
 * Description:
 *   Show error information.
 ****************************************************************************/

static void app_show_errinfo(void)
{
  int           ret;
  lte_errinfo_t info = {0};

  ret = lte_get_errinfo(&info);
  if (ret == 0)
    {
      if (info.err_indicator & LTE_ERR_INDICATOR_ERRCODE)
        {
          printf("err_result_code : %ld\n", info.err_result_code);
        }
      if (info.err_indicator & LTE_ERR_INDICATOR_ERRNO)
        {
          printf("err_no          : %ld\n", info.err_no);
        }
      if (info.err_indicator & LTE_ERR_INDICATOR_ERRSTR)
        {
          printf("err_string      : %s\n", info.err_string);
        }
    }
}

/****************************************************************************
 * do uppercase
 ****************************************************************************/

static void toupper_line(FAR char *line, int size)
{
  int i;
  for (i = 0; i < size; i++)
    {
      line[i] = (char)toupper(line[i]);
    }
}


/****************************************************************************
 * show_usage
 ****************************************************************************/
static void show_usage(FAR const char *progname)
{
  fprintf(stderr, "\nUSAGE: %s command\n", progname);
  fprintf(stderr, " lte_at_cmd <AT-CMD>\n");
  fprintf(stderr, "   Example: lte_at_cmd ATI\n");
  exit(0);
}

/****************************************************************************
 * lte_at_cmd_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
    FAR char *at_msg = NULL;
    FAR char *response = "";
    int respbufflen;
    int result;
    int at_sock;
    int len;
    int *p = &respbufflen;

    /**  Check arguments during app call */
    if (argc < 2)
    {
      show_usage(argv[0]);
      /**  with no arguments, show usage text */
    }

    /**  Formatting string (add \r and do UPPER CASE) and getting lenght */
    at_msg = argv[1];
    len = strlen(at_msg);

    strcat(at_msg, "\r");
    len = strlen(at_msg);
    toupper_line(at_msg,len);

    /**  send at_cmd */
    at_sock = lte_send_atcmd_sync(at_msg, len, response, respbufflen, p);

    /**  parse result */
    /**  Either it fails as not available, no at-cmd or because modem is still off */
    if (at_sock < 0)
    {
      printf("Failed to send at cmd:%d\n", at_sock);
      if (at_sock == -115)
         {
         printf("Please remember to start modem first via: lte_sysctl start\n");
         }
      if (at_sock == -22)
         {
         printf("Not an at-cmd. It should start with 'at'\n");
         }
      return ERROR;
    }

    /**  OUTPUT: sent at-cmd and reply from module */
    printf("%s\n", at_msg);
    printf("%s\n", response);

    /**  catch possible lte errors */
    lte_errinfo_t errinfo;
    result = lte_get_errinfo(&errinfo);

    if (result == LTE_RESULT_ERROR)
    {
      app_show_errinfo();
    }
    return 1;
}
