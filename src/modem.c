//////////////////////////////////////////////////////////////////////////////
//
//  modem.c
//
//  Modem Driver
//
//  Module description in modem.h
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

// FreeRTOS Includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Library Includes

// Project Includes

// Module Includes

/* ***************************   Definitions   **************************** */

#define AT_CMD_INIT "AT+COMMAND INIT"

#define MODEM_DATA_QUEUE_SIZE   5


#define TASK_NOTIF_DATA_FROM_MODEM  0x01
#define TASK_NOTIF_DATA_TO_MODEM    0x02

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

/* ***********************   File Scope Variables   *********************** */

static QueueHandle_t data_from_modem_q = NULL;

static QueueHandle_t data_to_modem_q = NULL;

/* *************************   Public  Functions   ************************ */

void modemInit(const int task_priority, const int max_message_size)
{
    // Create the task for processing incoming data
    xTaskCreate(void, "Modem Task", configMINIMAL_STACK_SIZE, void, task_priority, void);

    // Setup queue for receiving data from modem
    data_from_modem_q = xQueueCreate(MODEM_DATA_QUEUE_SIZE, MODEM_DATA_MAX_SIZE);
    assert(data_from_modem_q != NULL);

    // Setup queue for sending commands to modem from external modules
    data_from_modem_q = xQueueCreate(MODEM_DATA_QUEUE_SIZE, MODEM_DATA_MAX_SIZE);
    assert(data_from_modem_q != NULL);
}

void modemSendCommand(const char *command_str)
{
    // Send the command string to the queue

    // Send a notification to wake the task
}

/* *************************   Private Functions   ************************ */

// Modem task is responsible for handing the data coming back from the modem and sending out
// AT Commands to the modem that are sent to this module
static void modemTask(void *pvParameters)
{
    // Initialize the Modem
    modemHardwareInit();

    uint32_t notify_value = 0;
    for(;;)
    {
        // Wait forever for a notification
        xTaskNotifyWait(0, UINT32_MAX, &notify_value, portMAX_DELAY);

        if((notify_value & TASK_NOTIF_DATA_FROM_MODEM) != 0)
        {
            // TODO: Data received from modem, send to message handler
        }

        if((notify_value & TASK_NOTIF_DATA_TO_MODEM) != 0)
        {
            // TODO: Data received from external source, send to modem
        }
    }
}

// Function sets up the UART to communicate with the AT Device
static void modemHardwareInit(void)
{
    // Init the UART
    UART_Init(UART_3, &config_struct, etc);

    // Send some AT commands to the modem to initialize it
    UART_Send(UART_3, AT_CMD_INIT);
}

// Handles data coming from the modem
static void modemAtDataHandler(void)
{

}

/* *************************  Interrupt Handlers  ************************* */


// An abstraction of a UART receive interrupt

static void UART_Receive(void)
