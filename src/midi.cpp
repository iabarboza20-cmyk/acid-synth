#include "midi.h"

void MidiHandler::init(Synth* s, Sequencer* sq) {
  synth = s;
  seq = sq;
  bufIdx = 0;
  Serial.println("[MIDI] Initialized at 31250 baud");
}

void MidiHandler::update() {
  while (Serial.available()) {
    uint8_t byte = Serial.read();
    
    if (byte & 0x80) {
      // Status byte (comando MIDI)
      bufIdx = 0;
      midiBuffer[bufIdx++] = byte;
    } else if (bufIdx > 0 && bufIdx < 3) {
      // Data byte
      midiBuffer[bufIdx++] = byte;
      
      if (bufIdx == 3) {
        // Mensaje completo
        handleMessage(midiBuffer[0], midiBuffer[1], midiBuffer[2]);
        bufIdx = 0;
      }
    }
  }
}

void MidiHandler::handleMessage(uint8_t status, uint8_t d1, uint8_t d2) {
  uint8_t cmd = status & 0xF0;
  
  if (cmd == 0x90 && d2 > 0) {
    // Note On
    handleNoteOn(d1, d2);
  } else if (cmd == 0x80 || (cmd == 0x90 && d2 == 0)) {
    // Note Off
    handleNoteOff(d1);
  } else if (cmd == 0xB0) {
    // Control Change
    handleControlChange(d1, d2);
  }
}

void MidiHandler::handleNoteOn(uint8_t note, uint8_t velocity) {
  bool accent = velocity > 100;  // Velocity alta = accent
  synth->noteOn(note, accent, false);
  Serial.printf("[MIDI] Note On: %d, Velocity: %d\n", note, velocity);
}

void MidiHandler::handleNoteOff(uint8_t note) {
  synth->noteOff(note);
  Serial.printf("[MIDI] Note Off: %d\n", note);
}

void MidiHandler::handleControlChange(uint8_t cc, uint8_t value) {
  float normalized = value / 127.0f;
  
  switch(cc) {
    case 74:  // CC74 = Filter Cutoff (estándar)
      synth->params.cutoff = normalized;
      Serial.printf("[MIDI] CC74 Cutoff: %.2f\n", normalized);
      break;
    case 71:  // CC71 = Filter Resonance (estándar)
      synth->params.resonance = normalized;
      Serial.printf("[MIDI] CC71 Resonance: %.2f\n", normalized);
      break;
    case 75:  // CC75 = Filter Envelope Decay (estándar)
      synth->params.vcfDecay = 20.0f + normalized * 1980.0f;  // 20-2000ms
      Serial.printf("[MIDI] CC75 Decay: %.0fms\n", synth->params.vcfDecay);
      break;
    case 7:   // CC7 = Volume (estándar)
      synth->params.volume = normalized;
      Serial.printf("[MIDI] CC7 Volume: %.2f\n", normalized);
      break;
  }
}
