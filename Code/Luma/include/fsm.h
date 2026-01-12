/**
 * @file fsm.h
 * @author Sarvesh
 * @brief LUMA - MAIN FSM LOGIC
 * @version 1
 * @date 2025-12-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

// === Luma State Definitions ===
/**
 * @brief These defines every pages/screens. FSM is always in one of these states
 * 
 */
enum LumaState {
    STATE_DEVICE_ON,             // [Device] Power-on animation (1.5 sec)
    STATE_DEVICE_SCREENSAVER,    // [Device] Idle breathing hourglass (infinite)
    STATE_DEVICE_MENU,           // [Device] Choose TIMER or THEMES (infinite)
    STATE_COLOR_FLOOD,           // Color flood (infinite)
    STATE_FALLING_PIXEL,         // Falling Pixel   
    STATE_ERROR                  // Error state (optional)
};

// === Luma Menu Options ===
enum MenuOption {
    MENU_THEMES,
    MENU_FALLING_PIXELS,
    MENU_COUNT
};
extern const char* menuNames[MENU_COUNT];


// === FSM Class ===
class LumaFSM {
    public:
        
        LumaFSM();  // Constructors sets defaults
        void update();  // main loop handles current state (calls every 20ms)
        void onButtonAPressed(bool longPress);  // Button A logic
        void onButtonBPressed(bool longPress);  // Button B logic
        
        // Read current values
        LumaState getCurrentState() const { return currentState; }  
        MenuOption getCurrentMenuOption() const { return selectedMenuOption; }

    private:
        
        // States
        LumaState currentState; 
        LumaState previousState;    

        // User Selections
        MenuOption selectedMenuOption;          // Menu option selected by user

        // State Timing
        unsigned long stateStartTime;       // When current state began
        unsigned long timerStartTime;       // When RUNNING state timer started
        unsigned long totalTimerDuration;   // Total ms for countdown

        // State Handlers
        void handleState_DeviceOn();
        void handleState_DeviceScreensaver();
        void handleState_DeviceMenu();
        void handleState_ColorFlood();
        void handleState_FallingPixel();

        // State Transitions 
        void transitionTo(LumaState newState);

        // Utilities
        unsigned long getStateElapsedTime() const;
        void logStateTransition(LumaState from, LumaState to);

};


#endif