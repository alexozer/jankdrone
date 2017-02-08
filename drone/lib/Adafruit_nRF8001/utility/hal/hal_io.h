/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision$
 */ 

/** @file
*/

/** @defgroup hal_io hal_io
 * @{
 * @ingroup hal
 * @brief Macros input/output management
*/

#include "hal_platform.h"

#ifndef HAL_IO_H__
#define HAL_IO_H__

#define HAL_IO_P00 PIN0
#define HAL_IO_P01 PIN1
#define HAL_IO_P02 PIN2
#define HAL_IO_P03 PIN3
#define HAL_IO_P04 PIN4
#define HAL_IO_P05 PIN5
#define HAL_IO_P06 PIN6
#define HAL_IO_P07 PIN7
#define HAL_IO_P08 PIN8
#define HAL_IO_P09 PIN9

#define HAL_IO_OUTPUT OUTPUT
#define HAL_IO_INPUT INPUT

/**@brief Macro to configure an I/O.
 *  @details
 *  This macro configures a given I/O to input or output with pullup/buffer configuration.
 *  @param io_name I/O to configure.
 *  @param is_input Indicate if the I/O is to be set as an input.
 *  @param io_mode Pull resistor and buffer configuration (must be HAL_IO_OUTPUT_NORMAL_STRENGTH, HAL_IO_OUTPUT_HIGH_STRENGTH, HAL_IO_INPUT_BUF_ON_NO_PULL,
 *  HAL_IO_INPUT_BUF_ON_PULL_DOWN, HAL_IO_INPUT_BUF_ON_PULL_UP, or HAL_IO_INPUT_BUF_OFF).
*/
#define HAL_IO_CONFIG(io_name, is_input, io_mode) pinMode(io_name, is_input)

/**@brief Macro to set an output.
 *  @details
 *  This macro sets the given output to the given level (1 -> high, 0 -> low).
 *  @param io_name Output to change.
 *  @param io_state Level to set.
*/
#define HAL_IO_SET_STATE(io_name, io_state) digitalWrite(io_name, io_state) 

/**@brief Macro that reads the current state of an input.
 *  @details
 *  This macro reads the current state of a logical input.
 *  @param io_name Input to read.
 *  @return Input state level (1 if high, 0 if low).
*/
#define HAL_IO_READ(io_name) digitalRead(io_name)


#endif //HAL_IO_H__

/** @} */
