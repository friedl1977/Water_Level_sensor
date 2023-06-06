/*
 * Project Water_Level_sensor
 * Description:
 * Author:
 * Date:
 */


#include "../lib/tof/src/VL53L1X.h"
#include <Wire.h>

SYSTEM_MODE(SEMI_AUTOMATIC);                            // Enable  for control over system mode
SYSTEM_THREAD(ENABLED);                                 // DANGER!!! //

//--  ST VL53L1CD ToF declarations    --//
int XSHUT = D2;                                        // To power ToF On/Off
int LedPin = D3;

char x[] = "Level: ";
char y[] = "Distance: ";

int level = 0;
int current_level;
int new_level;

int L1_t = 4000, L1_b = 2500;                           // Water Level 1 range in mm
int L2_t = 2499, L2_b = 1000;                           // Water Level 2 range in mm
int L3_t = 999, L3_b = 500;                             // Water Level 3 range in mm

int tof_level = 0;

VL53L1X sensor;

const int numSamples = 100;                           // Number of reading from ToF that will be avaraged out to determine the final reading.  The higher this number, the
                                                      // more accurate thge reading will be but will consume more power.

int sensorValue;                                      // The integer value (between 0 and 4095) that we read from the sensor


void setup() {

    Wire.begin();
                               
    pinMode(XSHUT, OUTPUT);                                                                          // Set to LOW when using SYSTEM MODE SEMI_AUTOMATIC 
    digitalWrite(XSHUT, LOW);
    
    pinMode(LedPin, OUTPUT);
    digitalWrite(LedPin, LOW);
}

void ToF() {
    
    digitalWrite(XSHUT, HIGH);                                      // Enable ToF
    digitalWrite(LedPin, HIGH);                                     // Enable LED
  
    Wire.setClock(400000);                                          // use 400 kHz I2C     
    sensor.init(0x29);

    int value_prev = 0;                                             // The integaer value of the "prevous" sample.
    float val_now = 0.0;                                            // The measured values (integers) are converted to floats for the calculation.
    float val_prev = 0.0;
    float accum = 0.0;
    float avgValue = 0.0;

if (!sensor.init(0x29)) {
         Particle.publish("Could not find a valid ToF1 on bus 0, check wiring!");     //  DEBUG
     while (1);                                                                       //  DEBUG
     } else { Particle.publish("Tof Device OK");                                      //  DEBUG
    }

    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(50);                                                       // timing budget -- 50ms default

for (int k=0; k<numSamples; k++) {
        
        int distance = sensor.read();                                                 //Get the result of the measurement from the sensor
        val_prev = float(value_prev);
        val_now = float(distance);
        accum = (val_now + val_prev);

        value_prev = accum;
        avgValue = accum / numSamples;
    }

if (avgValue <=(L1_t) && avgValue >=(L1_b)) {            
        tof_level = 1;                               
    } else if                                       
        (avgValue <=(L2_t) && avgValue >=(L2_b)) {
        tof_level = 2;
    } else if                                       
        (avgValue <=(L3_t) && avgValue >=(L3_b)) {
        tof_level = 3;
    } else { tof_level = 4; 
}

    delay(50);
    digitalWrite(XSHUT, LOW);                                      // Disable ToF1 

if (tof_level != level) {                                          // only send data if Level has shifted
    
    if (Particle.connected() == false) {
        WiFi.on();
        waitUntil(WiFi.isOn);
        Particle.connect();
        waitUntil(Particle.connected);    
        } 
        
    if (Particle.connected() == true) {

        Particle.publish("Water Level: " + String(tof_level), PRIVATE);
        Particle.publish("Level Distance: " + String(avgValue), PRIVATE);
        }
    }    
    
    level = tof_level;
    delay(50);      
    
    digitalWrite (LedPin, LOW);
             
}


void Sleep() {

    Particle.disconnect(CloudDisconnectOptions().graceful(true).timeout(5000));       // Use only in SEMI_AUTOMATIC mode
    waitUntil(Particle.disconnected);
    WiFi.off();
    waitUntil(WiFi.isOff);

    delay(50);

    SystemSleepConfiguration config;
    config.mode(SystemSleepMode::ULTRA_LOW_POWER)
        .duration(1min);                                                              // set sleep duration to your preference (in min)                                                        
      
    System.sleep(config);
}


void loop() {

    ToF();
    Sleep();                                                                          // After readings system sleeps for duration set in Sleep() funtion
}