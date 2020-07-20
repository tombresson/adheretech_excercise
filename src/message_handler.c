//////////////////////////////////////////////////////////////////////////////
//
//  message_handler.c
//
//  Message Handler
//
//  Module description in message_handler.h
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
#include "eeprom.h"
#include "modem.h"

// Module Includes
#include "message_handler.h"

/* ***************************   Definitions   **************************** */

#define MSG_HDLR_MAX_MESSAGES_FROM_SERIAL       10

#define MSG_HDLR_MAX_MESSAGES_FROM_MODEM        5

#define TASK_NOTIF_SERIAL_MSG_RX                0x01
#define TASK_NOTIF_MODEM_MSG_RX                 0x02


typedef enum
{
    E_DEST_NONE = 0,
    E_DEST_UART1,
    E_DEST_UART2,
    E_DEST_EEPROM
}msgDestination_t;

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static void msgHandlerTask(void *pvParameters);
static msgDestination_t msgDetermineDestination(const msgData_t *p_msg);

static void UART1_Receive(void);
static void UART2_Receive(void)
static void UART_Receive(msgData_t *p_data, int *p_curr_idx, char incoming_byte);

/* ***********************   File Scope Variables   *********************** */

static QueueHandle_t modem_messages_q = NULL;

static QueueHandle_t serial_messages_q = NULL;

static TaskHandle_t message_task = NULL;

/* *************************   Public  Functions   ************************ */

// Initializes the message handler
void msgHandlerInit(const int task_priority)
{
    // Initialize the queue for handling the incoming message data from the serial interface(s)
    serial_messages_q = xQueueCreate(MSG_HDLR_MAX_MESSAGES_FROM_SERIAL, sizeof(msgData_t));
    assert(serial_messages_q != NULL);

    // Initialize the queue for handling messages from the modem
    modem_messages_q = xQueueCreate(MSG_HDLR_MAX_MESSAGES_FROM_MODEM, sizeof(msgData_t));
    assert(modem_messages_q != NULL);

    xTaskCreate(msgHandlerTask, "Message Handler", configMINIMAL_STACK_SIZE, void, task_priority, &message_task);
    assert(message_task != NULL);
}

// Receive a message data from the modem
void msgHandlerModemMsg(const msgData_t *p_msg_data)
{
    // Send the data and notify the task
    xQueueSend(modem_messages_q, p_msg_data, portMAX_DELAY);
    xTaskNotify(message_task, TASK_NOTIF_MODEM_MSG_RX, eSetBits);
}

/* *************************   Private Functions   ************************ */

static void msgHandlerTask(void *pvParameters)
{
    // Init the serial interfaces
    UART_Init(UART_1, &config_struct, etc);
    UART_Init(UART_2, &config_struct, etc);

    uint32_t notify_value = 0;
    msgData_t msg;
    for(;;)
    {
        // Wait indefintiely for data to come in on either serial interface or from the modem
        xTaskNotifyWait(0, UINT32_MAX, &notify_value, portMAX_DELAY);


        // Handle messages from Serial Interfaces
        if((notify_value & TASK_NOTIF_SERIAL_MSG_RX) != 0)
        {
            while(xQueueReceive(serial_messages_q, &msg, 0) == pdTRUE)
            {
                // Send message over to Modem Module
                modemSendCommand(&msg);
            }
        }

        // Handle message data from the modem
        if((notify_value & TASK_NOTIF_MODEM_MSG_RX) != 0)
        {
            // Handle all the messages coming from the modem
            while(xQueueReceive(modem_messages_q, &msg, 0) == pdTRUE)
            {
                // Determine if the data needs to go to one of the serial ports or the EEPROM
                msgDestination_t dest = msgDetermineDestination(&msg);

                // Send the data to it's proper destination
                switch(dest)
                {
                    case E_DEST_UART1:
                        UART_Send(UART1, msg.msg);
                        break;

                    case E_DEST_UART2:
                        UART_Send(UART2, msg.msg);
                        break;

                    case E_DEST_EEPROM:
                    {
                        // Determine num bytes to write
                        int msg_len = strnlen(msg.msg, (sizeof(msg.msg)));

                        // Somehow determine the address at which the EEPROM data belongs
                        // This might be a function of some kind of NVM data manager module
                        uint32_t address = nvmDataGetAddr();
                        eepromWriteBytes(address, msg.msg, msg_len);
                        break;
                    }

                    default:
                        // Shouldn't happen
                        assert(false);
                }
            }
        }
    }
}


static msgDestination_t msgDetermineDestination(const msgData_t *p_msg)
{
    // Determine where the message would go, based on some critera and return a valid destination
}

/* *************************  Interrupt Handlers  ************************* */

// An abstraction of a UART Rx interrupt
// The intention of a function like this would be to read the bytes coming in from the
// UART and write them to a buffer, when the entire message is received send it to the task
static void UART1_Receive(void)
{
    static msgData_t data;
    static int current_pos = 0;

    // Write the incoming data to the buffer
    char incoming_byte = UART_ReadData(UART1);
    UART_Receive(&data, &current_pos, incoming_byte);
}

// An abstraction of a UART Rx interrupt
// The intention of a function like this would be to read the bytes coming in from the
// UART and write them to a buffer, when the entire message is received send it to the task
static void UART2_Receive(void)
{
    static msgData_t data;
    static int current_pos = 0;

    // Write the incoming data to the buffer
    char incoming_byte = UART_ReadData(UART2);
    UART_Receive(&data, &current_pos, incoming_byte);
}


// Process the incoming serial data
static void UART_Receive(msgData_t *p_data, int *p_curr_idx, char incoming_byte)
{
    p_data->msg[(*p_curr_idx)++] = incoming_byte;

    // Check to see if the message is complete by checking for a '\n'
    if(incoming_byte == '\n')
    {
        // Message is complete, send to task for processing
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(serial_messages_q, p_data->msg, &xHigherPriorityTaskWoken);

        // Notify the task that data has been sent to queue
        xTaskNotifyFromISR(message_task, TASK_NOTIF_SERIAL_MSG_RX, eSetBits, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        // Reset position of buffer
        *p_curr_idx = 0;
    }
}
