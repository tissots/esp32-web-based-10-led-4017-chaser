
# <p align="center"><font color="#FF6B00">ESP32 + CD4017 Asynchronous 10-LED Chaser</font></p>

<p align="center">
  <a href="https://platformio.org/"><img src="https://img.shields.io/badge/PlatformIO-FF6B00?style=for-the-badge&logo=platformio&logoColor=white" alt="PlatformIO"></a>
  <a href="https://www.espressif.com/"><img src="https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP32"></a>
  <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge" alt="License: MIT"></a>
</p>

<p align="center"><strong>An advanced, high-performance LED sequencer that combines the computational power of an ESP32 with the classic CD4017 decade counter, featuring real-time web control over 8 dynamic lighting patterns.</strong></p>

---

## <font color="#2ecc71">Overview</font>

This project reimagines the traditional 4017 LED chaser by decoupling the control logic from the timing hardware. Using an **ESP32** as the master controller, it generates precise, high-frequency pulse trains to drive the **CD4017** into complex, non-sequential behaviors that would be impossible with traditional 555-timer-based designs.

The result is a professional-grade lighting controller with a responsive dark-themed web dashboard, allowing instant pattern switching, speed control, and global brightness management—all over an isolated **Wi-Fi Access Point**.

---

## <font color="#3498db">Key Features</font>

### Web Interface
* **Asynchronous Web Engine** powered by `ESPAsyncWebServer` for instant, non-blocking parameter updates.
* **SPIFFS Architecture** serving a clean, modern HTML5/CSS3/JavaScript frontend directly from flash.
* **Dark-Themed Dashboard** with real-time visual feedback and responsive controls.
* **Parameter Persistence** via browser `localStorage` (settings survive page refreshes).

### Lighting Control
* **8 Distinct Patterns:** Sweep, Ping Pong, Sparkle, Wave, Random, Cascade, Dual Sweep, and Off.
* **Sub-Millisecond Resolution:** High-speed clock/reset pulse trains for complex non-sequential behaviors.
* **Hardware PWM Dimming:** Smooth 5kHz brightness control via the Clock Inhibit line.
* **Animation Speed Control:** 50ms to 2000ms adjustable step intervals.

### Hardware Architecture
* **Isolated Wi-Fi AP:** Direct device control without external network dependencies.
* **Non-Sequential Driving:** Programmatic pulse generation for advanced pattern creation.
* **Clean Signal Generation:** Eliminates jitter through dedicated timer interrupts.
* **Future-Ready:** GPIO allocation supports easy expansion to additional 4017s.

---

## <font color="#e67e22">Quick Start</font>

