#include "sequencer.h"

void Sequencer::init(Synth* s) {
  synth = s;
  playing = false;
  currentStep = 0;
  bpm = 130;  // BPM clásico acid (Trance/House)
  
  // Calcula intervalo de step: 16 pasos por 4 beats = corcheas
  stepInterval = 60000000UL / (bpm * 4);
  lastStepTime = 0;
  
  // Carga el patrón de demostración
  loadAcidPattern();
  
  Serial.printf("[SEQUENCER] Initialized at %d BPM\n", bpm);
}

void Sequencer::loadAcidPattern() {
  // Patrón acid clásico en C (nota 36 = C2, ~65 Hz)
  // Este es el tipo de patrón que escucharías en ácid house
  uint8_t notes[] = {
    36, 36, 48, 36,  // Notas base
    39, 36, 43, 36,
    36, 48, 36, 43,
    41, 36, 43, 48
  };
  
  bool acc[] = {
    1, 0, 0, 0,  // Accent en notas seleccionadas
    1, 0, 1, 0,
    0, 1, 0, 0,
    1, 0, 0, 1
  };
  
  bool sld[] = {
    0, 1, 0, 0,  // Slide (portamento) entre notas
    0, 1, 0, 0,
    1, 0, 1, 0,
    0, 0, 1, 0
  };
  
  for (int i = 0; i < NUM_STEPS; i++) {
    setStep(i, notes[i], true, acc[i], sld[i]);
  }
  
  Serial.println("[SEQUENCER] Acid pattern loaded");
}

void Sequencer::setStep(int i, uint8_t note, bool active, bool accent, bool slide) {
  if (i < 0 || i >= NUM_STEPS) return;
  steps[i].note = note;
  steps[i].active = active;
  steps[i].accent = accent;
  steps[i].slide = slide;
  steps[i].octave = 0;  // Octava baja por defecto
}

void Sequencer::setBPM(int newBPM) {
  bpm = constrain(newBPM, 60, 240);
  stepInterval = 60000000UL / (bpm * 4);
  Serial.printf("[SEQUENCER] BPM changed to %d\n", bpm);
}

void Sequencer::update() {
  if (!playing) return;
  
  unsigned long now = micros();
  
  if (now - lastStepTime >= stepInterval) {
    lastStepTime = now;
    
    // Obtén el step actual
    Step& s = steps[currentStep];
    
    if (s.active) {
      // Trigger nota en el sintetizador
      bool nextSlide = (currentStep < NUM_STEPS - 1) ? 
                       steps[currentStep + 1].slide : 
                       steps[0].slide;
      synth->noteOn(s.note, s.accent, s.slide);
    }
    
    // Avanza al siguiente step
    currentStep = (currentStep + 1) % NUM_STEPS;
  }
}

void Sequencer::play() {
  playing = true;
  currentStep = 0;
  lastStepTime = micros();
  Serial.println("[SEQUENCER] PLAY");
}

void Sequencer::stop() {
  playing = false;
  synth->noteOff(0);
  Serial.println("[SEQUENCER] STOP");
}
