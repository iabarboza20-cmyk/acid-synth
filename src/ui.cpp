#include "ui.h"

void UI::init(Synth* s, Sequencer* seq) {
  synth = s;
  sequencer = seq;
  Serial.println("[UI] Initialized (OLED placeholder)");
}

void UI::update() {
  // Placeholder para UI
  // Próximo paso: Implementar con:
  // - Lectura de potenciómetros ADC
  // - Display OLED 128x64 I2C
  // - Visualización de parámetros en tiempo real
}
