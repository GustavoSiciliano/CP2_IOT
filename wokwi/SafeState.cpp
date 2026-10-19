#include "SafeState.h"

// Abre o espaco "cofre" e carrega senha e estado salvos.
// Se a senha salva nao tiver 4 digitos,
// comeca sem senha e destrancado.

void SafeState::begin() {
  _prefs.begin("cofre", false);
  _code = _prefs.getString("codigo", "");
  _locked = _prefs.getBool("trancado", false);

  if (_code.length() != 4) {
    _code = "";
    _locked = false;
  }
}

bool SafeState::locked() {
  return _locked;
}

// So tranca se ja houver senha cadastrada. Salva o estado na flash.
void SafeState::lock() {
  if (!hasCode()) 
  return;

  _locked = true;
  _prefs.putBool("trancado", true);
}

// Compara a senha digitada com a salva. Se estiver certa, destranca e salva.
// Se ja estiver destrancado, apenas retorna true.
bool SafeState::unlock(String code) {
  if (!_locked) 
  return true;

  if (code == _code) {
    _locked = false;
    _prefs.putBool("trancado", false);
    return true;
  }
  return false;
}

bool SafeState::hasCode() {
  return _code.length() == 4;
}

// Salva a nova senha (4 digitos) na flash.
// Observacao: a senha fica em texto simples, o que serve para simulacao.
// Ela nunca e enviada pelo MQTT.
void SafeState::setCode(String code) {
  _code = code;
  _prefs.putString("codigo", code);
}