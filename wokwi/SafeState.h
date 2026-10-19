#ifndef SAFE_STATE_H
#define SAFE_STATE_H

#include <Arduino.h>
#include <Preferences.h>

// Guarda a senha e o estado (trancado/destrancado) do cofre.
// Alteracao em relacao ao original: a EEPROM do Arduino foi trocada por
// Preferences (NVS da flash do ESP32). Apos reiniciar, a senha e o estado
// sao lidos de volta da memoria.
class SafeState {
public:
  void begin();
  bool locked();
  void lock();
  bool unlock(String code);
  bool hasCode();
  void setCode(String code);

private:
  Preferences _prefs;
  String _code = "";
  bool _locked = false;
};

#endif