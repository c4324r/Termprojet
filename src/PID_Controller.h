/**
 * @file PID_Controller.h
 * 
 * This file is te header file for the PID controller task which lays out how
 * the PID controller is setup. The function definitions can be found in
 * the PID_Controller.cpp file.
 * 
 * @author Mathew Smith and Cal Miller
 * @date 2023-12-8
 * 
 */
#ifndef PID_Controller_h
#define PID_Controller_h

#include <Arduino.h>
#include "PID_Controller.h"
#include "MotorDriver.h"

class PID_Controller{
    private:
        MotorDriver motor;
        float kp;
        float ki;
        float kd;
        float total_error;
        float setpoint;
        uint8_t period;
        float prev_error;
    public:  
        PID_Controller(MotorDriver drive, float portional, float integral, float derivative, float point, uint8_t per);
        void set_setpoint(float point);
        float run(float val);
        void set_kp(float new_kp);
        void set_ki(float new_ki);
        void set_kd(float new_kd);
        float get_setpoint();
};

#endif