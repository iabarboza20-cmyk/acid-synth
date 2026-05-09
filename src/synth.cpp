#include "synth.h"
#include <string.h>
#include <math.h>

void Synth::init() {
  memset(stage, 0, sizeof(stage));
  memset(delay, 0, sizeof(delay));
  phase = subPhase = lfoPhase = 0.0f;
  vcfEnv = vcaEnv = accentEnv = 0.0f;
  currentFreq = targetFreq = 440.0f;
  cutoffSmooth = 0.3f;
  
  // Parámetros por defecto - sonido acid clásico TB-303
  params.cutoff    = 0.25f;   // Filtro cerrado inicialmente
  params.resonance = 0.75f;   // Resonancia alta
  params.vcfDecay  = 200.0f;  // Decay rápido
  params.vcfSustain= 0.05f;   // Sustain bajo
  params.vcaDecay  = 500.0f;  // Decay de amplitud
  params.accentAmt = 0.8f;    // Accent fuerte
  params.slideTime = 60.0f;   // Portamento suave
  params.volume    = 0.7f;    // Volumen moderado
  params.lfoRate   = 0.5f;    // LFO lento
  params.lfoDepth  = 0.0f;    // LFO desactivado por defecto
  params.wave      = WAVE_SAW; // Sawtooth - el sonido acid
  
  buildWavetables();
  Serial.println("[SYNTH] Initialized - wavetables generated");
}

void Synth::buildWavetables() {
  // Construir tablas de ondas band-limited (anti-aliasing)
  for (int i = 0; i < WAVETABLE_SIZE; i++) {
    float t = (float)i / WAVETABLE_SIZE;  // 0.0 - 1.0
    
    // ─── SAWTOOTH: suma de armónicos (8 parciales para evitar aliasing) ───
    sawTable[i] = 0.0f;
    for (int h = 1; h <= 8; h++) {
      sawTable[i] += sinf(TWO_PI * h * t) / h;
    }
    sawTable[i] *= 0.4f;  // Normalizar amplitud
    
    // ─── SQUARE: solo armónicos impares ───
    sqrTable[i] = 0.0f;
    for (int h = 1; h <= 8; h += 2) {
      sqrTable[i] += sinf(TWO_PI * h * t) / h;
    }
    sqrTable[i] *= 0.5f;
    
    // ─── TRIANGLE: onda triangular ───
    triTable[i] = (t < 0.5f) ? (4.0f * t - 1.0f) : (3.0f - 4.0f * t);
  }
}

