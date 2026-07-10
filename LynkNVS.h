#include <Preferences.h>

Preferences configPrefs;
ConfigStruct savedConfig{};

void nvsBegin() {
  if(!configPrefs.begin("box_config")) return;
  if (configPrefs.isKey("struct_version")
      && configPrefs.getUChar("struct_version") == configStructVersion
      && configPrefs.getBytesLength("config") == sizeof(config)) {
    size_t bytesCount = configPrefs.getBytes("config", &savedConfig, sizeof(savedConfig));
    if (bytesCount == sizeof(config)) config = savedConfig;
    else {
      savedConfig = config;
      configPrefs.putBytes("config", &config, sizeof(config));
    }
  } else {
    configPrefs.putUChar("struct_version", configStructVersion);
    configPrefs.putBytes("config", &config, sizeof(config));
    savedConfig = config;
  }
  configPrefs.end();
}

void saveConfigNVS() {
  if (memcmp(&config, &savedConfig, sizeof(config)) == 0) return;  // no change
  if(!configPrefs.begin("box_config")) return;
  configPrefs.putBytes("config", &config, sizeof(config));
  savedConfig = config;
  configPrefs.end();
}