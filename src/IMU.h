/**
 * @file IMU.h
 * 
 * This file is te header file for the IMU class which lays out how
 * the IMU is setup. The function definitions can be found in
 * the IMU.cpp file.
 * 
 * @author Mathew Smith and Cal Miller
 * @date 2023-12-8
 * 
 */
#ifndef IMU_h
#define IMU_h

#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include <Wire.h>


class IMU{
    private:
        uint8_t i2c_addr;
        sh2_SensorValue_t sensorValue;
        Adafruit_BNO08x bno08x;
    public:
        IMU(uint8_t addr, uint8_t SCL, uint8_t SDA);
        void start();
        float getVal();
};

#endif