/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"

#include "shell.h"
#include "chprintf.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
#define TEST_WA_SIZE    THD_WORKING_AREA_SIZE(256)

static void
cmd_mem(BaseSequentialStream *chp, int argc, char *argv[])
{
   size_t n, size;

   (void)argv;

   if (argc > 0)
   {
      chprintf(chp, "Usage: mem\r\n");
      return;
   }
   n = chHeapStatus(NULL, &size);
   chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
   chprintf(chp, "heap fragments   : %u\r\n", n);
   chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void
cmd_threads(BaseSequentialStream *chp, int argc, char *argv[])
{
   static const char *states[] = {CH_STATE_NAMES};
   thread_t *tp;

   (void)argv;
   if (argc > 0)
   {
      chprintf(chp, "Usage: threads\r\n");
      return;
   }

   chprintf(chp, "    addr    stack prio refs     state\r\n");
   tp = chRegFirstThread();
   do
   {
      chprintf(chp, "%08lx %08lx %4lu %4lu %9s\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state]);
      tp = chRegNextThread(tp);
   } while (tp != NULL);
}

static void
cmd_test(BaseSequentialStream *chp, int argc, char *argv[])
{
   thread_t *tp;

   (void)argv;
   
   if (argc > 0)
   {
      chprintf(chp, "Usage: test\r\n");
      return;
   }

   tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriorityX(), TestThread, chp);
   if (tp == NULL)
   {
      chprintf(chp, "out of memory\r\n");
      return;
   }
   chThdWait(tp);
}

static void
cmd_write(BaseSequentialStream *chp, int argc, char *argv[])
{
   static uint8_t buf[] =
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\r\n";

   (void)argv;
   if (argc > 0)
   {
      chprintf(chp, "Usage: write\r\n");
      return;
   }

   while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
   {
      chSequentialStreamWrite(&SD1, buf, sizeof buf - 1);
   }
   chprintf(chp, "\r\n\nstopped\r\n");
}




static const ShellCommand commands[] = 
{
   {"mem", cmd_mem},
   {"threads", cmd_threads},
   {"test", cmd_test},
   {"write", cmd_write},
   {NULL, NULL}
};

static const ShellConfig shell_cfg1 = 
{
   (BaseSequentialStream*)&SD1,
   commands,
};

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg)
{

   (void)arg;
   chRegSetThreadName("blinker");
   while (true)
   {
      palClearPad(GPIOC, GPIOC_LED);
      chThdSleepMilliseconds(50);
      palSetPad(GPIOC, GPIOC_LED);
      chThdSleepMilliseconds(50);
   }
}

/*
 * Application entry point.
 */
int
main(void)
{
   thread_t*   shelltp = NULL;
   /*
    * System initializations.
    * - HAL initialization, this also initializes the configured device drivers
    *   and performs the board-specific initializations.
    * - Kernel initialization, the main() function becomes a thread and the
    *   RTOS is active.
    */
   halInit();
   chSysInit();

   /*
    * Activates the serial driver 2 using the driver default configuration.
    */
   sdStart(&SD1, NULL);

   shellInit();

   /*
    * Creates the blinker thread.
    */
   chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

   /*
    * Normal main() thread activity, in this demo it does nothing except
    * sleeping in a loop and check the button state.
    */
   while (true)
   {
      if(!shelltp)
      {
         shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
      }
      else if(chThdTerminatedX(shelltp))
      {
         chThdRelease(shelltp);
         shelltp = NULL;
      }
      chThdSleepMilliseconds(1000);
   }
}
