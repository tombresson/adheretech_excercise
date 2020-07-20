//////////////////////////////////////////////////////////////////////////////
//
//  message_handler.h
//
//  Message Handler
//
//  Receives data from the modem and routes the messages to external data interfaces or saves data
//  to an EEPROM.
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

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

/* ***************************    Includes     **************************** */

/* ***************************   Definitions   **************************** */

// Max size of any data string that could go to OR come from the modem
#define MSG_DATA_MAX_SIZE           20

/* ****************************   Structures   **************************** */

typedef struct
{
    char msg[MSG_DATA_MAX_SIZE];
}msgData_t;

/* ***********************   Function Prototypes   ************************ */

void msgHandlerInit(const int task_priority);
void msgHandlerModemMsg(const msgData_t *p_msg_data);

#endif /* MESSAGE_HANDLER_H */
