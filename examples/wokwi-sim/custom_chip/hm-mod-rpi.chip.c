// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright 2023

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define CRC_SEED 0xffff // initial crc with start byte 0xfd included

#define IDX_DESTINATION 0
#define IDX_COUNTER 1
#define IDX_COMMAND 2

// command, type, data
// HMSYSTEM ACK: 0x04
static uint8_t system_identify_response[] = {0x04, 0x02, 0x43, 0x6F, 0x5F, 0x43, 0x50, 0x55, 0x5F, 0x42, 0x4C}; // "Co_CPU_BL"
static uint8_t system_start_app_response[] = {0x04, 0x01};

// COMMON ACK 0x05
static uint8_t common_identify_response[] = {0x05, 0x01, 0x44, 0x75, 0x61, 0x6C, 0x43, 0x6F, 0x50, 0x72, 0x6F, 0x5F, 0x41, 0x70, 0x70}; //"DualCoPro_App"
static uint8_t common_start_bl_response[] = {0x05, 0x01};
static uint8_t common_get_sgtin_response[] = {0x05, 0x01, 0x30, 0x14, 0xF7, 0x11, 0xA0, 0x61, 0xA7, 0xD5, 0x69, 0x9D, 0xAB, 0x52}; // SGTIN

// TRX ACK: 0x04
static uint8_t trx_get_version_response[] = {0x04, 0x01, 0x02, 0x08, 0x06, 0x01, 0x00, 0x03, 0x01, 0x14, 0x03}; // version 0x02, 0x08, 0x06, 2.8.6
static uint8_t trx_get_dutycycle_response[] = {0x04, 0x01, 0x00};
static uint8_t trx_set_dutycycle_limit_response[] = {0x04, 0x01};
static uint8_t trx_get_mcu_type_response[] = {0x04, 0x01, 0x03};

