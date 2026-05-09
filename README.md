# 🎛️ ACID SYNTH ESP32

Sintetizador estilo TB-303 con ESP32 WROOM-32U. Replicas el sonido acid legendario del Roland TB-303 en hardware de bajo costo.

## 📋 Características

- **Oscilador VCO**: sawtooth, square, triangle seleccionables
- **Filtro Ladder 24dB/oct** (Moog-like) con resonancia hasta auto-oscilación
- **Envolvente VCF + VCA** dedicada
- **Accent**: boost volumen + apertura de filtro en notas marcadas
- **Portamento/Slide**: deslizamiento suave entre notas
- **LFO**: modulación de cutoff, pitch o amplitud
- **Secuenciador de 16 pasos** ajustable
- **Control MIDI** vía USB
- **Display OLED** 128×64 I2C para visualización en tiempo real
- **8 Potenciómetros** para parámetros en tiempo real

## 🛠️ Hardware Requerido

| Componente | Modelo | Cantidad |
|-----------|--------|----------|
| ESP32 | WROOM-32U | 1 |
| DAC I2S | PCM5102A breakout | 1 |
| Display OLED | GM12864-59N (I2C) | 1 |
| Potenciómetro 10kΩ | Linear B10K | 8 |
| Botón pulsador | 12mm momentáneo | 12+ |
| LED 3mm | Varios colores | 18 |
| Conector TRS 3.5mm | Hembra PCB | 1 |
| Resistencias | Varios valores | ~30 |
| Condensadores | Varios valores | ~10 |

**Costo total estimado**: $25-45 USD

## 🔌 Esquema de Conexiones

### ESP32 Pinout

| Pin ESP32 | GPIO | Función | Conectado a |
|-----------|------|---------|------------|
| GPIO 25 | 25 | I2S LRCK | PCM5102A LCK |
| GPIO 26 | 26 | I2S BCLK | PCM5102A BCK |
| GPIO 22 | 22 | I2S DOUT | PCM5102A DIN |
| GPIO 21 | 21 | I2C SDA | OLED SDA |
| GPIO 19 | 19 | I2C SCL | OLED SCL |
| GPIO 34 | 34 | ADC Pot 1 | POT Cutoff |
| GPIO 35 | 35 | ADC Pot 2 | POT Resonance |
| GPIO 32 | 32 | ADC Pot 3 | POT Decay |
| GPIO 33 | 33 | ADC Pot 4 | POT Accent |
| GPIO 36 | 36 | ADC Pot 5 | POT Volume |
| GPIO 39 | 39 | ADC Pot 6 | POT Waveform |
| GPIO 4 | 4 | ADC Pot 7 | POT LFO Rate |
| GPIO 2 | 2 | ADC Pot 8 | POT Slide Time |
| GPIO 5 | 5 | Botón PLAY | Pulsador Play/Stop |
| GPIO 13 | 13 | LED status | LED indicador beat |

### DAC PCM5102A

```
VCC ──────────→ 3.3V ESP32
GND ──────────→ GND
LCK (LRCK) ───→ GPIO 25
BCK (BCLK) ───→ GPIO 26
DIN ──────────→ GPIO 22
SCK ──────────→ GND (modo esclavo)
FMT ──────────→ GND (I2S estándar)
XSMT ─────────→ 3.3V (deshabilitar mute)
LOUT ─────────→ Jack L (via 33Ω)
ROUT ─────────→ Jack R (via 33Ω)
```

### Display OLED GM12864-59N (I2C)

```
VCC ──────→ 3.3V
GND ──────→ GND
SDA ──────→ GPIO 21
SCL ──────→ GPIO 19
```

## 📦 Estructura del Proyecto

