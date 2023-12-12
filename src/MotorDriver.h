/**
 * @file MotorDriver.h
 * 
 * This file is te header file for the Motor Driver Class which lays out how
 * the Motor Driver is setup. The function definitions can be found in
 * the MotorDriver.cpp file.
 * 
 * @author Mathew Smith and Cal Miller
 * @date 2023-12-8
 * 
 */
#ifndef MotorDriver_h
#define MotorDriver_h

#include <Arduino.h>

class MotorDriver{
    private:
        uint8_t IN1;
        uint8_t IN2;
    public:
        MotorDriver();
        MotorDriver(uint8_t addr_1, uint8_t addr_2);
        void setPWM(float val);
};

#endif