//////////////////////////////////////////////////////////////////////////////
//
//  main.c
//
//  Main
//
//  Main application file for Device
//
// The MIT License (MIT)
//
// Copyright (c) 2020, Thomas Bresson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////


/* ***************************    Includes     **************************** */

// Standard Includes
#include <stdint.h>
#include <stdbool.h>

// FreeRTOS Includes
#include "FreeRTOS.h"
#include "task.h"

// Library Includes

// Project Includes

// Module Includes
#include "eeprom.h"
#include "modem.h"
#include "message_handler.h"

/* ***************************   Definitions   **************************** */

#define MODEM_TASK_PRIORITY             3
#define MSG_HANDLER_TASK_PRIORITY       2

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

int main(void)
{
    // Setup the Hardware and init the tasks
    eepromInit();
    modemInit(MODEM_TASK_PRIORITY);
    msgHandlerInit(MSG_HANDLER_TASK_PRIORITY);

    // Tasks are setup, start the scheduler
    vTaskStartScheduler();
}

/* *************************   Private Functions   ************************ */
