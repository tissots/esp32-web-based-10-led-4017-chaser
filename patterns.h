#ifndef PATTERNS_H
#define PATTERNS_H

#include <Arduino.h>

// Pins and system configurations
extern const int clockPin;
extern const int resetPin;
extern const int enablePin;
extern const int pwmChannel;

extern int pattern;
extern int speed;
extern int brightness;
extern bool countdownActive;
extern int countdownValue;
extern unsigned long countdownTimer;

// Core manipulation primitives
inline void clockPulse() {
  digitalWrite(clockPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(clockPin, LOW);
}

inline void resetCounter() {
  digitalWrite(resetPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(resetPin, LOW);
}

inline void jumpToLed(int pos) {
  resetCounter();
  for (int i = 0; i < pos; i++) {
    clockPulse();
    delayMicroseconds(50);
  }
}

// Timing-safe pattern wrappers
inline void patternSlowChase() {
  static int pos = 0;
  static unsigned long lastMove = 0;
  if (millis() - lastMove >= (unsigned long)speed) {
    lastMove = millis();
    jumpToLed(pos);
    pos = (pos + 1) % 10;
  }
}

inline void patternBreathing() {
  static int pos = 0;
  static int dir = 1;
  static unsigned long lastMove = 0;
  int adjustedSpeed = speed / 2;
  if (millis() - lastMove >= (unsigned long)adjustedSpeed) {
    lastMove = millis();
    jumpToLed(pos);
    pos += dir;
    if (pos >= 9 || pos <= 0) dir *= -1;
  }
}

inline void patternGentleWave() {
  static int pos = 0;
  static unsigned long lastMove = 0;
  int waveSpeed = speed / 3;
  if (millis() - lastMove >= (unsigned long)waveSpeed) {
    lastMove = millis();
    jumpToLed(pos % 10);
    pos++;
  }
}

inline void patternSlowFill() {
  static int step = 0;
  static bool filling = true;
  static unsigned long lastStep = 0;
  if (millis() - lastStep >= (unsigned long)speed) {
    lastStep = millis();
    if (filling) {
      jumpToLed(step);
      step++;
      if (step >= 10) {
        filling = false;
        step = 9;
        delay(speed * 2);
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

inline void patternPingPong() {
  static int pos = 0;
  static int dir = 1;
  static unsigned long lastMove = 0;
  if (millis() - lastMove >= (unsigned long)speed) {
    lastMove = millis();
    jumpToLed(pos);
    delay(30);
    jumpToLed(9 - pos);
    pos += dir;
    if (pos >= 5 || pos <= 0) dir *= -1;
  }
}

inline void patternEvenOdd() {
  static bool evenOn = true;
  static unsigned long lastSwitch = 0;
  if (millis() - lastSwitch >= (unsigned long)(speed * 2)) {
    lastSwitch = millis();
    if (evenOn) {
      for (int i = 0; i < 10; i += 2) { jumpToLed(i); delay(20); }
    } else {
      for (int i = 1; i < 10; i += 2) { jumpToLed(i); delay(20); }
    }
    evenOn = !evenOn;
  }
}

inline void patternSparkle() {
  static unsigned long lastSparkle = 0;
  if (millis() - lastSparkle >= (unsigned long)(speed / 2)) {
    lastSparkle = millis();
    jumpToLed(random(10));
  }
}

inline void patternSweepBounce() {
  static int pos = 0;
  static int dir = 1;
  static unsigned long lastMove = 0;
  int sweepSpeed = speed / 4;
  if (millis() - lastMove >= (unsigned long)sweepSpeed) {
    lastMove = millis();
    jumpToLed(pos);
    pos += dir;
    if (pos >= 9) { dir = -1; delay(speed); }
    else if (pos <= 0) { dir = 1; delay(speed); }
  }
}

inline void runCountdown() {
  if (!countdownActive) return;
  if (millis() - countdownTimer >= 1000) {
    countdownTimer = millis();
    if (countdownValue > 0) {
      resetCounter();
      for (int i = 0; i < countdownValue; i++) { clockPulse(); delayMicroseconds(100); }
      countdownValue--;
    } else {
      resetCounter(); delay(200);
      for (int i = 0; i < 10; i++) { clockPulse(); delayMicroseconds(50); }
      delay(200);
      countdownActive = false;
      countdownValue = 10;
    }
  }
}

#endif