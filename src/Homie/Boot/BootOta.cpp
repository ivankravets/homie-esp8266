#include "BootOta.hpp"

using namespace HomieInternals;

BootOta::BootOta(SharedInterface* shared_interface)
: Boot("OTA")
, _shared_interface(shared_interface)
{
}

BootOta::~BootOta() {
}

void BootOta::setup() {
  Boot::setup();

  WiFi.mode(WIFI_STA);

  WiFi.begin(Config.wifi_ssid, Config.wifi_password);

  int wifi_attempts = 1;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    if (wifi_attempts <= 3) {
      WiFi.begin(Config.wifi_ssid, Config.wifi_password);
      Logger.log("Retrying Wi-Fi connection");
      Logger.log(String(wifi_attempts));
      Logger.logln("/3");
      wifi_attempts++;
    } else {
      Logger.logln("✖ Connection failed, rebooting in normal mode...");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
    }
  }
  Logger.logln("✔ Connected to Wi-Fi");
  Logger.logln("Starting OTA...");

  String dataToPass = Helpers::getDeviceId();
  dataToPass += '=';
  dataToPass += this->_shared_interface->fwname;
  dataToPass += '@';
  dataToPass += this->_shared_interface->fwversion;
  t_httpUpdate_return ret = ESPhttpUpdate.update(Config.homie_host, Config.homie_ota_port, Config.homie_ota_path, dataToPass, false, "", false);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      Logger.logln("✖ Update failed");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Logger.logln("✖ No updates");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
      break;
    case HTTP_UPDATE_OK:
      Logger.logln("✔ Success, rebooting");
      Config.boot_mode = BOOT_NORMAL;
      Config.save();

      ESP.restart();
      break;
  }
}

void BootOta::loop() {
  Boot::loop();
}
