/**
 * @file main.cpp
 * 
 * This file contains the main program for the bike balancing program
 * 
 * @author Mathew Smith and Cal Miller
 * @date 8-Dec-2023
 */
#include <Arduino.h>
#include <ESP32Servo.h>
#include <Adafruit_BNO08x.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

#include "taskshare.h"
#include "IMU.h"
#include "MotorDriver.h"
#include "PID_Controller.h"

Share<float> drive_val ("drive_val");
Share<float> angle_val ("angle_val");
Share<float> pwm_val ("pwm_val");
Share<int> steer_val("steer_val");

//Define motor control pins
#define IN1_1 13
#define IN2_1 12
#define IN1_2 14
#define IN2_2 27

//Define Servo Pin
#define SERVO 32

//Define IMU Pins
#define IMU_ADDR 0x4A
#define IMU_SCL 21
#define IMU_SDA 22

// Initialize Servo Controller
Servo myservo;

// Initialize Motor Controller
MotorDriver motor2 = MotorDriver(IN1_1, IN2_1);

MotorDriver motor1 = MotorDriver(IN1_2, IN2_2);
  
// IMU Class
IMU bno = IMU(IMU_ADDR, IMU_SCL, IMU_SDA);

// Controller Class
PID_Controller controller = PID_Controller(motor1, 225, 0.1, 1000, 0, 1);

//Define SSID and Password
const char* ssid = "Controller";
const char* password = "password1";

WebServer server(80);

//Raw HTML text
const char html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Web Server</title>
  <style>
    body {
      font-family: sans-serif;
    }
    #drive {
      appearance: slider-vertical;
      width: 70%; /* Adjusted width for the drive slider */
    }
    .container {
      display: flex;
      justify-content: space-evenly;
    }
    #steer {
      width: 70%; /* Adjusted width for the trackbar slider */
    }
    .value{
      text-align: center;
    }
    .pwm{
      text-align: center;
    }
  </style>
