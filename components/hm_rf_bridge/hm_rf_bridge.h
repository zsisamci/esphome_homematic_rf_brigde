#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

#ifdef USE_ESP32
#include "radiomoduleconnector.h"
#include "rawuartudplistener.h"
#include "esphome/components/uart/uart_component_esp_idf.h"

namespace esphome::hm_rf_bridge {

class HmRFBridge : public PollingComponent {
 public:
  HmRFBridge(uart::IDFUARTComponent *uart, output::BinaryOutput *reset) : uart_(uart), reset_(reset) {}

  void setup() override;
  void update() override;
  void dump_config() override;

  void set_red_led(output::BinaryOutput *red) { red_ = red; }
  void set_blue_led(output::BinaryOutput *blue) { blue_ = blue; }
  void set_green_led(output::BinaryOutput *green) { green_ = green; }

  void set_radio_module_sensor(text_sensor::TextSensor *sensor) { radio_module_sensor_ = sensor; }
  void set_firmware_sensor(text_sensor::TextSensor *sensor) { firmware_sensor_ = sensor; }
  void set_serial_sensor(text_sensor::TextSensor *sensor) { serial_sensor_ = sensor; }
  void set_SGTIN_sensor(text_sensor::TextSensor *sensor) { SGTIN_sensor_ = sensor; }

  void set_connected_sensor(binary_sensor::BinarySensor *sensor) { connected_ = sensor; }

  float get_setup_priority() const override { return esphome::setup_priority::ETHERNET; }

 protected:
  uart::IDFUARTComponent *uart_;
  RadioModuleConnector *radioModuleConnector_{nullptr};
  RawUartUdpListener *rawUartUdpListener_{nullptr};
  output::BinaryOutput *reset_;
  output::BinaryOutput *red_{nullptr};
  output::BinaryOutput *green_{nullptr};
  output::BinaryOutput *blue_{nullptr};
  binary_sensor::BinarySensor *connected_{nullptr};
  text_sensor::TextSensor *radio_module_sensor_{nullptr};
  text_sensor::TextSensor *firmware_sensor_{nullptr};
  text_sensor::TextSensor *serial_sensor_{nullptr};
  text_sensor::TextSensor *SGTIN_sensor_{nullptr};
};

}  // namespace esphome::hm_rf_bridge

#endif
