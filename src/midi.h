#pragma once

#include "synth.h"
#include "sequencer.h"
#include <stdint.h>

class MidiHandler {
public:
  void init(Synth* s, Sequencer* sq);
  void update();
  
private:
  Synth* synth;
  Sequencer* seq;
  uint8_t midiBuffer[3];
  int bufIdx;
  
  void handleMessage(uint8_t status, uint8_t d1, uint8_t d2);
  void handleNoteOn(uint8_t note, uint8_t velocity);
  void handleNoteOff(uint8_t note);
  void handleControlChange(uint8_t cc, uint8_t value);
};
