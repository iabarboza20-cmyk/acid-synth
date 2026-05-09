#pragma once

#include "synth.h"

#define NUM_STEPS 16

// ─── Estructura de un Step del secuenciador ───
struct Step {
  uint8_t  note;       // 0–127 (nota MIDI)
  bool     active;     // ¿suena en este step?
  bool     accent;     // ¿tiene accent activado?
  bool     slide;      // ¿slide al siguiente step?
  uint8_t  octave;     // 0=octava baja, 1=media, 2=alta (opcional)
};

// ─── Clase del secuenciador de 16 pasos ───
class Sequencer {
public:
  Step steps[NUM_STEPS];
  bool playing;
  int  bpm;              // Tempo en beats por minuto (60-240)
  int  currentStep;      // Step actual (0-15)
  
  void init(Synth* s);
  void update();         // Actualiza el secuenciador (llamar cada loop)
  void play();           // Inicia playback
  void stop();           // Detiene playback
  void setStep(int i, uint8_t note, bool active, bool accent, bool slide);
  void setBPM(int newBPM);
  void loadAcidPattern(); // Carga un patrón acid clásico demo
  
  Synth* synth;
  
private:
  unsigned long lastStepTime;
  unsigned long stepInterval;  // Microsegundos entre steps
};
