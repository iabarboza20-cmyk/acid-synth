#pragma once

#include "synth.h"
#include "sequencer.h"

// Declaraciones placeholder para UI
// TODO: Implementar con Adafruit SSD1306 OLED

class UI {
public:
  void init(Synth* s, Sequencer* seq);
  void update();
  
private:
  Synth* synth;
  Sequencer* sequencer;
};