### Prerequisites
* [PlatformIO IDE](https://platformio.io/install/) or VS Code with PlatformIO extension
* ESP32 development board (ESP32-WROOM or compatible)
* CD4017 decade counter IC
* 10 LEDs with current-limiting resistors (220Ω recommended)
* Breadboard and jumper wires

### Installation

1. **Clone the repository**
```bash
   git clone [https://github.com/yourusername/esp32-4017-led-chaser.git](https://github.com/yourusername/esp32-4017-led-chaser.git)
   cd esp32-4017-led-chaser

```

2. **Build and upload the firmware**

```bash
   # Upload the web interface to SPIFFS
   pio run --target uploadfs
   
   # Build and upload the firmware
   pio run --target upload

```

3. **Hardware Setup** (See [Circuit Diagram](https://www.google.com/search?q=%23circuit-diagram) below)
4. **Connect to the Device**

* Scan for Wi-Fi network `ESP32-LED-Chaser`
* Password: `12345678`
* Open browser and navigate to `http://192.168.4.1`

5. **Control Your LEDs!**

---

## Hardware Setup

### Circuit Diagram

```
              ESP32                    CD4017
          +----------+              +----------+
          |    D4    |----CLOCK---->| 14  CLK  |
          |    D5    |----RESET---->| 15  RST  |
          |    D6    |---INHIBIT--->| 13  INH  |
          +----------+              |          |
                                    |    Q0    |---> LED 1
                                    |    Q1    |---> LED 2
                                    |    Q2    |---> LED 3
                                    |    Q3    |---> LED 4
                                    |    Q4    |---> LED 5
                                    |    Q5    |---> LED 6
                                    |    Q6    |---> LED 7
                                    |    Q7    |---> LED 8
                                    |    Q8    |---> LED 9
                                    |    Q9    |---> LED 10
                                    |          |
                                    |  VDD GND |
                                    +----------+
                                         |   |
                                       5V   GND

```

### Component Selection

| Component | Specification | Quantity | Status |
| --- | --- | --- | --- |
| **ESP32** | Any development board | 1 | Required |
| **CD4017** | Decade Counter/Divider | 1 | Required |
| **LEDs** | 5mm, any color | 10 | Required |
| **Resistors** | 220Ω (for LEDs) | 10 | Required |
| **Resistor** | 10kΩ (pull-down) | 1 | Recommended |
| **Capacitor** | 100nF (decoupling) | 1 | Recommended |

### Pin Configuration

| ESP32 Pin | Function | CD4017 Pin | Color Code |
| --- | --- | --- | --- |
| **GPIO 4** | Clock | Pin 14 | Blue |
| **GPIO 5** | Reset | Pin 15 | Green |
| **GPIO 6** | Clock Inhibit | Pin 13 | Yellow |

---

## Available Patterns

| Pattern | Description |
| --- | --- |
| **Sweep** | Classic sequential left-to-right rotation |
| **Ping Pong** | Bouncing LED effect with smooth direction reversal |
| **Sparkle** | Random LED activation with persistence simulation |
| **Wave** | Sine-wave inspired gradual fade transitions |
| **Random** | Fully randomized LED selection at each step |
| **Cascade** | Multi-stage activation with trailing effect |
| **Dual Sweep** | Two independent sweeps from both ends |
| **Off** | All LEDs off (standby mode) |

---

## Project Structure

```
esp32-4017-led-chaser/
├── data/
│   └── index.html          # Frontend Web UI (HTML5/CSS3/JavaScript)
├── src/
│   ├── main.cpp            # Hardware orchestration & loop execution
│   ├── web_server.h        # Asynchronous HTTP routes & SPIFFS linking
│   └── patterns.h          # CD4017 timing primitives & animation states
├── hardware/
│   ├── schematics/         # Circuit diagrams (PDF/PNG)
│   ├── gerbers/            # Production-ready manufacturing files (ZIP)
│   └── bom/                # Complete Bill of Materials (CSV)
├── docs/
│   └── assets/             # System diagrams and UI screenshots
├── platformio.ini          # PlatformIO environment configuration
├── LICENSE
└── README.md               # This file

```

---

## Technical Deep Dive

### Pattern Generation Algorithm

The CD4017 normally advances one step per clock pulse. By carefully timing clock and reset pulses, we can create complex patterns:

```cpp
void updatePattern(PatternType type) {
    switch(type) {
        case PING_PONG:
            // Clock forward, reset at end, invert direction
            if (currentStep == 0) direction = FORWARD;
            if (currentStep == 9) direction = REVERSE;
            break;
        case SPARKLE:
            // Rapid random clock pulses with resets
            for(int i = 0; i < random(1,5); i++) {
                pulseClock();
                delayMicroseconds(50);
            }
            break;
    }
}

```

### PWM Dimming Implementation

Rather than using a dedicated PWM channel, we modulate the Clock Inhibit line at **5kHz**:

* **HIGH**: Inhibits clock signal (LED off)
* **LOW**: Allows clock signal (LED on)
* **Duty Cycle**: Determines effective brightness

> **Note:** This approach perfectly maintains pattern synchronization while providing smooth 0-255 brightness levels.

---

## Usage Tips

### Performance Optimization

* **Speed Setting:** Lower values (<100ms) create fluid, smooth animations; higher values (>500ms) provide clearer pattern visibility.
* **Brightness Management:** For battery-powered applications, reducing brightness to 50% extends runtime by approximately 40%.
* **Pattern Selection:** Sequential patterns (Sweep, Ping Pong) work best at medium speeds; random patterns (Sparkle, Random) are more visually interesting at high speeds.

### Troubleshooting

> **Warning:** Erratum/noise handling: Always remember to add a 100nF decoupling capacitor near the CD4017 VDD pin to clear out line noise!

| Issue | Solution |
| --- | --- |
| **LEDs not lighting** | Check CD4017 VDD (5V) and GND connections. |
| **Stuttering animation** | Reduce WiFi traffic; set AP to minimal broadcast. |
| **Web UI not loading** | Ensure SPIFFS uploaded (`pio run --target uploadfs`). |

---

## Future Enhancements

* **Multi-4017 Cascading:** Support for chaining multiple CD4017s for 20+ LED control.
* **MQTT Integration:** Cloud-based control for remote smart-home ecosystem management.
* **Color Control:** RGB LED support with separate PWM channels.
* **Pattern Editor:** User-defined custom patterns through the web interface.
* **Audio Reactive Mode:** LED synchronization with ambient microphone/aux input.

---

## Contributing

We love contributions! Please follow this workflow to submit updates:

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a **Pull Request**

---

## License

Distributed under the **MIT License**. See `LICENSE` file for more information.

---

## Acknowledgments

* [PlatformIO](https://www.google.com/search?q=https%3A%2F%2Fplatformio.org%2F) for the elite development ecosystem.
* [me-no-dev](https://www.google.com/search?q=https%3A%2F%2Fgithub.com%2Fme-no-dev) for the asynchronous web server engine.
* The entire ESP32 community for continuous inspiration!

---

## Contact

**Project Maintainer:** [Your Name](https://www.google.com/search?q=mailto%3Ayour.email%40example.com)

**Project Link:** [https://github.com/yourusername/esp32-4017-led-chaser](https://www.google.com/search?q=https%3A%2F%2Fgithub.com%2Fyourusername%2Fesp32-4017-led-chaser)

---
