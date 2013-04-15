/*
 * keyboard_app.c
 *
 *  Created on: Mar 24, 2013
 *      Author: hathach
 */

/*
 * Software License Agreement (BSD License)
 * Copyright (c) 2012, hathach (tinyusb.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the tiny usb stack.
 */

//--------------------------------------------------------------------+
// INCLUDE
//--------------------------------------------------------------------+
#include "keyboard_app.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
#define QUEUE_KEYBOARD_REPORT_DEPTH   5

//--------------------------------------------------------------------+
// INTERNAL OBJECT & FUNCTION DECLARATION
//--------------------------------------------------------------------+
static tusb_keyboard_report_t usb_keyboard_report TUSB_CFG_ATTR_USBRAM;

OSAL_QUEUE_DEF(queue_kbd_report, QUEUE_KEYBOARD_REPORT_DEPTH, tusb_keyboard_report_t);
static osal_queue_handle_t q_kbd_report_hdl;

// only convert a-z (case insensitive) +  0-9
static inline uint8_t keycode_to_ascii(uint8_t keycode) ATTR_CONST ATTR_ALWAYS_INLINE;

//--------------------------------------------------------------------+
// tinyusb callback (ISR context)
//--------------------------------------------------------------------+
void tusbh_hid_keyboard_isr(uint8_t dev_addr, uint8_t instance_num, tusb_event_t event)
{
  switch(event)
  {
    case TUSB_EVENT_INTERFACE_OPEN: // application set-up
      osal_queue_flush(q_kbd_report_hdl);
      tusbh_hid_keyboard_get_report(dev_addr, instance_num, (uint8_t*) &usb_keyboard_report); // first report
    break;

    case TUSB_EVENT_INTERFACE_CLOSE: // application tear-down

    break;

    case TUSB_EVENT_XFER_COMPLETE:
      osal_queue_send(q_kbd_report_hdl, &usb_keyboard_report);
      tusbh_hid_keyboard_get_report(dev_addr, instance_num, (uint8_t*) &usb_keyboard_report);
    break;

    case TUSB_EVENT_XFER_ERROR:
      tusbh_hid_keyboard_get_report(dev_addr, instance_num, (uint8_t*) &usb_keyboard_report); // ignore & continue
    break;

    default :
    break;
  }
}

//--------------------------------------------------------------------+
// APPLICATION
//--------------------------------------------------------------------+
void keyboard_app_init(void)
{
  q_kbd_report_hdl = osal_queue_create(&queue_kbd_report);

  // TODO keyboard_app_task create
}

//------------- main task -------------//
OSAL_TASK_DECLARE( keyboard_app_task )
{
  tusb_error_t error;
  tusb_keyboard_report_t kbd_report;

  OSAL_TASK_LOOP_BEGIN

  osal_queue_receive(q_kbd_report_hdl, &kbd_report, OSAL_TIMEOUT_WAIT_FOREVER, &error);

  for(uint8_t i=0; i<6; i++)
  {
    if ( kbd_report.keycode[i] != 0 )
      printf("%c", keycode_to_ascii(kbd_report.keycode[i]));
  }

  OSAL_TASK_LOOP_END
}

//--------------------------------------------------------------------+
// HELPER
//--------------------------------------------------------------------+
static inline uint8_t keycode_to_ascii(uint8_t keycode)
{
  return
      ( KEYBOARD_KEYCODE_a <= keycode && keycode <= KEYBOARD_KEYCODE_z) ? ( (keycode - KEYBOARD_KEYCODE_a) + 'a' ) :
      ( KEYBOARD_KEYCODE_1 <= keycode && keycode < KEYBOARD_KEYCODE_0)  ? ( (keycode - KEYBOARD_KEYCODE_1) + '1' ) :
      ( KEYBOARD_KEYCODE_0 == keycode)                                  ? '0' : 'x';
}