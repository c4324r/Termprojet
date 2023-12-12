/**
 * @file IMU.cpp
 * 
 * This file contains the funciton defitions for the IMU class. This
 * class has two functions, one to start reading from the IMU, and one
 * to get values from the IMU. 
 * 
 * @author Mathew Smith and Cal Miller
 * @date 2023-12-08
 */
#include <Arduino.h>
#include <Adafruit_BNO08x.h>
#include <Wire.h>
#include "IMU.h"

//define BNO Reset
#define BNO08X_RESET -1

/**
 * @brief Construct a new IMU::IMU object
 * 
 * @param addr Address of the I2C on the IMU
 * @param SCL The pin being used for SCL
 * @param SDA The pin being used for SDA
 */
IMU::IMU(uint8_t addr, uint8_t SCL, uint8_t SDA){
    i2c_addr = addr;
    Wire.begin(SCL, SDA);
}

/**
 * @brief Starts the IMU to begin reading values
 * @details This function starts the IMU to it can read from the IMU
 *          and calculate the rotation vector from the raw data. We are
 *          using a BNO085 so it is able to merge the value from the 
 *          accelerometer and gyro to give us values in the i, j, k and r
 *          vectors.
 * 
 */
void IMU::start(){
    if(!bno08x.begin_I2C(i2c_addr)){
        Serial.println("Could not find chip");
        for(;;){
            exit(1);
        }
    }
    Serial.println("Found");

    if(!bno08x.enableReport(SH2_ROTATION_VECTOR)){
        Serial.println("Failed");
    }else{
        Serial.println("Succeeded");
    }
    delay(100);
}

/**
 * @brief When a new value is availabe, the BNO will provide in the form of
 *        i, j, k, and r. Using some math we are able to convert these
 *        values to yaw, pitch, and roll angles. Depending on the values
 *        used to calculate the roll angle, we may need to change the sign.
 * 
 * @return float 
 */
float IMU::getVal(){
    if(bno08x.wasReset()){
        bno08x.enableReport(SH2_ROTATION_VECTOR);
        Serial.println("RESET LMAOOOO");
        delay(100);
    }
    //If there is not a value ready, return a float value that is too high
    while(!bno08x.getSensorEvent(&sensorValue)){
        return 12;
    }
    //Get the float value from the IMU
    switch(sensorValue.sensorId){
        case SH2_ROTATION_VECTOR:
            float i = sensorValue.un.rotationVector.i;
            float j = sensorValue.un.rotationVector.j;
            float k = sensorValue.un.rotationVector.k;
            float r = sensorValue.un.rotationVector.real;
            float a = sensorValue.un.rotationVector.accuracy;
/*             Serial.print("a: ");
            Serial.print(a);
            Serial.print(", i: ");
            Serial.print(i);
            Serial.print(", j: ");
            Serial.print(j);
            Serial.print(", k: ");
            Serial.print(k);
            Serial.print(", r: ");
            Serial.print(r);
            // Pitch will tell us what angle we are at, roll will tell us if we are in the positive or negative direction
            Serial.print(", Yaw: ");
            Serial.print(atan2(2.0 * (i * j + k * r), (sq(i) - sq(j) - sq(k) + sq(r)))); 
            Serial.print(", Pitch: ");
            Serial.print(asin(-2.0 * (i * k - j * r) / (sq(i) + sq(j) + sq(k) + sq(r))));
            Serial.print(", Roll: ");
            Serial.println(atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r)))); */
            if((-sq(i) - sq(j) + sq(k) + sq(r)) > 0){
                if(atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r))) > 0){
                    return atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r)));
                }else{
                    return atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r)));
                }
            }else{
                if(atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r))) > 0){
                    return atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r))) - 3.14;
                }else{
                    return atan2(2.0 * (j * k + i * r), (-sq(i) - sq(j) + sq(k) + sq(r))) + 3.14;
                }
            }
    }
    return 0;
}