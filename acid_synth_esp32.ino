/*
  ACID SYNTH ESP32 - Main Sketch
  Sintetizador estilo TB-303 con ESP32 WROOM-32U
  
  Autor: Your Name
  Fecha: 2026
  Descripción: Motor de síntesis acid con secuenciador de 16 pasos,
               filtro ladder Moog-like, y control vía potenciómetros + OLED
*/

#include "Arduino.h"
#include "driver/i2s.h"
#include "src/synth.h"
#include "src/sequencer.h"
#include "src/midi.h"
#include "src/ui.h"

// ─── Configuración I2S ───────────────────────────────────────
#define I2S_NUM         I2S_NUM_0
#define I2S_BCK_PIN     26    // Bit Clock
#define I2S_WS_PIN      25    // Word Select / LRCK
#define I2S_DATA_PIN    22    // Serial Data Out
#define SAMPLE_RATE     44100
#define BUFFER_SIZE     256   // muestras por buffer DMA

static int16_t audioBuffer[BUFFER_SIZE * 2]; // stereo L+R

// Instancias globales
Synth synth;
Sequencer seq;
MidiHandler midi;
UI ui;

// ─── Tarea de audio (Núcleo 0) ──────────────────────────────
// Esta tarea genera audio en tiempo real sin interferencias
void audioTask(void* param) {
  size_t bytesWritten;
  
  Serial.println("[AUDIO] Iniciando tarea de síntesis en Core 0");
  
  while(true) {
    // Procesa BUFFER_SIZE muestras
    synth.process(audioBuffer, BUFFER_SIZE);
    
    // Envía datos al DAC vía I2S
    i2s_write(I2S_NUM, audioBuffer, BUFFER_SIZE * 4, &bytesWritten, portMAX_DELAY);
  }
}

void setup() {
  // ─── Serial para debugging + MIDI ───
  Serial.begin(115200);  // Debugging
  delay(1000);
  
  Serial.println("\n\n╔═══════════════════════════════════════╗");
  Serial.println("║   🎛️  ACID SYNTH ESP32 - BOOT 🎛️    ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  
  // ─── Inicializa módulos ───
  Serial.println("[SETUP] Inicializando sintetizador...");
  synth.init();
  
  Serial.println("[SETUP] Inicializando secuenciador...");
  seq.init(&synth);
  
  Serial.println("[SETUP] Inicializando MIDI...");
  midi.init(&synth, &seq);
  
  Serial.println("[SETUP] Inicializando UI...");
  ui.init(&synth, &seq);
  
  // ─── Configurar I2S para DAC PCM5102A ───
  Serial.println("[SETUP] Configurando I2S...");
  
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Estéreo
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = true,  // Usar APLL para mejor precisión
    .tx_desc_auto_clear = true
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
  
  Serial.println("[I2S] Configurado correctamente");
  Serial.printf("[I2S] Sample Rate: %d Hz\n", SAMPLE_RATE);
  Serial.printf("[I2S] Buffer Size: %d muestras\n", BUFFER_SIZE);
  
  // ─── Lanzar tarea de audio en Core 0 ───
  Serial.println("[SETUP] Lanzando tarea de audio en Core 0...");
  xTaskCreatePinnedToCore(
    audioTask,        // función
    "Audio",          // nombre
    8192,             // stack size
    NULL,             // parámetro
    24,               // prioridad (máxima)
    NULL,             // task handle
    0                 // Core 0
  );
  
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║  ✅  ACID SYNTH LISTO PARA USAR  ✅  ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  Serial.println("Presiona PLAY en el panel para comenzar...\n");
}

void loop() {
  // ─── Core 1: Secuenciador + UI + MIDI ───
  // Esta tarea maneja entrada del usuario y lógica de secuenciador
  // NO genera audio para evitar conflictos
  
  seq.update();      // Actualiza step del secuenciador
  midi.update();     // Lee mensajes MIDI
  ui.update();       // Lee potenciómetros, botones, actualiza OLED
  
  vTaskDelay(1);     // Yield al OS para evitar watchdog
}

// ─── Debug: Info de memoria y núcleos ───
void printDebugInfo() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 5000) return;  // Cada 5 segundos
  lastPrint = millis();
  
  Serial.printf("[DEBUG] Free Heap: %d bytes\n", esp_get_free_heap_size());
  Serial.printf("[DEBUG] Core 0 running: %s\n", 
                pcTaskGetTaskName(NULL) ? "yes" : "no");
}
