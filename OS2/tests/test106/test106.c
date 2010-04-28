#define INCL_BASE



#include <os2.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <perfutil.h>



/*

   Convert 8-byte (low, high) time value to double

*/

#define LL2F(high, low) (4294967296.0*(high)+(low))





/* This is a 1 processor example */



void main (int argc, char *argv[])

{

    APIRET      rc;

    int         i, iter, sleep_sec;

    double      ts_val, ts_val_prev;

    double      idle_val, idle_val_prev;

    double      busy_val, busy_val_prev;

    double      intr_val, intr_val_prev;

    CPUUTIL     CPUUtil;



    if ((argc < 2) || (*argv[1] < '1') || (*argv[1] > '9')) {

        fprintf(stderr, "usage: %s [1-9]\n", argv[0]);

        exit(0);

    }

    sleep_sec = *argv[1] - '0';



    iter = 0;

    do {

        rc = DosPerfSysCall(CMD_KI_RDCNT,(ULONG) &CPUUtil,0,0);

        if (rc) {

            fprintf(stderr, "CMD_KI_RDCNT failed rc = %d\n",rc);

            exit(1);

        }

        ts_val = LL2F(CPUUtil.ulTimeHigh, CPUUtil.ulTimeLow);

        idle_val = LL2F(CPUUtil.ulIdleHigh, CPUUtil.ulIdleLow);

        busy_val = LL2F(CPUUtil.ulBusyHigh, CPUUtil.ulBusyLow);

        intr_val = LL2F(CPUUtil.ulIntrHigh, CPUUtil.ulIntrLow);



        if (iter > 0) {

            double  ts_delta = ts_val - ts_val_prev;

            printf("idle: %4.2f%%  busy: %4.2f%%  intr: %4.2f%%\n",

                   (idle_val - idle_val_prev)/ts_delta*100.0,

                   (busy_val - busy_val_prev)/ts_delta*100.0,

                   (intr_val - intr_val_prev)/ts_delta*100.0);

        }



        ts_val_prev = ts_val;

        idle_val_prev = idle_val;

        busy_val_prev = busy_val;

        intr_val_prev = intr_val;



        iter++;

        DosSleep(1000*sleep_sec);



    } while (1);

}
