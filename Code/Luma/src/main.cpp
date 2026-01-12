#include <Arduino.h>
#include "fsm.h"

// Function Prototypes 
void pollButtons();

// Create one global FSM object for Class LumaFSM - has all the functions 
LumaFSM fsm;

// === Button & Debouncing Configs === 
const int BUTTON_A_PIN = 5;     // GPIO 5 
const int BUTTON_B_PIN = 2;     // GPIO 2

const int DEBOUNCE_TIME = 20;       // 20ms
const int LONG_PRESS_TIME = 1000;   // 1s

unsigned long buttonA_DownTime = 0;  // tracks the time for A
unsigned long buttonB_DownTime = 0;  // tracks the time for B
bool buttonA_WasDown = false;        // flag for button press - A      
bool buttonB_WasDown = false;        // flag for button press - B

void setup(){
  Serial.begin(115200);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);  // Button A 
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);  // Button B
}

void loop(){
  fsm.update();     // Asks FSM what to do now - This updates FSM every 20ms
  pollButtons();    // Polls Button - to check for any button presses
  delay(20);        // 50 FPS update rate
}

// === Button Polling ===
void pollButtons() {
    
    // === Button A logic ===
    bool buttonA_Now = digitalRead(BUTTON_A_PIN) == LOW; // Pressed == LOW -> then o/p 1 , Released == HIGH -> then o/p 0
    
    if (buttonA_Now && !buttonA_WasDown) {    // Button A just pressed
        buttonA_DownTime = millis();         // Start timer 
        buttonA_WasDown = true;
    } 
    else if (!buttonA_Now && buttonA_WasDown) {   // Button A just released
        unsigned long press_Duration = millis() - buttonA_DownTime;   // Calculate the press duration
        buttonA_WasDown = false;
        
        if (press_Duration > DEBOUNCE_TIME) {        // Ignore any press less than Debounce time(20ms) - noise
            if (press_Duration > LONG_PRESS_TIME)  
                fsm.onButtonAPressed(true);  // Long Press        
            else                              
                fsm.onButtonAPressed(false); // Short press      
        }
    }
    
    // === Button B logic ===
    bool buttonB_Now = digitalRead(BUTTON_B_PIN) == LOW; // Pressed == LOW -> then o/p 1 , Released == HIGH -> then o/p 0
    
    if (buttonB_Now && !buttonB_WasDown) {    // Button B just pressed
        buttonB_DownTime = millis();         // Start timer
        buttonB_WasDown = true;
    } else if (!buttonB_Now && buttonB_WasDown) {     // Button B just released
        unsigned long press_Duration = millis() - buttonB_DownTime;   // Calculate the press duration
        buttonB_WasDown = false;
        
        if (press_Duration > DEBOUNCE_TIME) {        // Ignore any press less than Debounce time(20ms) - noise   
            if (press_Duration > LONG_PRESS_TIME) 
                fsm.onButtonBPressed(true); // Long Press
            else 
                fsm.onButtonBPressed(false); // Short press 
        }
    }
}