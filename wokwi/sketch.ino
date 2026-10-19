/**
  Cofre Eletronico com ESP32 + MQTT (HiveMQ)

  Baseado em "Arduino Electronic Safe"
  Copyright (C) 2020, Uri Shaked.
  Released under the MIT License.

  Adaptacoes feitas pelo grupo:
  - Migracao de Arduino UNO para ESP32 (novos GPIOs, ESP32Servo).
  - Senha e estado salvos na flash com Preferences (antes: EEPROM).
  - Conexao Wi-Fi e MQTT (PubSubClient), com publicacao retida do estado.
  - Todos os delay() e laços de espera trocados por funcoes que
    continuam atendendo a rede (esperar() e lerTecla()).
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include "SafeState.h"
#include "Icons.h"

// Wi-Fi e MQTT 
const char* WIFI_SSID  = "Wokwi-GUEST";
const char* WIFI_SENHA = "";
const int   WIFI_CANAL = 6;

const char* MQTT_HOST   = "broker.hivemq.com";
const uint16_t MQTT_PORTA  = 1883;
const char* MQTT_TOPICO = "fiap/DA/CP2/1tdsps/grupo-k7q2/cofre/estado";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
String clientId;

bool estadoPendente = true;
unsigned long ultimaTentativaWifi = 0;
unsigned long ultimaTentativaMqtt = 0;
unsigned long ultimoEnvio = 0;
unsigned long semRedeAte = 0;


// Pinos do ESP32
#define PIN_SERVO 4

#define LCD_RS 23
#define LCD_E 22
#define LCD_D4 21
#define LCD_D5 19
#define LCD_D6 18
#define LCD_D7 5

#define SERVO_LOCK_POS 20
#define SERVO_UNLOCK_POS 90

Servo lockServo;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Teclado
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {13, 14, 27, 26};
byte colPins[KEYPAD_COLS] = {25, 33, 32, 16};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

SafeState safeState;

// Rede

// Envia o estado atual para o HiveMQ
void publicarEstado() {
  if (millis() - ultimoEnvio < 1000) return;
  ultimoEnvio = millis();

  const char* texto = safeState.locked() ? "Locked" : "Unlocked";
  if (mqtt.publish(MQTT_TOPICO, texto, true)) {
    estadoPendente = false;
    Serial.print("Enviado: ");
    Serial.println(texto);
  }
}

// Teste: digitar 'd' no Serial Monitor derruba o MQTT por 20 segundos
void verificarSerial() {
  while (Serial.available()) {
    if (Serial.read() == 'd') {
      mqtt.disconnect();
      semRedeAte = millis() + 20000;
      Serial.println("Teste: MQTT desligado por 20 segundos");
    }
  }
}

// Cuida do Wi-Fi e do MQTT. E chamada o tempo todo, inclusive nas esperas.
void servicoRede() {
  verificarSerial();
  unsigned long agora = millis();

  if (agora < semRedeAte) return;

  // Wi-Fi caiu: tenta de novo a cada 10 segundos
  if (WiFi.status() != WL_CONNECTED) {
    if (agora - ultimaTentativaWifi > 10000) {
      ultimaTentativaWifi = agora;
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_SENHA, WIFI_CANAL);
    }
    return;
  }

  // MQTT desconectado: tenta de novo a cada 5 segundos
  if (!mqtt.connected()) {
    if (agora - ultimaTentativaMqtt > 5000) {
      ultimaTentativaMqtt = agora;
      if (mqtt.connect(clientId.c_str())) {
        Serial.println("MQTT conectado");
        estadoPendente = true;
      }
    }
    return;
  }

  mqtt.loop();
  if (estadoPendente) publicarEstado();
}

// Faz o papel do delay(), mas continua atendendo a rede
void esperar(unsigned long ms) {
  unsigned long inicio = millis();
  while (millis() - inicio < ms) {
    servicoRede();
    delay(1);
  }
}

// Le uma tecla e atende a rede ao mesmo tempo
char lerTecla() {
  servicoRede();
  return keypad.getKey();
}

// Trava

void lock() {
  safeState.lock();
  lockServo.write(SERVO_LOCK_POS);
  estadoPendente = true;
}

void unlock() {
  lockServo.write(SERVO_UNLOCK_POS);
  estadoPendente = true;
}

// LCD e teclado

void showStartupMessage() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  esperar(1000);

  lcd.setCursor(1, 1);
  String message = "ESP32Safe v1.0";
  for (byte i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    esperar(100);
  }
  esperar(500);
}

// Le 4 digitos e mostra * no LCD
String inputSecretCode() {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);
  String result = "";
  while (result.length() < 4) {
    char key = lerTecla();
    if (key >= '0' && key <= '9') {
      lcd.print('*');
      result += key;
    }
  }
  return result;
}

void showWaitScreen(int delayMillis) {
  lcd.setCursor(2, 1);
  lcd.print("[..........]");
  lcd.setCursor(3, 1);
  for (byte i = 0; i < 10; i++) {
    esperar(delayMillis);
    lcd.print("=");
  }
}

// Cadastra a senha. Se as duas digitadas forem diferentes, nao tranca.
bool setNewCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new code:");
  String newCode = inputSecretCode();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm new code");
  String confirmCode = inputSecretCode();

  if (newCode.equals(confirmCode)) {
    safeState.setCode(newCode);
    return true;
  } else {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Code mismatch");
    lcd.setCursor(0, 1);
    lcd.print("Safe not locked!");
    esperar(2000);
    return false;
  }
}

void showUnlockMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.setCursor(4, 0);
  lcd.print("Unlocked!");
  lcd.setCursor(15, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  esperar(1000);
}

// Logica do cofre

// Cofre destrancado: # tranca, A muda a senha
void safeUnlockedLogic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.setCursor(2, 0);
  lcd.print(" # to lock");
  lcd.setCursor(15, 0);
  lcd.write(ICON_UNLOCKED_CHAR);

  bool newCodeNeeded = true;
  if (safeState.hasCode()) {
    lcd.setCursor(0, 1);
    lcd.print("  A = new code");
    newCodeNeeded = false;
  }

  char key = lerTecla();
  while (key != 'A' && key != '#') {
    key = lerTecla();
  }

  bool readyToLock = true;
  if (key == 'A' || newCodeNeeded) {
    readyToLock = setNewCode();
  }

  if (readyToLock) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.write(ICON_UNLOCKED_CHAR);
    lcd.print(" ");
    lcd.write(ICON_RIGHT_ARROW);
    lcd.print(" ");
    lcd.write(ICON_LOCKED_CHAR);

    lock();
    showWaitScreen(100);
  }
}

// Cofre trancado: pede a senha
void safeLockedLogic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_LOCKED_CHAR);
  lcd.print(" Safe Locked! ");
  lcd.write(ICON_LOCKED_CHAR);

  String userCode = inputSecretCode();
  bool unlockedSuccessfully = safeState.unlock(userCode);
  showWaitScreen(200);

  if (unlockedSuccessfully) {
    showUnlockMessage();
    unlock();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied!");
    showWaitScreen(1000);
  }
}

void setup() {
  Serial.begin(115200);

  lcd.begin(16, 2);
  init_icons(lcd);

  lockServo.setPeriodHertz(50);
  lockServo.attach(PIN_SERVO, 500, 2400);

  safeState.begin();

  lockServo.write(safeState.locked() ? SERVO_LOCK_POS : SERVO_UNLOCK_POS);
 
  clientId = "cofre-esp32-" + String((uint32_t)esp_random(), HEX);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA, WIFI_CANAL);
  mqtt.setServer(MQTT_HOST, MQTT_PORTA);
  mqtt.setSocketTimeout(2);

  showStartupMessage();
}

void loop() {
  servicoRede();
  if (safeState.locked()) {
    safeLockedLogic();
  } else {
    safeUnlockedLogic();
  }
}
