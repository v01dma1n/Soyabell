#include "SoyabellManager.h"

SoyabellManager soyabell;

void setup() {
    Serial.begin(115200);

    soyabell.begin();
}

void loop() {
    soyabell.loop();
}