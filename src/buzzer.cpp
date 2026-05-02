#include "buzzer.h"
#include "config.h"
#include <Arduino.h>

void buzzer_init() {
    // tone() / noTone() handle pin setup automatically,
    // but ensure the pin starts silent.
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void buzzer_shutter() {
    // Short high click  →  softer low tick
    tone(BUZZER_PIN, 4000, 25);
    delay(35);
    tone(BUZZER_PIN, 2500, 45);
    delay(55);
    noTone(BUZZER_PIN);
}
