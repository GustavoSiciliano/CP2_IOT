// Icones personalizados do LCD
// Baseado em icons.h do projeto original de Uri Shaked

#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>
#include <LiquidCrystal.h>

#define ICON_LOCKED_CHAR   1
#define ICON_UNLOCKED_CHAR 2
#define ICON_RIGHT_ARROW   3

// Grava os 3 icones na memoria do LCD. Deve ser chamada depois de lcd.begin().
void init_icons(LiquidCrystal &lcd);

#endif