// LLMAC ACK: 0x01
static uint8_t llmac_get_default_rf_address_response[] = {0x01, 0x01, 0x4F, 0x68, 0xF1};
static uint8_t llmac_get_serial_response[] = {0x01, 0x01, 0x46, 0x4B, 0x45, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
static uint8_t llmac_get_timestamp_response[] = {0x01, 0x01, 0x2D, 0xEA};
static uint8_t llmac_rfd_init_response[] = {0x01, 0x01, 0x12, 0x34};

// HMIP ACK:  0x06
static uint8_t hmip_set_radio_address_response[] = {0x06, 0x01};
static uint8_t hmip_get_default_rf_address_response[] = {0x06, 0x01, 0x4F, 0x68, 0xF1};
static uint8_t hmip_get_security_counter_response[] = {0x06, 0x01, 0x0C, 0xFF, 0xFF, 0xFF};
static uint8_t hmip_set_security_counter_response[] = {0x06, 0x01};
static uint8_t hmip_set_max_sent_attemps_response[] = {0x06, 0x01};
static uint8_t hmip_cmd19_response[] = {0x06, 0x01, 0xE6, 0x3C, 0xD5, 0x60, 0x36, 0x4B, 0xAB, 0xEC, 0x8C, 0xC4, 0xE1, 0x2F, 0xF8, 0x19, 0x81, 0x06, 0xE5};
static uint8_t hmip_get_nwkey_response[] = {0x06, 0x01, 0xC2, 0x14, 0x22, 0xF3, 0xCA, 0x9C, 0xBD, 0xA3, 0x5F, 0x71, 0x88, 0xA4, 0x73, 0xCE, 0x6F, 0x03, 0xA7, 0xA5, 0xCA, 0x26, 0xBA, 0xE8, 0xA2, 0x2A, 0x0D, 0x3D, 0x48, 0x97, 0xB6, 0xBA, 0x47, 0xF0, 0x1D, 0xCA, 0xA7, 0x39, 0xB8, 0x4D, 0xB3, 0xFB, 0x13, 0x47, 0x02, 0x15, 0xE5, 0x37, 0x52, 0xB9, 0x39, 0xDC, 0xBA, 0x22, 0x89, 0x34, 0xCA, 0x66, 0x66, 0x5B, 0xD3, 0x6F, 0xB3, 0xD5, 0x51, 0xB6, 0x67, 0x98};
static uint8_t hmip_get_linkpartner_response[] = {0x06, 0x01};
static uint8_t hmip_set_nwkey_response[] = {0x06, 0x01};
static uint8_t hmip_add_linkpartner_response[] = {0x06, 0x01};
static uint8_t hmip_send_response[] = {0x06, 0x01};

typedef enum hm_dst
{
  HM_DST_SYSTEM = 0x00,
  HM_DST_TRX = 0x01,
  HM_DST_HMIP = 0x02,
  HM_DST_LLMAC = 0x03,
  HM_DST_COMMON = 0xfe,
} hm_dst_t;

typedef enum hm_system_cmd
{
  HM_SYSTEM_IDENTIFY = 0x00,
  HM_SYSTEM_START_APP = 0x03,
} hm_system_cmd_t;

typedef enum hm_common_cmd
{
  HM_COMMON_IDENTIFY = 0x01,
  HM_COMMON_START_BL = 0x02,
  HM_COMMON_GET_SGTIN = 0x04,
} hm_common_cmd_t;

typedef enum hm_trx_cmd
{
  HM_TRX_GET_VERSION = 0x02,
  HM_TRX_GET_DUTYCYCLE = 0x03,
  HM_TRX_SET_DCUTYCYCLE_LIMIT = 0x07,
  HM_TRX_GET_MCU_TYPE = 0x09,
} hm_trx_cmd_t;

typedef enum hm_llmac_cmd
{
  HM_LLMAC_GET_TIMESTAMP = 0x02,
  HM_LLMAC_RFD_INIT = 0x06,
  HM_LLMAC_GET_SERIAL = 0x07,
  HM_LLMAC_GET_DEFAULT_RF_ADDR = 0x08,
} hm_llmac_cmd_t;

typedef enum hm_hmip_cmd
{
  HM_HMIP_SET_RADIO_ADDR = 0x00,
  HM_HMIP_GET_DEFAULT_RF_ADDR = 0x01,
  HM_HMIP_SEND = 0x03,
  HM_HMIP_ADD_LINK_PARTNER = 0x04,
  HM_HMIP_GET_SECURITY_COUNTER = 0x0a,
  HM_HMIP_SET_SECURITY_COUNTER = 0x08,
  HM_HMIP_SET_MAX_SENT_ATTEMPS = 0x0d,
  HM_HMIP_GET_LINK_PARTNER = 0x12,
  HM_HMIP_GET_NWKEY = 0x13,
  HM_HMIP_SET_NWKEY = 0x14,
} hm_hmip_cmd_t;

typedef enum Status
{
  WAIT_FOR_START,
  RECEIVE_LENGTH_HIGH,
  RECEIVE_LENGTH_LOW,
  RECEIVE_FRAME_DATA,
  RECEIVE_CRC_HIGH,
  RECEIVE_CRC_LOW,
} uart_state_t;

typedef struct chip_state
{
  uint8_t buffer[BUFFER_SIZE];
  uart_dev_t uart0;
  pin_t reset;
  uint16_t frame_length;
  uint16_t frame_pos;
  uint16_t crc;
  uart_state_t state;
  bool escaped;
  bool reset_active;
  bool is_in_bl;
} chip_state_t;

static uint16_t calculate_crc(uint16_t crc, uint8_t byte)
{
  crc ^= byte << 8;

  for (int i = 0; i < 8; i++)
  {
    if (crc & 0x8000)
    {
      crc <<= 1;
      crc ^= 0x8005;
    }
    else
    {
      crc <<= 1;
    }
  }
  return crc;
}

static void send_response_frame(void *user_data, uint8_t response[], size_t response_len)
{
  chip_state_t *chip = (chip_state_t *)user_data;
  uint16_t response_pos = 0;
  uint8_t *response_buffer = &chip->buffer[chip->frame_pos]; // use existing buffer in chip state  for response
  uint16_t response_data_len = 1 + 1 + response_len;         // + destination  + counter + response(command + data)

  response_buffer[response_pos++] = 0xFD;                                // start byte
  response_buffer[response_pos++] = (uint8_t)(response_data_len >> 8);   // length High byte -2 save without length
  response_buffer[response_pos++] = (uint8_t)(response_data_len & 0xFF); // Low byte
  response_buffer[response_pos++] = chip->buffer[IDX_DESTINATION];       // Destination same as source frame
  response_buffer[response_pos++] = chip->buffer[IDX_COUNTER];           //  send same Counter value back
  memcpy(&response_buffer[response_pos], response, response_len);
  response_pos += response_len;

  // escape and calulate crc
  uint16_t crc = CRC_SEED;
  for (int i = 1; i < response_pos; i++)
  {
    crc = calculate_crc(crc, response_buffer[i]); // update crc

    if (response_buffer[i] == 0xFD || response_buffer[i] == 0xFC) //  escaping needed
    {
      memmove(&response_buffer[i + 1], &response_buffer[i], response_pos - i); // make space for escaping by moving array 1 forward
      response_buffer[i] = 0xFC;                                               // add escape marker
      response_buffer[i + 1] = response_buffer[i] & 0x7F;                      // escape actual byte
      response_pos++;
      i++;
    }
  }

  // add  crc with escaping
  uint8_t crc_high = (uint8_t)(crc >> 8);
  if (crc_high == 0xFD || crc_high == 0xFC){
    response_buffer[response_pos++] = 0xFC;
    response_buffer[response_pos++] = crc_high & 0x7F;
  } else{
    response_buffer[response_pos++] = crc_high;
  }

  uint8_t crc_low = (uint8_t)(crc & 0xFF);
  if (crc_low == 0xFD || crc_low == 0xFC){
    response_buffer[response_pos++] = 0xFC;
    response_buffer[response_pos++] = crc_low & 0x7F;
  }else{
    response_buffer[response_pos++] = crc_low;
  }

  uart_write(chip->uart0,response_buffer,response_pos);
}

static void process_frame(void *user_data)
{
  chip_state_t *chip = (chip_state_t *)user_data;
  hm_dst_t destination = (hm_dst_t)chip->buffer[IDX_DESTINATION];

  switch (destination)
  {

  case HM_DST_SYSTEM:
  {
    hm_system_cmd_t command = chip->buffer[IDX_COMMAND];
    switch (command)
    {
    case HM_SYSTEM_IDENTIFY:
      if (chip->is_in_bl)
        send_response_frame(chip, system_identify_response, sizeof(system_identify_response));
      break;
    case HM_SYSTEM_START_APP: // exist bootloader
      chip->is_in_bl = false;
      send_response_frame(chip,system_start_app_response,sizeof(system_start_app_response));
      break;
    }
    break;
  }
  case HM_DST_COMMON:
  {
    hm_common_cmd_t command = chip->buffer[IDX_COMMAND];
    switch (command)
    {
    case HM_COMMON_IDENTIFY:
      if (!chip->is_in_bl)
        send_response_frame(chip, common_identify_response, sizeof(common_identify_response));
      break;
    case HM_COMMON_START_BL:
      chip->is_in_bl = true;
      send_response_frame(chip, common_start_bl_response, sizeof(common_start_bl_response));
      break;
    case HM_COMMON_GET_SGTIN:
      send_response_frame(chip, common_get_sgtin_response, sizeof(common_get_sgtin_response));
      break;
    }
    break;
  }
  case HM_DST_TRX:
  {
    hm_trx_cmd_t command = chip->buffer[IDX_COMMAND];
    switch (command)
    {
    case HM_TRX_GET_VERSION:
      send_response_frame(chip, trx_get_version_response, sizeof(trx_get_version_response));
      break;
    case HM_TRX_GET_DUTYCYCLE:
      send_response_frame(chip, trx_get_dutycycle_response, sizeof(trx_get_dutycycle_response));
      break;
    case HM_TRX_SET_DCUTYCYCLE_LIMIT:
      send_response_frame(chip, trx_set_dutycycle_limit_response, sizeof(trx_set_dutycycle_limit_response));
      break;
    case HM_TRX_GET_MCU_TYPE:
      send_response_frame(chip, trx_get_mcu_type_response, sizeof(trx_get_mcu_type_response));
      break;
    }
    break;
  }
  case HM_DST_LLMAC:
  {
    hm_llmac_cmd_t command = chip->buffer[IDX_COMMAND];
    switch (command)
    {
    case HM_LLMAC_GET_TIMESTAMP:
      send_response_frame(chip, llmac_get_timestamp_response, sizeof(llmac_get_timestamp_response));
      break;
    case HM_LLMAC_RFD_INIT:
      send_response_frame(chip, llmac_rfd_init_response, sizeof(llmac_rfd_init_response));
      break;
    case HM_LLMAC_GET_SERIAL:
      send_response_frame(chip, llmac_get_serial_response, sizeof(llmac_get_serial_response));
      break;
    case HM_LLMAC_GET_DEFAULT_RF_ADDR:
      send_response_frame(chip, llmac_get_default_rf_address_response, sizeof(llmac_get_default_rf_address_response));
      break;
    }
    break;
  }
  case HM_DST_HMIP:
  {
    hm_hmip_cmd_t command = chip->buffer[IDX_COMMAND];
    switch (command)
    {
    case HM_HMIP_SET_RADIO_ADDR:
      send_response_frame(chip, hmip_set_radio_address_response, sizeof(hmip_set_radio_address_response));
      break;
    case HM_HMIP_GET_DEFAULT_RF_ADDR:
      send_response_frame(chip, hmip_get_default_rf_address_response, sizeof(hmip_get_default_rf_address_response));
      break;
    case HM_HMIP_SEND:
      send_response_frame(chip, hmip_send_response, sizeof(hmip_send_response));
      break;
    case HM_HMIP_ADD_LINK_PARTNER:
      send_response_frame(chip, hmip_add_linkpartner_response, sizeof(hmip_add_linkpartner_response));
      break;
    case HM_HMIP_GET_SECURITY_COUNTER:
      send_response_frame(chip, hmip_get_security_counter_response, sizeof(hmip_get_security_counter_response));
      break;
    case HM_HMIP_SET_SECURITY_COUNTER:
      send_response_frame(chip, hmip_set_security_counter_response, sizeof(hmip_set_security_counter_response));
      break;
    case HM_HMIP_SET_MAX_SENT_ATTEMPS:
      send_response_frame(chip, hmip_set_max_sent_attemps_response, sizeof(hmip_set_max_sent_attemps_response));
      break;
    case HM_HMIP_GET_LINK_PARTNER:
      send_response_frame(chip, hmip_get_linkpartner_response, sizeof(hmip_get_linkpartner_response));
      break;
    case HM_HMIP_GET_NWKEY:
      send_response_frame(chip, hmip_get_nwkey_response, sizeof(hmip_get_nwkey_response));
      break;
    case HM_HMIP_SET_NWKEY:
      send_response_frame(chip, hmip_set_nwkey_response, sizeof(hmip_set_nwkey_response));
      break;
    }

    break;
  }

  default:
  {
    printf("unsupported destination");
    break;
  }
  }
}

static void chip_reset_change(void *user_data, pin_t pin, uint32_t value)
{
  chip_state_t *chip = (chip_state_t *)user_data;

  if (value == LOW)
  {
    // Reset asserted
    printf("Reset active\n");
    chip->reset_active = true;
    chip->state = WAIT_FOR_START;
    chip->escaped = false;
    chip->is_in_bl = true;
  }
  else
  {
    // Reset released
    printf("Reset released\n");
    chip->reset_active = false;
  }
}

static void on_uart_rx_data(void *user_data, uint8_t byte)
{
  chip_state_t *chip = (chip_state_t *)user_data;

  if (chip->reset_active) // do nothing in reset
    return;

  printf("0x%02X", byte);

  if (byte == 0xFD)
  {
    chip->state = RECEIVE_LENGTH_HIGH;
    chip->crc = CRC_SEED;
    chip->escaped = false;
    return;
  }
  else if (byte == 0xFC)
  {
    chip->escaped = true;
    return;
  }

  if (chip->escaped)
  {
    chip->escaped = false;
    byte = byte | 0x80;
  }

  switch (chip->state)
  {

  case RECEIVE_LENGTH_HIGH:
    chip->frame_length = byte << 8;
    chip->crc = calculate_crc(chip->crc, byte);
    chip->state = RECEIVE_LENGTH_LOW;
    break;

  case RECEIVE_LENGTH_LOW:
    chip->frame_length |= byte;
    chip->crc = calculate_crc(chip->crc, byte);
    // chip->frame_length += 2; // Include CRC
    chip->frame_pos = 0;
    chip->state = RECEIVE_FRAME_DATA;
    break;

  case RECEIVE_FRAME_DATA:

    chip->buffer[chip->frame_pos++] = byte;
    chip->crc = calculate_crc(chip->crc, byte);

    if (chip->frame_pos == chip->frame_length)
    {
      chip->state = RECEIVE_CRC_HIGH;
    }
    else if (chip->frame_pos == BUFFER_SIZE - 1)
    {
      chip->state = WAIT_FOR_START;
    }
    break;

  case RECEIVE_CRC_HIGH:

    if ((chip->crc >> 8) == byte)
    { // check high byte
      chip->state = RECEIVE_CRC_LOW;
    }
    else
    {
      printf("CRC Error\n");
      chip->state = WAIT_FOR_START;
    }
    break;

  case RECEIVE_CRC_LOW:
    if ((chip->crc & 0xFF) == byte && chip->frame_length >= 3)
    { //  crc correct and has counter destination comand
      printf("        : Valid Frame Received\n");
      process_frame(chip);
    }
    else
    {
      if (chip->frame_length < 3)
        printf("Frame too short");
      else
        printf("CRC Error\n");

      chip->state = WAIT_FOR_START;
    }

    break;

  case WAIT_FOR_START:
    break;
  }
}

static void on_uart_write_done(void *user_data)
{
  chip_state_t *chip = (chip_state_t *)user_data;
  printf("UART write done\n");
}

void chip_init()
{
  chip_state_t *chip = calloc(1, sizeof(chip_state_t));
  if (!chip)
  {
    printf(" HM-MOD_RPI initialisation Failed!\n");
    return;
  }

  // uart init
  const uart_config_t uart_config = {
      .tx = pin_init("TX", INPUT_PULLUP),
      .rx = pin_init("RX", INPUT),
      .baud_rate = 115200,
      .rx_data = on_uart_rx_data,
      .write_done = on_uart_write_done,
      .user_data = chip,
  };
  chip->uart0 = uart_init(&uart_config);

  // reset init
  chip->reset = pin_init("RESET", INPUT_PULLUP);
  const pin_watch_config_t reset_watch = {
      .edge = BOTH,
      .pin_change = chip_reset_change,
      .user_data = chip,
  };
  pin_watch(chip->reset, &reset_watch);

  printf(" HM-MOD_RPI initialized!\n");
}