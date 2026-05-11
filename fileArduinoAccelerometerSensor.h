#pragma once

#include "Arduino.h"

struct AccelerometerSensorStub {
    void setup() {}
    int read() const { return 0; }
};

inline AccelerometerSensorStub Accelerometer;