float Synth::noteToFreq(uint8_t note) {
  // Convierte nota MIDI (0-127) a frecuencia en Hz
  // A4 (nota 69) = 440 Hz
  return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

void Synth::noteOn(uint8_t note, bool accent, bool slide) {
  targetFreq = noteToFreq(note);
  
  // Si no hay slide, salta directamente a la frecuencia
  if (!slide) {
    currentFreq = targetFreq;
  }
  
  // Calcula la tasa de deslizamiento (portamento)
  if (params.slideTime < 1.0f) {
    slideRate = 1.0f;  // Sin suavidad si slideTime es 0
  } else {
    slideRate = expf(-1.0f / (params.slideTime * 0.001f * SAMPLE_RATE));
  }
  
  // Trigger envelopes
  vcfEnv = 1.0f;
  vcaEnv = 1.0f;
  accentEnv = accent ? 1.0f : 0.0f;
  
  // Precalcula factores de decay
  vcfEnvDecay = expf(-1.0f / (params.vcfDecay * 0.001f * SAMPLE_RATE));
  vcaEnvDecay = expf(-1.0f / (params.vcaDecay * 0.001f * SAMPLE_RATE));
}

void Synth::noteOff(uint8_t note) {
  // Release rápido (50ms)
  vcaEnvDecay = expf(-1.0f / (50.0f * 0.001f * SAMPLE_RATE));
}

inline float Synth::tanh_fast(float x) {
  // Aproximación rápida de tanh() para simulación de saturación
  // Evita usar la función tanh() costosa
  return x / (1.0f + fabsf(x));
}

float Synth::processFilter(float input) {
  // Filtro Ladder Moog (Huovilainen 0-delay feedback)
  // Este es el corazón del sonido acid del TB-303
  
  float fc = cutoffSmooth * 0.5f;  // Nyquist normalizado
  float f  = fc * 1.16f;
  float k  = params.resonance * 3.98f;  // Gain de retroalimentación
  float p  = f * (1.8f - 0.8f * f);
  float q  = k * (1.0f + 0.5f * p * (1.0f - p));
  
  // Realimentación
  input -= q * stage[3];
  
  // Saturación suave (tanh) - simula la no-linealidad analógica
  input = tanh_fast(input);
  
  // Cascada de 4 polos
  stage[0] = input * p + delay[0] * (1.0f - p);
  stage[1] = stage[0] * p + delay[1] * (1.0f - p);
  stage[2] = stage[1] * p + delay[2] * (1.0f - p);
  stage[3] = stage[2] * p + delay[3] * (1.0f - p);
  
  // Guarda estados previos
  delay[0] = input;
  delay[1] = stage[0];
  delay[2] = stage[1];
  delay[3] = stage[2];
  
  return stage[3];
}

void Synth::process(int16_t* buffer, int numSamples) {
  float invSR = 1.0f / SAMPLE_RATE;
  
  for (int i = 0; i < numSamples; i++) {
    // ─── PORTAMENTO (deslizamiento suave de pitch) ───
    currentFreq = targetFreq + (currentFreq - targetFreq) * slideRate;
    phaseInc = currentFreq * WAVETABLE_SIZE * invSR;
    
    // ─── OSCILADOR PRINCIPAL ───
    int idx = (int)phase & (WAVETABLE_SIZE - 1);
    float osc = (params.wave == WAVE_SAW) ? sawTable[idx] :
                (params.wave == WAVE_SQUARE) ? sqrTable[idx] : triTable[idx];
    phase += phaseInc;
    if (phase >= WAVETABLE_SIZE) phase -= WAVETABLE_SIZE;
    
    // ─── SUB-OSCILADOR (octava abajo, square) ───
    int subIdx = (int)subPhase & (WAVETABLE_SIZE - 1);
    float sub = sqrTable[subIdx] * 0.3f;  // 30% de amplitud
    subPhase += phaseInc * 0.5f;          // Octava abajo = mitad de frecuencia
    if (subPhase >= WAVETABLE_SIZE) subPhase -= WAVETABLE_SIZE;
    
    // ─── LFO (Low Frequency Oscillator) ───
    float lfo = sinf(lfoPhase) * params.lfoDepth;
    lfoPhase += TWO_PI * params.lfoRate * invSR;
    if (lfoPhase > TWO_PI) lfoPhase -= TWO_PI;
    
    // ─── CUTOFF DINÁMICO ───
    // El filtro se abre por:
    // 1. Envolvente VCF (apertura rápida al pulsar nota)
    // 2. Accent (boost adicional en notas marcadas)
    // 3. LFO (modulación opcional)
    float envCutoff = params.cutoff + vcfEnv * 0.6f + accentEnv * params.accentAmt * 0.4f + lfo * 0.2f;
    envCutoff = fmaxf(0.02f, fminf(0.98f, envCutoff));  // Clamp entre 0.02 y 0.98
    
    // Suavizado exponencial (evita zipper noise)
    cutoffSmooth += (envCutoff - cutoffSmooth) * 0.01f;
    
    // ─── PROCESAR FILTRO ───
    float filtered = processFilter(osc + sub);
    
    // ─── VCA (Voltage Controlled Amplifier) ───
    // El accent también boostea la amplitud
    float amp = vcaEnv * params.volume * (1.0f + accentEnv * 0.5f);
    float out = filtered * amp;
    
    // ─── ACTUALIZAR ENVELOPES (exponential decay) ───
    vcfEnv = params.vcfSustain + (vcfEnv - params.vcfSustain) * vcfEnvDecay;
    vcaEnv *= vcaEnvDecay;
    accentEnv *= 0.9995f;  // Decay rápido del accent
    
    // ─── CLAMP y CONVERTIR A 16-BIT ───
    out = fmaxf(-1.0f, fminf(1.0f, out));
    int16_t sample = (int16_t)(out * 32767.0f);
    
    // Estéreo (mismo sample en L y R)
    buffer[i*2]     = sample;  // Left
    buffer[i*2 + 1] = sample;  // Right
  }
}