```
acid-synth/
├── README.md                    # Este archivo
├── acid_synth_esp32.pdf         # Documentación completa
├── acid_synth_esp32.ino         # Archivo principal
├── src/
│   ├── synth.h                  # Declaraciones del sintetizador
│   ├── synth.cpp                # Motor de síntesis (VCO + VCF + VCA)
│   ├── sequencer.h              # Declaraciones del secuenciador
│   ├── sequencer.cpp            # Lógica del secuenciador de 16 pasos
│   ├── midi.h                   # Manejo MIDI
│   ├── midi.cpp                 # Parser MIDI + handlers
│   ├── ui.h                     # Declaraciones UI
│   └── ui.cpp                   # UI y display OLED
├── hardware/
│   ├── schematic.txt            # Esquemático (descripción)
│   └── pinout.txt               # Referencia de pines
└── docs/
    ├── SETUP.md                 # Guía de instalación
    ├── TROUBLESHOOTING.md       # Solución de problemas
    └── SOUND_DESIGN.md          # Guía de sonido acid
```

## 🚀 Inicio Rápido

### 1. Instalación del Entorno

1. Instala **Arduino IDE 2.x** desde [arduino.cc](https://www.arduino.cc/)
2. Agrega ESP32 al Boards Manager:
   - File → Preferences → Additional Boards Manager URLs
   - URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Busca 'esp32' → Instala 'esp32 by Espressif Systems'
3. Instala librerías requeridas (Sketch → Include Library → Manage Libraries):
   - `Adafruit SSD1306`
   - `Adafruit GFX Library`

### 2. Configuración de Arduino IDE

- Tools → Board: `ESP32 Arduino` → `ESP32 Dev Module`
- Tools → CPU Frequency: `240 MHz`
- Tools → Flash Size: `4MB`
- Tools → Partition Scheme: `Default 4MB with spiffs`

### 3. Compilación y Carga

1. Conecta tu ESP32 por USB
2. Abre `acid_synth_esp32.ino` en Arduino IDE
3. Verifica: `Sketch → Verify` (o Ctrl+R)
4. Sube: `Sketch → Upload` (o Ctrl+U)

### 4. Prueba Inicial

Si todo va bien, deberías ver:
- ✅ El OLED mostrando "ACID SYNTH ESP32"
- ✅ Un LED parpadeando (indicador de beat)
- ✅ Los potenciómetros leyendo valores

## 🎵 Primeros Pasos con el Sonido

1. Conecta altavoces al jack de salida de audio (3.5mm)
2. Presiona el botón PLAY
3. Deberías escuchar el patrón acid demo
4. Ajusta los potenciómetros en tiempo real:
   - **Cutoff**: abre/cierra el filtro (efecto principal acid)
   - **Resonance**: intensidad del pico del filtro
   - **Decay**: qué tan rápido se cierra el filtro después de cada nota

## 🔊 El Sonido Acid

El secreto del acid está en la interacción de:

1. **Onda sawtooth**: rica en armónicos
2. **Filtro paso-bajo**: cutoff bajo + resonancia alta
3. **Envolvente VCF corta**: abre y cierra rápido
4. **Accent**: notas marcadas suenan más "abiertas"
5. **Slide**: deslizamiento suave entre notas

**Configuración clásica 303**:
- Cutoff: 25-40%
- Resonance: 65-85%
- VCF Decay: 100-250ms
- Accent: 70-90%
- Tempo: 130-145 BPM

## 📚 Documentación

- **SETUP.md**: Guía detallada de instalación y calibración
- **TROUBLESHOOTING.md**: Solución de problemas comunes
- **SOUND_DESIGN.md**: Técnicas para lograr diferentes sonidos acid
- **acid_synth_esp32.pdf**: Documentación técnica completa (teoría, código, etc.)

## 🛠️ Próximos Pasos

- [ ] Prueba I2S y DAC
- [ ] Configura OLED
- [ ] Ajusta ADCs (calibración)
- [ ] Prueba secuenciador
- [ ] Integra MIDI
- [ ] Construcción de hardware completo
- [ ] Diseño de enclosure

## 🤝 Contribuciones

Si tienes mejoras, bug fixes o sugerencias, ¡abre un issue o PR!

## 📄 Licencia

Este proyecto es de código abierto. Úsalo libremente para aprender y construir.

## 🎛️ Inspiración

Basado en la documentación técnica completa incluida (acid_synth_esp32.pdf) y el sonido legendario del Roland TB-303 (1981).

---

**¡Bienvenido al mundo del acid synth! 🎉**
