#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ========== CD4017 PINS ==========
const int clockPin = 21;
const int resetPin = 13;
const int enablePin = 19;

// ========== PWM FOR BRIGHTNESS ==========
const int pwmChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8;

// ========== SETTINGS ==========
int pattern = 0;
int speed = 150;        // 50-1000ms
int brightness = 200;   // 0-255
bool countdownActive = false;
int countdownValue = 10;
unsigned long countdownTimer = 0;

// ========== WIFI ==========
const char* ssid = "ESP32_Chaser";
const char* password = "12345678";

AsyncWebServer server(80);

// ========== CD4017 FUNCTIONS ==========
void clockPulse() {
  digitalWrite(clockPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(clockPin, LOW);
}

void resetCounter() {
  digitalWrite(resetPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(resetPin, LOW);
}

void jumpToLed(int pos) {
  resetCounter();
  for (int i = 0; i < pos; i++) {
    clockPulse();
    delayMicroseconds(50);
  }
}

// ========== PATTERN 1: Slow Single Chase ==========
void patternSlowChase() {
  static int pos = 0;
  static unsigned long lastMove = 0;
  
  if (millis() - lastMove >= speed) {
    lastMove = millis();
    jumpToLed(pos);
    pos = (pos + 1) % 10;
  }
}

// ========== PATTERN 2: Breathing Back-Forth ==========
void patternBreathing() {
  static int pos = 0;
  static int dir = 1;
  static unsigned long lastMove = 0;
  int adjustedSpeed = speed / 2;
  
  if (millis() - lastMove >= adjustedSpeed) {
    lastMove = millis();
    jumpToLed(pos);
    pos += dir;
    if (pos >= 9 || pos <= 0) dir *= -1;
  }
}

// ========== PATTERN 3: Gentle Wave (3 LEDs) ==========
void patternGentleWave() {
  static int pos = 0;
  static unsigned long lastMove = 0;
  int waveSpeed = speed / 3;
  
  if (millis() - lastMove >= waveSpeed) {
    lastMove = millis();
    int led = pos % 10;
    jumpToLed(led);
    pos++;
  }
}

// ========== PATTERN 4: Slow Fill & Fade ==========
void patternSlowFill() {
  static int step = 0;
  static bool filling = true;
  static unsigned long lastStep = 0;
  
  if (millis() - lastStep >= speed) {
    lastStep = millis();
    
    if (filling) {
      jumpToLed(step);
      step++;
      if (step >= 10) {
        filling = false;
        step = 9;
        delay(speed * 2);  // Pause when full
      }
    } else {
      jumpToLed(step);
      step--;
      if (step < 0) {
        filling = true;
        step = 0;
        delay(speed);
      }
    }
  }
}

// ========== PATTERN 5: Ping Pong Two Dots ==========
void patternPingPong() {
  static int pos = 0;
  static int dir = 1;
  static unsigned long lastMove = 0;
  
  if (millis() - lastMove >= speed) {
    lastMove = millis();
    jumpToLed(pos);
    delay(30);
    jumpToLed(9 - pos);
    pos += dir;
    if (pos >= 5 || pos <= 0) dir *= -1;
  }
}

// ========== PATTERN 6: Even/Odd Soft Blink ==========
void patternEvenOdd() {
  static bool evenOn = true;
  static unsigned long lastSwitch = 0;
  
  if (millis() - lastSwitch >= speed * 2) {
    lastSwitch = millis();
    if (evenOn) {
      for (int i = 0; i < 10; i += 2) {
        jumpToLed(i);
        delay(20);
      }
    } else {
      for (int i = 1; i < 10; i += 2) {
        jumpToLed(i);
        delay(20);
      }
    }
    evenOn = !evenOn;
  }
}

// ========== PATTERN 7: Random Gentle Sparkle ==========
void patternSparkle() {
  static unsigned long lastSparkle = 0;
  
  if (millis() - lastSparkle >= speed / 2) {
    lastSparkle = millis();
    int led = random(10);
    jumpToLed(led);
  }
}

// ========== PATTERN 8: Sweep Bounce ==========
void patternSweepBounce() {
  static int pos = 0;
  static int dir = 1;
  static unsigned long lastMove = 0;
  int sweepSpeed = speed / 4;
  
  if (millis() - lastMove >= sweepSpeed) {
    lastMove = millis();
    jumpToLed(pos);
    pos += dir;
    if (pos >= 9) {
      dir = -1;
      delay(speed);  // Pause at end
    } else if (pos <= 0) {
      dir = 1;
      delay(speed);  // Pause at end
    }
  }
}

// ========== COUNTDOWN TIMER ==========
void runCountdown() {
  if (!countdownActive) return;
  
  if (millis() - countdownTimer >= 1000) {
    countdownTimer = millis();
    
    if (countdownValue > 0) {
      // Light up all LEDs up to countdown value
      resetCounter();
      for (int i = 0; i < countdownValue; i++) {
        clockPulse();
        delayMicroseconds(100);
      }
      countdownValue--;
    } else {
      // Flash all when done
      resetCounter();
      delay(200);
      for (int i = 0; i < 10; i++) {
        clockPulse();
        delayMicroseconds(50);
      }
      delay(200);
      countdownActive = false;
      countdownValue = 10;
    }
  }
}

// ========== WEB PAGE ==========
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LED Chaser</title>
  <style>
    body { font-family: Arial; text-align: center; background: #1a1a2e; color: #eee; padding: 15px; }
    h1 { color: #e94560; margin-bottom: 5px; }
    .card { background: #16213e; padding: 15px; margin: 10px; border-radius: 10px; }
    button { padding: 10px 18px; margin: 4px; border: none; border-radius: 5px; font-size: 14px; cursor: pointer; transition: 0.2s; }
    .active { background: #e94560; color: white; }
    .inactive { background: #0f3460; color: white; }
    .countdown-btn { background: #f0a500; color: #1a1a2e; font-weight: bold; font-size: 16px; padding: 14px 30px; }
    input[type=range] { width: 85%; margin: 8px; }
    .value { font-size: 18px; color: #e94560; font-weight: bold; }
    .pattern-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 5px; }
  </style>
</head>
<body>
  <h1>Soft LED Chaser</h1>
  
  <div class="card">
    <h3>Patterns</h3>
    <div class="pattern-grid">
      <button id="b0" class="active" onclick="setPattern(0)">Single Chase</button>
      <button id="b1" class="inactive" onclick="setPattern(1)">Breathing</button>
      <button id="b2" class="inactive" onclick="setPattern(2)">Gentle Wave</button>
      <button id="b3" class="inactive" onclick="setPattern(3)">Fill & Fade</button>
      <button id="b4" class="inactive" onclick="setPattern(4)">Ping Pong</button>
      <button id="b5" class="inactive" onclick="setPattern(5)">Even/Odd</button>
      <button id="b6" class="inactive" onclick="setPattern(6)">Sparkle</button>
      <button id="b7" class="inactive" onclick="setPattern(7)">Sweep Bounce</button>
    </div>
  </div>
  
  <div class="card">
    <h3>Speed: <span class="value" id="speedVal">150</span>ms</h3>
    <input type="range" min="50" max="1000" value="150" oninput="setSpeed(this.value)">
  </div>
  
  <div class="card">
    <h3>Brightness: <span class="value" id="brightVal">200</span></h3>
    <input type="range" min="10" max="255" value="200" oninput="setBrightness(this.value)">
  </div>
  
  <div class="card">
    <h3>Countdown Timer</h3>
    <button class="countdown-btn" onclick="startCountdown()">Start 10→0</button>
  </div>

  <script>
    function setPattern(p) {
      fetch("/pattern?p=" + p);
      for (let i = 0; i < 8; i++) {
        document.getElementById('b' + i).className = (i == p) ? 'active' : 'inactive';
      }
    }
    function setSpeed(v) {
      fetch("/speed?s=" + v);
      document.getElementById('speedVal').innerText = v;
    }
    function setBrightness(v) {
      fetch("/brightness?b=" + v);
      document.getElementById('brightVal').innerText = v;
    }
    function startCountdown() {
      fetch("/countdown");
    }
  </script>
</body>
</html>
)rawliteral";

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);

  pinMode(clockPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  digitalWrite(resetPin, LOW);

  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(enablePin, pwmChannel);
  ledcWrite(pwmChannel, brightness);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", htmlPage);
  });

  server.on("/pattern", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("p")) {
      pattern = request->getParam("p")->value().toInt();
      countdownActive = false;  // Stop countdown when switching
      resetCounter();
    }
    request->send(200);
  });

  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("s")) {
      speed = request->getParam("s")->value().toInt();
    }
    request->send(200);
  });

  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("b")) {
      brightness = request->getParam("b")->value().toInt();
      ledcWrite(pwmChannel, brightness);
    }
    request->send(200);
  });

  server.on("/countdown", HTTP_GET, [](AsyncWebServerRequest* request) {
    countdownActive = true;
    countdownValue = 10;
    countdownTimer = millis();
    resetCounter();
    request->send(200);
  });

  server.begin();
}

// ========== LOOP ==========
void loop() {
  if (countdownActive) {
    runCountdown();
    return;
  }

  switch (pattern) {
    case 0: patternSlowChase(); break;
    case 1: patternBreathing(); break;
    case 2: patternGentleWave(); break;
    case 3: patternSlowFill(); break;
    case 4: patternPingPong(); break;
    case 5: patternEvenOdd(); break;
    case 6: patternSparkle(); break;
    case 7: patternSweepBounce(); break;
  }
}




