#include <Preferences.h>

struct StatPersistant {
  uint32_t sPanelMeteringTotal = 0;  //Wh*10 0 - 4,294,967,295
  uint32_t gridOutputMetering = 0;   // *0.01 KWH
  uint32_t invOutputMetering = 0;    // *0.01 KWH
} storedStat;


Preferences configPrefs;
ConfigStruct savedConfig{};
Preferences statPrefs;

void _nvsBeginConfig() {
  if (!configPrefs.begin("box_config")) return;
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

void _nvsBeginStat() {
  if (!statPrefs.begin("box_stat")) return;
  if (statPrefs.isKey("stored_struct")
      && statPrefs.getBytesLength("stored_struct") == sizeof(storedStat)) {
    size_t bytesCount = statPrefs.getBytes("stored_struct", &storedStat, sizeof(storedStat));
    if (bytesCount == sizeof(storedStat)) {
      statInfo.sPanelMeteringTotal = storedStat.sPanelMeteringTotal;
      statInfo.gridOutputMetering = storedStat.gridOutputMetering;
      statInfo.invOutputMetering = storedStat.invOutputMetering;
    } else {
      storedStat.sPanelMeteringTotal = statInfo.sPanelMeteringTotal;
      storedStat.gridOutputMetering = statInfo.gridOutputMetering;
      storedStat.invOutputMetering = statInfo.invOutputMetering;
      statPrefs.putBytes("stored_struct", &storedStat, sizeof(storedStat));
    }
  } else {
    storedStat.sPanelMeteringTotal = statInfo.sPanelMeteringTotal;
    storedStat.gridOutputMetering = statInfo.gridOutputMetering;
    storedStat.invOutputMetering = statInfo.invOutputMetering;
    statPrefs.putBytes("stored_struct", &storedStat, sizeof(storedStat));
  }
  statPrefs.end();
}


void nvsBegin() {
  _nvsBeginConfig();
  _nvsBeginStat();
}

void saveConfigNVS() {
  if (memcmp(&config, &savedConfig, sizeof(config)) == 0) return;  // no change
  if (!configPrefs.begin("box_config")) return;
  configPrefs.putBytes("config", &config, sizeof(config));
  savedConfig = config;
  configPrefs.end();
}

void saveStatNVS() {
  StatPersistant temp;
  temp.sPanelMeteringTotal = statInfo.sPanelMeteringTotal;
  temp.gridOutputMetering = statInfo.gridOutputMetering;
  temp.invOutputMetering = statInfo.invOutputMetering;
  if (memcmp(&temp, &storedStat, sizeof(temp)) == 0) return;  // no change
  if (!statPrefs.begin("box_stat")) return;
  statPrefs.putBytes("stored_struct", &temp, sizeof(temp));
  storedStat = temp;
  statPrefs.end();
}