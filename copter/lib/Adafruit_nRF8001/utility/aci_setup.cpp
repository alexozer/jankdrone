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
 
/**
 * My project template
 */
 
#include <avr/pgmspace.h>
#include <ble_system.h>
#include <lib_aci.h>
#include "aci_setup.h"


// aci_struct that will contain 
// total initial credits
// current credit
// current state of the aci (setup/standby/active/sleep)
// open remote pipe pending
// close remote pipe pending
// Current pipe available bitmap
// Current pipe closed bitmap
// Current connection interval, slave latency and link supervision timeout
// Current State of the the GATT client (Service Discovery status)

static hal_aci_evt_t  aci_data;
static hal_aci_data_t aci_cmd;

aci_status_code_t aci_setup(aci_state_t *aci_stat, uint8_t num_cmds, uint8_t num_cmd_offset)
{
  uint8_t i = 0;
  uint8_t evt_count = 0;
  aci_evt_t * aci_evt = NULL;
  
  while (i < num_cmds)
  {
    //Copy the setup ACI message from Flash to RAM
    //Add 2 bytes to the length byte for status byte, length for the total number of bytes
    memcpy_P(&aci_cmd, &(aci_stat->aci_setup_info.setup_msgs[num_cmd_offset+i]), 
              pgm_read_byte_near(&(aci_stat->aci_setup_info.setup_msgs[num_cmd_offset+i].buffer[0]))+2); 
    
    //Put the Setup ACI message in the command queue
    if (!hal_aci_tl_send(&aci_cmd))
    {
      Serial.println(F("Cmd Queue Full"));
      return ACI_STATUS_ERROR_INTERNAL;
    }
    else
    {
        //Debug messages:
        //Serial.print(F("Setup msg"));
        //Serial.println(i, DEC);
        #ifdef __arm__
        // This entire setup scheme may have an off-by-one error, where it tries to
        // put 8 commends into the queue which can only hold 7 due to the way the
        // head & tail indexes are managed.  On AVR, the processor is simply too
        // slow to fill the queue before at least one interrupt, but a fast ARM
        // processor can easily do so.  This delay is a workaround to avoid having
        // to restructure a lot of code...
        delayMicroseconds(10);
        #endif
    }
    
    i++;
  }
  while (1)
  {
    //We will sit here if we do not get the same number of command response evts as the commands sent to the ACI
    //
    //@check The setup wil fail in the while(1) below when the 32KHz source for the nRF8001 is in-correct in the setup generated in the nRFgo studio
    if (true == lib_aci_event_get(aci_stat, &aci_data))
    {
      aci_evt = &aci_data.evt;
      
      evt_count++;
  
      if (ACI_EVT_CMD_RSP != aci_evt->evt_opcode )
      {
        //Got something other than a command response evt -> Error
        return ACI_STATUS_ERROR_INTERNAL;
      }
      
      if (!((ACI_STATUS_TRANSACTION_CONTINUE == aci_evt->params.cmd_rsp.cmd_status) || 
           (ACI_STATUS_TRANSACTION_COMPLETE == aci_evt->params.cmd_rsp.cmd_status)))
      {
        return (aci_status_code_t )aci_evt->params.cmd_rsp.cmd_status;
      }
      else
      {
        //Serial.print(F("Cmd Response Evt "));
        //Serial.println(evt_count);
      }
      
      if (num_cmds == evt_count)
      {
        break;
      }                  
    }
  }
  
  return ((aci_status_code_t)aci_evt->params.cmd_rsp.cmd_status);              
}


aci_status_code_t do_aci_setup(aci_state_t *aci_stat)
{
  aci_status_code_t status = ACI_STATUS_ERROR_CRC_MISMATCH;
  uint8_t i=0;

  if(ACI_QUEUE_SIZE >= aci_stat->aci_setup_info.num_setup_msgs)
  {
    status = aci_setup(aci_stat, aci_stat->aci_setup_info.num_setup_msgs, 0);
  }
  else
  {
    for(i=0; i<(aci_stat->aci_setup_info.num_setup_msgs/ACI_QUEUE_SIZE); i++)
    {
      //Serial.print(ACI_QUEUE_SIZE, DEC);
      //Serial.print(F(" "));
      //Serial.println(0+(ACI_QUEUE_SIZE*i), DEC);
      status = aci_setup(aci_stat, ACI_QUEUE_SIZE, (ACI_QUEUE_SIZE*i));
    }
    if ((aci_stat->aci_setup_info.num_setup_msgs % ACI_QUEUE_SIZE) != 0)
    {
     status = aci_setup(aci_stat, aci_stat->aci_setup_info.num_setup_msgs % ACI_QUEUE_SIZE, (ACI_QUEUE_SIZE*i));               
    }
  }

  return status;
}

