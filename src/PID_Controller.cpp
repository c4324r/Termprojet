/**
 * @file PID_Controller.cpp
 * 
 * This file contains the funciton defitions for the PID Controller.
 * This includes get functions for the PID constants and the setpoint and
 * a run task that calculates a new PWM value for the motor. This value
 * is caluclated from the different between the current angle and current
 * setpoint. The run function also handles dithering and integral value
 * saturation.
 * 
 * @author Mathew Smith and Cal Miller
 * @date 2023-12-08
 */

#include <Arduino.h>
#include "PID_Controller.h"
#include "MotorDriver.h"

/**
 * @brief       Constructs the PID Controller Class
 * @details     The PID controller needs to know what motor to control,
 *              initial PID values, the setpoint, and the period it updates
 *              at.
 * @param drive         The motor that the controller updates
 * @param portional     The initial KP value
 * @param integral      The initial KI value
 * @param derivative    The initial KD value
 * @param point         The initial setpoint
 * @param per           The update period
 */
PID_Controller::PID_Controller(MotorDriver drive, float portional, float integral, float derivative, float point, uint8_t per){
    motor = drive;
    kp = portional;
    ki = integral;
    kd = derivative;
    setpoint = point;
    period = per;
    prev_error = 0;
}
/**
 * @brief Gets the setpoint of the controller
 * 
 * @return float 
 */
float PID_Controller::get_setpoint(){
    return setpoint;
}

/**
 * @brief Updates the setpoint of the controller
 * 
 * @param point new Setpoint value
 */
void PID_Controller::set_setpoint(float point){
    setpoint = point;
}

/**
 * @brief Updates the KP of the controller
 * 
 * @param new_kp new KP value
 */
void PID_Controller::set_kp(float new_kp){
    kp = new_kp;
}

/**
 * @brief Updates the KI of the controller
 * 
 * @param new_ki new KI value
 */
void PID_Controller::set_ki(float new_ki){
    ki = new_ki;
}

/**
 * @brief Updates the KD of the controller
 * 
 * @param new_kd new KD value
 */
void PID_Controller::set_kd(float new_kd){
    kd = new_kd;
}
 /**
  * @brief Runs the controller to update the motor
  * @details The run function task the angle of the bike and compares
  *          it to the current setpoint. The error between them then 
  *          use to calculate the PWM for the motor. The error is also
  *          used to change how we change the setpoint due to dithering.
  * 
  * @param val current angle of the bike
  * @return float 
  */
float PID_Controller::run(float val){
    float error = (setpoint - val);
    total_error += error;
    float sat = 50 / ki;
    float increment = 0.0002;
    if(error < 0){
        setpoint -= increment;
    }else{
        setpoint += increment;
    }
    if(total_error >= sat){
        total_error = sat;
    }else if(total_error <= -sat){
        total_error = -sat;
    }
    float kiVal = ki * total_error;
    float pwmVal = (kp * error) + kiVal + (kd * ((error - prev_error) / period));
    motor.setPWM(pwmVal);
    prev_error = error;
    return pwmVal;
}
        