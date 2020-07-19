//////////////////////////////////////////////////////////////////////////////
//
//  eeprom.c
//
//  EEPROM Driver
//
//  Module description in eeprom.h
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

// Library Includes

// Project Includes

// Module Includes

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

// Initializes the SPI interface and
void eepromInit(void)
{
    SPI_Init();
}

bool eepromReadBytes(const uint32_t addr, uint8_t *bytes, const unsigned int num_bytes)
{
    // Send SPI commands to setup EEPROM for reading data at specified address

    // Read data out and write to data buffer
    bool result = true;
    for(int idx = 0; idx < num_bytes; idx++)
    {
        // Get byte from EEPROM'

        // Write byte
        uint8_t incoming_byte = eepromGetByte();
        *(bytes + idx) = incoming_byte;
    }

    // If no errors, return is true

    return result;
}

// Writes bytes to the EEPROM
bool eepromWriteBytes(const uint32_t addr, uint8_t *bytes, const unsigned int num_bytes)
{
    // Send SPI commands to setup EEPROM for reading data at specified address

    // Read data out and write to data buffer
    bool result = true;
    for(int idx = 0; idx < num_bytes; idx++)
    {
        // Get byte from EEPROM'

        // Write byte
        uint8_t incoming_byte = eepromGetByte();
        *(bytes + idx) = incoming_byte;
    }

    // If no errors, return is true

    return result;
}


/* *************************   Private Functions   ************************ */
