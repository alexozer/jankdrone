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
 * @brief Interface for hal_aci_tl.
 */
 
/** @defgroup hal_aci_tl hal_aci_tl
@{
@ingroup hal
 
@brief Module for the ACI Transport Layer interface
@details This module is responsible for sending and receiving messages over the ACI interface of the nRF8001 chip.
 The hal_aci_tl_send_cmd() can be called directly to send ACI commands.


The RDYN line is hooked to an interrupt on the MCU when the level is low.
The SPI master clocks in the interrupt context.
The ACI Command is taken from the head of the command queue is sent over the SPI
and the received ACI event is placed in the tail of the event queue.

*/
 
#ifndef HAL_ACI_TL_H__
#define HAL_ACI_TL_H__

#include "hal_platform.h"

#ifndef HAL_ACI_MAX_LENGTH
#define HAL_ACI_MAX_LENGTH 31
#endif //HAL_ACI_MAX_LENGTH

#define ACI_QUEUE_SIZE  8

/** Data type for ACI commands and events */
typedef struct hal_aci_data_t{
  uint8_t status_byte;
  uint8_t buffer[HAL_ACI_MAX_LENGTH+1];
} hal_aci_data_t;


/** @brief Message received hook function.
 *  @details A hook function that must be implemented by the client of this module. 
 * The function will be called by this module when a new message has been received from the nRF8001.
 *  @param received_msg Pointer to a structure containing a pointer to the received data.
 */
extern void hal_aci_tl_msg_rcv_hook(hal_aci_data_t *received_msg);

/** ACI Transport Layer configures inputs/outputs.
 */
void hal_aci_tl_io_config(void);


/** ACI Transport Layer initialization.
 */
void hal_aci_tl_init(void);

/**@brief Sends an ACI command to the radio.
 *  @details
 *  This function sends an ACI command to the radio. This will memorize the pointer of the message to send and 
 *  lower the request line. When the device lowers the ready line, @ref hal_aci_tl_poll_rdy_line() will send the data.
 *  @param aci_buffer Pointer to the message to send.
 *  @return True if the send is started successfully, false if a transaction is already running.
 */
bool hal_aci_tl_send(hal_aci_data_t *aci_buffer);


/** @brief Check for pending transaction.
 *  @details 
 *  Call this function from the main context at regular intervals to check if the nRF8001 RDYN line indicates a pending transaction.
 *  If a transaction is pending, this function will treat it and call the receive hook.
 */
void hal_aci_tl_poll_rdy_line(void);

hal_aci_data_t * hal_aci_tl_poll_get(void);

bool hal_aci_tl_event_get(hal_aci_data_t *p_aci_data);

/** @brief Flush the ACI command Queue and the ACI Event Queue
 *  @details
 *  Call this function in the main thread
*/
void m_aci_q_flush(void);

/** @brief Enable debug printing of all ACI commands sent and ACI events received
 *  @details
 *  when the enable parameter is true. The debug printing is enabled on the Serial.
 *  When the enable parameter is false. The debug printing is disabled on the Serial.
 *  By default the debug printing is disabled.
*/
void hal_aci_debug_print(bool enable);

#endif // HAL_ACI_TL_H__
/** @} */




