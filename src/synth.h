#pragma once

#include <stdint.h>
#include <math.h>

// ─── Constantes ───
#define WAVETABLE_SIZE  2048
#define SAMPLE_RATE     44100
#define TWO_PI          6.28318530f

// ─── Tipos de onda ───
enum WaveShape { 
  WAVE_SAW = 0,     // Sawtooth - la onda acid clásica
  WAVE_SQUARE = 1,  // Square - más suave
  WAVE_TRI = 2      // Triangle - más redondeada
};

// ─── Parámetros del sintetizador ───
struct SynthParams {
  float cutoff;      // 0.0 – 1.0  (frecuencia de corte normalizada)
  float resonance;   // 0.0 – 1.0  (Q del filtro, intensidad del pico)
  float vcfDecay;    // ms, 20–2000 (qué tan rápido se cierra el filtro)
  float vcfSustain;  // 0.0 – 1.0  (nivel de sustain del filtro)
  float vcaDecay;    // ms, 50–500 (envolvente de amplitud)
  float accentAmt;   // 0.0 – 1.0  (intensidad del accent)
  float slideTime;   // ms, 0–500  (velocidad del portamento)
  float volume;      // 0.0 – 1.0  (volumen general)
  float lfoRate;     // Hz, 0.01–20 (velocidad del LFO)
  float lfoDepth;    // 0.0 – 1.0  (amplitud del LFO)
  WaveShape wave;    // Forma de onda seleccionada
};

// ─── Clase principal del sintetizador ───
class Synth {
public:
  SynthParams params;
  
  // Métodos públicos
  void init();                                    // Inicializa el sintetizador
  void noteOn(uint8_t note, bool accent, bool slide);  // Nota ON (MIDI)
  void noteOff(uint8_t note);                    // Nota OFF (MIDI)
  void process(int16_t* buffer, int numSamples); // Genera audio (llamado por audioTask)
  
private:
  // ─── Tablas de ondas pre-calculadas ───
  float sawTable[WAVETABLE_SIZE];   // Sawtooth band-limited
  float sqrTable[WAVETABLE_SIZE];   // Square band-limited
  float triTable[WAVETABLE_SIZE];   // Triangle band-limited
  
  // ─── Estado del oscilador ───
  float phase;           // Acumulador de fase del VCO (0.0 - WAVETABLE_SIZE)
  float phaseInc;        // Incremento de fase por muestra
  float currentFreq;     // Frecuencia actual (portamento)
  float targetFreq;      // Frecuencia objetivo (siguiente nota)
  float slideRate;       // Tasa de interpolación para portamento
  
  // ─── Sub-oscilador (octava abajo, square) ───
  float subPhase;        // Fase del sub-oscilador
  
  // ─── Estado del filtro Ladder (Huovilainen) ───
  float stage[4];        // Los 4 polos del filtro
  float delay[4];        // Estados de delay para retroalimentación
  float cutoffSmooth;    // Cutoff suavizado (evita zipper noise)
  
  // ─── Envelopes ───
  float vcfEnv;          // Envolvente del filtro (0.0 - 1.0)
  float vcaEnv;          // Envolvente de amplitud (0.0 - 1.0)
  float accentEnv;       // Envolvente del accent (0.0 - 1.0)
  float vcfEnvDecay;     // Factor de decay exponencial del VCF
  float vcaEnvDecay;     // Factor de decay exponencial del VCA
  
  // ─── LFO (Low Frequency Oscillator) ───
  float lfoPhase;        // Fase del LFO
  
  // ─── Métodos privados ───
  void buildWavetables();              // Construye las tablas de ondas
  float noteToFreq(uint8_t note);      // Convierte nota MIDI a frecuencia
  float processFilter(float input);    // Procesa el filtro ladder
};