</head>
<body>
  <h1>ESP32 Web Server</h1>
  <button type="submit" onClick="resetSetpoint()">Reset Setpoint</button>
  <br>
  <div class="container">
    <div>
      <h3>Drive State Control</h3>
      <input id="drive" type="range" min="-70" max="70" value="0" oninput="setDrive()">
    </div>
    <div>
      <h3>Steer State Control</h3>
      <input type="range" id="steer" min="45" max="135" step="1" value="90" oninput="setSteer()">
    </div>
  </div>
  <div>
    <div class = "value">
      <h1>
        Current Setpoint
      </h1>
      <h1 id="Setpoint"></h1>
      <h1>
        Current Angle
      </h1>
      <h1 id="Value">0</h1>
    </div>
    <div class = "pwm">
      <h1>Current PWM Value</h1>
      <h1 id="pwmValue">0</h1>
    </div>
  </div>
  <h1>KP</h1><input id="KP" type="number" step=".01"><input type="submit" value="Submit" onclick="updateKP()"><br>
  <h1>KI</h1><input id="KI" type="number" step=".01"><input type="submit" value="Submit" onclick="updateKI()"><br>
  <h1>KD</h1><input id="KD" type="number" step=".01"><input type="submit" value="Submit" onclick="updateKD()">
  <script>
    var drive = document.getElementById('drive');
    var steer = document.getElementById('steer');
    var pointValue = document.getElementById('Value');
    var pwmValue = document.getElementById('pwmValue');
    var setpointValue = document.getElementById('Setpoint');
    var kpValue = document.getElementById('KP');
    var kiValue = document.getElementById('KI');
    var kdValue = document.getElementById('KD');
    var driveVal = 0;
    var steerVal = 0;
    var kpVal = 0;
    var kiVal = 0;
    var kdVal = 0;
    function setDrive() {
      driveVal = drive.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/drive?value=' + driveVal, true);
      xhr.send();
    }
    function setSteer() {
      steerVal = steer.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/steer?value=' + steerVal, true);
      xhr.send();
    }
    function getValue(){
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function(){
        if(this.readyState == 4 && this.status == 200){
          pointValue.innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "readIMU", true);
      xhr.send();
    }
    function getPWM(){
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function(){
        if(this.readyState == 4 && this.status == 200){
          pwmValue.innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "readPWM", true);
      xhr.send();
    }
    function getSetpoint(){
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function(){
        if(this.readyState == 4 && this.status == 200){
          setpointValue.innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "readSetpoint", true);
      xhr.send();
    }
    function updateKP(){
      kpVal = kpValue.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/kp?value=' + kpVal, true);
      xhr.send();
    }
    function updateKI(){
      kiVal = kiValue.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/ki?value=' + kiVal, true);
      xhr.send();
    }
    function updateKD(){
      kdVal = kdValue.value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/kd?value=' + kdVal, true);
      xhr.send();
    }
    function resetSetpoint(){
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/resetSetpoint', true);
      xhr.send();
    }
    setInterval(function() {
  // Call a function repetatively with 2 Second interval
      getValue();
      getPWM();
      getSetpoint();
    }, 100); //2000mSeconds update rate
  </script>
</body>
</html>
)rawliteral";

/**
 * @brief     The task that controls the bike's reaction wheel
 * @details   This taks starts reading from the IMU. If there is a new
 *            data value then it will run the controller with the new value.
 *            If there is no new data, then it will countinue to resue the
 *            previous data. The controller will update the reaction wheel
 *            with a new pwm value, depending on the error from the 
 *            centerpoint. 
 */
void balance(void * p_params){
  Serial.println("Balance");
  bno.start();
  vTaskDelay(100);
  float point;
  float prev;
  float val;
  for(;;){
    point = bno.getVal();
    if(point == 12){
      point = prev;
    }
    prev = point;
    angle_val.put(point);
    val = controller.run(-1 * point);
    Serial.println(point);
    pwm_val.put(val);
    vTaskDelay(1);
  }
}

/**
 * @brief   The task that runs the drive motor
 * @details The task updates the drive motor pwm every time it is ran.
 *          The value it updates with comes from the web server.
 */
void drive(void * p_params){
  float value = drive_val.get();
  motor2.setPWM(0);
  for(;;){
    value = drive_val.get();
    motor2.setPWM(value);
    vTaskDelay(10);
  }
}

/**
 * @brief   The task that runs the steer servo
 * @details The task updates the steer servo every time it is ran.
 *          The value it updates with comes from the web server.
 */
void steer(void * p_params){
  myservo.attach(SERVO);
  myservo.write(90);
  float value = steer_val.get();
  for(;;){
    value = steer_val.get();
    myservo.write(value);
    vTaskDelay(10);
  }
}

/**
 * @brief   Sends the IMU value to the server
 * @details The task gets whatever the most recent IMU value from the
 *          reaction wheel task. It then sends this value to the server
 *          so it can update the webpage.
 */
void handleIMU(){
  float point = angle_val.get();
  String value = String(point);
  server.send(200, "text/plane", value);
}

/**
 * @brief   Sends the PWM value to the server
 * @details The task gets whatever the most recent PWM value from the
 *          reaction wheel task. It then sends this value to the server
 *          so it can update the webpage.
 */
void handlePWM(){
  float point = pwm_val.get();
  String value = String(point);
  server.send(200, "text/plane", value);
}

/**
 * @brief   Updates the controller with a new KP value
 * @details The task takes the new KP value from the server and
 *          changes the controller to the new value.
 */
void handleKP(){
  controller.set_kp(server.arg("value").toFloat());
}


/**
 * @brief   Updates the controller with a new KI value
 * @details The task takes the new KI value from the server and
 *          changes the controller to the new value.
 */
void handleKI(){
  controller.set_ki(server.arg("value").toFloat());
}

/**
 * @brief   Updates the controller with a new KD value
 * @details The task takes the new KD value from the server and
 *          changes the controller to the new value.
 */
void handleKD(){
  controller.set_kd(server.arg("value").toFloat());
}

/**
 * @brief   Sends the inital webpage on a new reqeust
 * @details Sends the new user the webpage so that the user can interact
 *          with the bike.
 */
void handleRoot(){
  server.send(200, "text/html", html);
}

/**
 * @brief   Updates the servo value
 * @details Changes the steer value so that the steering task
 *          can update the servo with a new value
 */
void handleSteer(){
  steer_val.put(server.arg("value").toInt());
}

/**
 * @brief   Updates the drive value
 * @details Changes the drive value so that the driving task
 *          can update the motor with a new value
 */
void handleDrive(){
  drive_val.put(server.arg("value").toFloat());
}

/**
 * @brief   Sends the Setpoint value to the server
 * @details The task gets whatever the most recent Setpoint value from the
 *          reaction wheel task. It then sends this value to the server
 *          so it can update the webpage.
 */
void handleSetpoint(){
  server.send(200, "text/plane", String(controller.get_setpoint()));
}

/**
 * @brief   Resets the setpoint value to 0
 * @details This function reset the setpoint of the controller to 0
 *          to counteract possible wandering from dithering
 */
void resetSetpoint(){
  controller.set_setpoint(0);
}


/**
 * @brief   Sets up certain aspects of the bike balancer
 * @details Sets up the server, serial, and multitasking function for
 *          the bike balancer.
 */
void setup() {
  //Begin Serial
  Serial.begin(115200);
  while (!Serial){delay(1);}
  Serial.println("Starting");
  WiFi.softAP(ssid, password);
  Serial.println("Setting Up ESP32 AP Network..");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/readIMU", handleIMU);
  server.on("/readPWM", handlePWM);
  server.on("/kp", handleKP);
  server.on("/ki", handleKI);
  server.on("/kd", handleKD);
  server.on("/steer", handleSteer);
  server.on("/drive", handleDrive);
  server.on("/readSetpoint", handleSetpoint);
  server.on("/resetSetpoint", resetSetpoint);
  server.begin();
  //Create Tasks
  xTaskCreate(balance, "Balance", 9182, NULL, 3, NULL);
  //xTaskCreate(handleServer, "Server", 9182, NULL, 0, NULL);
  xTaskCreate(drive, "Drive", 2048, NULL, 1, NULL);
  xTaskCreate(steer, "Steer", 2048, NULL, 2, NULL);
}

/**
 * @brief   Handles a new user when they connect to the ESP
 * @details When a new user connects or makes a new GET request,
 *          the server must know which function to run based on the
 *          request. The handle client function will tell the server
 *          where to go.
 */
void loop() {
  server.handleClient();
  vTaskDelay(1);
}
