/**
 * @file fsm.h
 * @author sarvesh
 * @brief LUMA - MAIN FSM LOGIC
 * @version 1
 * @date 2026-1-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef FSM_H
#define FSM_H

#include <Arduino.h>

// ==================== Luma State Definition ====================
enum LumaState {               // States of Luma : Every Page/Screens
    STATE_DEVICE_ON,           // [Device] Power-on animation (1.5 sec)
    STATE_DEVICE_SCREENSAVER,  // [Device] Idle breathing hourglass (infinite)
    STATE_DEVICE_MENU,         // [Device] Choose TIMER or THEMES (infinite)
    STATE_COLOR_FLOOD,         // Color flood interaction
    STATE_FALLING_PIXEL,       // Falling Pixel interaction
    STATE_ERROR                // Error state (optional)
};

// ==================== Luma Menu Option ====================
enum MenuOption {           // Sub States of FSM - Menu Option
    MENU_COLOR_FLOOD,       // Corresponds to STATE_COLOR_FLOOD
    MENU_FALLING_PIXELS,    // Corresponds to STATE_FALLING_PIXEL
    MENU_COUNT              // This is used to wrap around and bound checking
};
extern const char* menuNames[MENU_COUNT];   // Global read only array of menu names


// ==================== FSM Class ====================
class LumaFSM {
    public:
        LumaFSM();                              // Constructors sets defaults - intial settings
        void update();                          // main loop handles current state (calls every 20ms)
        void onButtonAPressed(bool longPress);  // Button A logic
        void onButtonBPressed(bool longPress);  // Button B logic
        
        // Read current values
        LumaState getCurrentState() const { return currentState; }              // Gets the current FSM State  
        MenuOption getCurrentMenuOption() const { return selectedMenuOption; }  // Gets the current FSM Sub State

    private:
        // States Tracking
        LumaState currentState;             // active state 
        LumaState previousState;            // previous state

        // User Selections
        MenuOption selectedMenuOption;      // Menu option selected by user

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