#include "Icons.h"

// O HD44780 aceita ate 8 caracteres personalizados
// Aqui usamos os indices 1, 2 e 3, definidos em Icons.h.
// Cada icone tem 8 linhas de 5 pontos
static uint8_t iconeFechado[8] = {
  0b01110, 0b10001, 0b10001, 0b11111, 0b11011, 0b11011, 0b11111, 0b00000
};
static uint8_t iconeAberto[8] = {
  0b01110, 0b10000, 0b10000, 0b11111, 0b11011, 0b11011, 0b11111, 0b00000
};
static uint8_t iconeSeta[8] = {
  0b00000, 0b00100, 0b00010, 0b11111, 0b00010, 0b00100, 0b00000, 0b00000
};

void init_icons(LiquidCrystal &lcd) {
  lcd.createChar(ICON_LOCKED_CHAR, iconeFechado);
  lcd.createChar(ICON_UNLOCKED_CHAR, iconeAberto);
  lcd.createChar(ICON_RIGHT_ARROW, iconeSeta);
}