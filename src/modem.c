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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// FreeRTOS Includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Library Includes

// Project Includes
#include "message_handler.h"

// Module Includes
#include "modem.h"

/* ***************************   Definitions   **************************** */

#define AT_CMD_INIT "AT+COMMAND INIT"

#define AT_CMD_RESPONSE_STR     "COMMAND_RESPONSE"
#define AT_STATUS_MSG_STR       "STATUS_MESSAGE"

#define MODEM_DATA_QUEUE_SIZE   5


#define TASK_NOTIF_DATA_FROM_MODEM  0x01
#define TASK_NOTIF_DATA_TO_MODEM    0x02

typedef enum
{
    E_MSGTYPE_INVALID = 0,
    E_MSGTYPE_COMMAND_RESPONSE,
    E_MSGTYPE_STATUS_MSG
}modemAtMsgType_t;

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static void modemTask(void *pvParameters);
static void modemHardwareInit(void);
static modemAtMsgType_t modemDetermineMsgType(const modemAtCmdData_t *p_cmd_type);
void modemBuildDataMessageFromAtData(const char *const at_message_str, msgData_t *p_msg);

static void UART3_Receive(void);

/* ***********************   File Scope Variables   *********************** */

static QueueHandle_t data_from_modem_q = NULL;

static QueueHandle_t data_to_modem_q = NULL;

static TaskHandle_t modem_task = NULL;

/* *************************   Public  Functions   ************************ */

void modemInit(const int task_priority)
{
    // Setup queue for receiving data from modem
    data_from_modem_q = xQueueCreate(MODEM_DATA_QUEUE_SIZE, sizeof(modemAtCmdData_t));
    assert(data_from_modem_q != NULL);

    // Setup queue for sending commands to modem from external modules
    data_to_modem_q = xQueueCreate(MODEM_DATA_QUEUE_SIZE, sizeof(modemAtCmdData_t));
    assert(data_from_modem_q != NULL);

    // Create the task for processing incoming data
    xTaskCreate(modemTask, "Modem Task", configMINIMAL_STACK_SIZE, void, task_priority, &modem_task);
    assert(modem_task != NULL);
}

// Interface to send data to the modem
// Generates and AT Data command and appends the data
void modemSendCommand(msgData_t *p_data_msg)
{
    // Convert the data to an AT command by appending the data to the AT command
    modemAtCmdData_t data;
    snprintf(data.modem_at_cmd_data, sizeof(data.modem_at_cmd_data) "AT+COMMAND %s", p_data_msg->msg);

    // Send the data to the queue
    xQueueSend(data_to_modem_q, &data, portMAX_DELAY);

    // Send a notification to wake the task
    xTaskNotify(modem_task, TASK_NOTIF_DATA_TO_MODEM, eSetBits);
}

/* *************************   Private Functions   ************************ */

// Modem task is responsible for handing the data coming back from the modem and sending out
// AT Commands to the modem that are sent to this module
static void modemTask(void *pvParameters)
{
    // Initialize the Modem
    modemHardwareInit();

    uint32_t notify_value = 0;
    modemAtCmdData_t msg;
    for(;;)
    {
        // Wait forever for a notification and return to wait state after processing data
        xTaskNotifyWait(0, UINT32_MAX, &notify_value, portMAX_DELAY);

        if((notify_value & TASK_NOTIF_DATA_FROM_MODEM) != 0)
        {
            while(xQueueReceive(data_from_modem_q, &msg, 0) == pdTRUE)
            {
                // Determine if the AT command coming from the modem is DATA or is a STATUS message
                modemAtMsgType_t type = modemDetermineMsgType(&msg);

                switch (type)
                {

                // If STATUS, handle internally
                case E_MSGTYPE_STATUS_MSG:
                    // TODO: Call status message handler to handle the status message

                    break;

                // If Command Response, send to message handler
                case E_MSGTYPE_COMMAND_RESPONSE:
                    {
                        // Create a message for the message handler
                        msgData_t data_message;

                        // Copy the data into the message
                        modemBuildDataMessageFromAtData(msg.modem_at_cmd_data, &data_message);

                        // Return a response to the message handler
                        msgHandlerModemMsg(&data_message);
                    }
                    break;

                default:
                    // An AT string was received of unknown type
                    break;
                }
            }
        }

        if((notify_value & TASK_NOTIF_DATA_TO_MODEM) != 0)
        {
            while(xQueueReceive(data_to_modem_q, &msg, 0) == pdTRUE)
            {
                // Data received from external source, send to modem
                UART_Send(UART_3, msg.modem_at_cmd_data);
            }
        }
    }
}


// Builds a message to be processed by the message handler
void modemBuildDataMessageFromAtData(const char *const at_message_str, msgData_t *p_msg)
{
    // Find the message length, this will help separate out the data
    int at_msg_len = strlen(at_message_str);

    // Find the data portion of the AT string skipping past "AT+COMMAND_RESSPONSE " in the string
    int num_bytes_to_skip =  sizeof("AT+" AT_CMD_RESPONSE_STR " ");
    char *p_data_str = (at_message_str + num_bytes_to_skip);

    // Copy the data string into the message
    strncpy(p_msg->msg, p_data_str, sizeof(p_msg->msg));
}

// Determines the type of AT data coming back based on the contents of the message
static modemAtMsgType_t modemDetermineMsgType(const modemAtCmdData_t *p_msg)
{
    modemAtMsgType_t rev_val = E_MSGTYPE_INVALID;

    // Check for Command Response
    if(strstr(p_msg->modem_at_cmd_data, AT_CMD_RESPONSE_STR) != NULL)
    {
        ret_val = E_MSGTYPE_COMMAND_RESPONSE;
    }

    // Check for Status Message
    if(strstr(p_msg->modem_at_cmd_data, AT_STATUS_MSG_STR) != NULL)
    {
        ret_val = E_MSGTYPE_STATUS_MSG;
    }

    // If no type is matched, return invalid type
    return ret_val;
}

// Function sets up the UART to communicate with the AT Device
static void modemHardwareInit(void)
{
    // Init the UART
    UART_Init(UART_3, &config_struct, etc);

    // Send some AT commands to the modem to initialize it
    UART_Send(UART_3, AT_CMD_INIT);
}

/* *************************  Interrupt Handlers  ************************* */


// An abstraction of a UART receive interrupt
static void UART3_Receive(void)
{
    static modemAtCmdData_t data;
    static int current_pos = 0;
x
    // Read data from UART and store to buffer
    UART_Read(UART3, &incoming_byte);
    data.modem_at_cmd_data[current_pos++] = incoming_byte;

    // Check to see if the message is complete by checking for a '\n'
    if(incoming_byte == '\n')
    {
        // Message is complete, send to task for processing
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(data_from_modem_q, &data, &xHigherPriorityTaskWoken);

        // Notify the task that data has been sent to queue
        xTaskNotifyFromISR(modem_task, TASK_NOTIF_DATA_FROM_MODEM, eSetBits, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        // Reset position of buffer
        current_pos = 0;
    }
}
