*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M3 STM32F103.                            **
*****************************************************************************

** TARGET **

The demo runs on an Chinese Version of STM32F103C8 based board.

** The Demo **

The demo flashes the board LED using a thread. It also shell interfaces via SD1 interface
providing various commands to test the system

** Build Procedure **

The demo has been tested by using the free Codesourcery GCC-based toolchain
and YAGARTO.
Just modify the TRGT line in the makefile in order to use different GCC ports.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
