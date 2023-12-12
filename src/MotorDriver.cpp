/**
 * @file MotorDriver.cpp
 * 
 * This file contains the function defition for the MotorDriver class.
 * This class only have one fuction, and that is to change the PWM value
 * of the motor.
 * 
 * @author Mathew Smith and Cal Miller
 * @date 2023-12-08
 */
#include <Arduino.h>

#include "MotorDriver.h"

MotorDriver::MotorDriver(){
    IN1 = 1;
    IN2 = 2;
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
}

/**
 * @brief Construct a new Motor Driver:: Motor Driver object
 * @details. This class is intended to be used with a DVR8871 motor
 *           driver, although it should work with any motor driver
 *           that uses a two pins to control the motor.
 * 
 * @param addr_1 Address of the IN1 Pin of the motor driver
 * @param addr_2 Address of the IN2 Pin of the motor driver
 */
MotorDriver::MotorDriver(uint8_t addr_1, uint8_t addr_2){
    IN1 = addr_1;
    IN2 = addr_2;
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
}

/**
 * @brief Sets a new PWM value for the motor
 * @details The class takes a new value between -100 and 100
 *          and updates the pwm accordingly. If the values is 
 *          outside of this range, then it will saturate to the closest
 *          values
 * 
 * @param val PWM value
 */
void MotorDriver::setPWM(float val){
    float analogVal = val * 2.55;
    if(analogVal >= 255){
        analogVal = 255;
    }
    if(analogVal <= -255 ){
        analogVal = -255;
    }
    if(analogVal > 0){
        analogWrite(IN1, analogVal);
        digitalWrite(IN2, LOW);
    }else if(analogVal < 0){
        analogWrite(IN2, analogVal * -1);
        digitalWrite(IN1, LOW);
    }else{
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
    }
}