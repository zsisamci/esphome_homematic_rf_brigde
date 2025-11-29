#include "hm_rf_bridge.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

#ifdef USE_ESP32
#include "radiomoduledetector.h"

static const char *const TAG = "HmRFBridge";

namespace esphome::hm_rf_bridge {

void HmRFBridge::setup() {
  ESP_LOGD(TAG, "setup started");
  this->radioModuleConnector_ = new RadioModuleConnector(this->reset_, this->uart_->get_uart_event_queue(),
                                                         static_cast<uart_port_t>(this->uart_->get_hw_serial_number()),
                                                         this->uart_->get_rx_buffer_size());

  radioModuleConnector_->addLed(this->red_, this->green_, this->blue_);

  ESP_LOGD(TAG, "RadioModuleConnector started");
  this->radioModuleConnector_->start();

  ESP_LOGD(TAG, "HM Modul detecting started");
  RadioModuleDetector radio_module_detector;
  radio_module_detector.detectRadioModule(radioModuleConnector_);

  auto radio_module_type = radio_module_detector.getRadioModuleType();

  std::string type;
  if (radio_module_type != RADIO_MODULE_NONE) {
    switch (radio_module_type) {
      case RADIO_MODULE_HM_MOD_RPI_PCB:
        ESP_LOGD(TAG, "Detected HM-MOD-RPI-PCB");
        type = "HM-MOD-RPI-PCB";
        break;
      case RADIO_MODULE_RPI_RF_MOD:
        ESP_LOGD(TAG, "Detected RPI-RF-MOD");
        type = "RPI-RF-MOD";
        break;
      default:
        ESP_LOGD(TAG, "Detected unknown radio module");
        type = "unknown radio module";
        break;
    }

    auto firmware_version = radio_module_detector.getFirmwareVersion();

    auto firmware_str = esphome::str_sprintf("%u.%u.%u", firmware_version[0], firmware_version[1], firmware_version[2]);
    ESP_LOGD(TAG, "Firmware Version: %s", firmware_str.c_str());
    if (this->firmware_sensor_) {
      this->firmware_sensor_->publish_state(firmware_str);
    }

    auto serial = radio_module_detector.getSerial();
    ESP_LOGD(TAG, "Serial %s", serial);
    if (this->serial_sensor_) {
      this->serial_sensor_->publish_state(serial);
    }

    auto sgtin = radio_module_detector.getSGTIN();
    ESP_LOGD(TAG, " SGTIN: %s", sgtin);
    if (this->SGTIN_sensor_) {
      this->SGTIN_sensor_->publish_state(sgtin);
    }

    ESP_LOGD(TAG, "Starting Raw Uart Udp Listener");
    this->rawUartUdpListener_ = new RawUartUdpListener(this->radioModuleConnector_);
    this->rawUartUdpListener_->start();

    this->disable_loop();

  } else {
    ESP_LOGE(TAG, "Radio module could not be detected.");
    type = "No Radio Module";
    this->radioModuleConnector_->stop();
    delete this->radioModuleConnector_;
    this->radioModuleConnector_ = nullptr;
    this->mark_failed();
  }

  if (this->radio_module_sensor_) {
    this->radio_module_sensor_->publish_state(type);
  }
}

void HmRFBridge::update() {
  if (this->connected_) {
    if (bool new_state = this->rawUartUdpListener_->isConnected(); new_state != this->connected_->state) {
      this->connected_->publish_state(new_state);
    }
  }
}

void HmRFBridge::dump_config() {
  ESP_LOGCONFIG(TAG, "hm_rf_brigde Component Configuration:");
  ESP_LOGCONFIG(TAG, "uart number %i", this->uart_->get_hw_serial_number());

  if (this->red_) {
    ESP_LOGCONFIG(TAG, "  Red LED: Configured");
  }
  if (this->green_) {
    ESP_LOGCONFIG(TAG, "  Green LED: Configured");
  }
  if (this->blue_) {
    ESP_LOGCONFIG(TAG, "  Blue LED: Configured");
  }
}

}  // namespace esphome::hm_rf_bridge

#endif
