/**
 * @file fsm.cpp
 * @author sarvesh
 * @brief Implementation of fsm.h 
 * @version 1.0
 * @date 2026-1-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "fsm.h"
#include "ws2812b.h"
#include "animations.h"

// === Screensaver global variables (The FSM and Button Handler both need this) ===
static SaverPhase phase = MOVE;
static unsigned long phaseStart = 0;

// Menu Label Strings
const char* menuNames[MENU_COUNT] = {
    "COLOR FLOOD",
    "FALLING PIXELS"
};

// ==================== Constructor [Initializng Valid States] ====================
LumaFSM::LumaFSM()
    : currentState(STATE_DEVICE_ON),    // starts in boot animation
      previousState(STATE_DEVICE_ON),   // same as current state so no false transition
      stateStartTime(millis()),         // starts state timer
      timerStartTime(0),
      totalTimerDuration(0) {
        Serial.println("[FSM] LUMA Initialized - Starting STATE_DEVICE_ON");
}

// ==================== Main Update Loop ====================
// First public function - Called every 20ms -> 1000ms / 20ms = 50FPS
void LumaFSM::update() {
    // Check if state has changed (for transition logging)
    if (currentState != previousState) {
        logStateTransition(previousState, currentState);
        stateStartTime = millis();
        previousState = currentState;
    }

    // Dispatch to appropriate state handler - Each state to its corresponding state handler
    switch (currentState) {
        case STATE_DEVICE_ON:
            handleState_DeviceOn();
            break;
        case STATE_DEVICE_SCREENSAVER:
            handleState_DeviceScreensaver();
            break;
        case STATE_DEVICE_MENU:
            handleState_DeviceMenu();
            break;
        case STATE_COLOR_FLOOD:
            handleState_ColorFlood();
            break;
        case STATE_FALLING_PIXEL:
            handleState_FallingPixel();
            break;
        default:
            Serial.println("[FSM] ERROR: Unknown state!");
            break;
    }
}

// ==================== Button Inputs ====================
// Button A : All state transistions are mentioned below
void LumaFSM::onButtonAPressed(bool longPress) {
    Serial.print("[BTN] Button A ");
    Serial.println(longPress ? "LONG PRESS" : "SHORT PRESS");
    
    // Button A behavior varies by state
    switch (currentState) {

        case STATE_DEVICE_SCREENSAVER:
            if (!longPress) {   // Short press
                Serial.println("[ACTION] Screensaver -> Menu (Button A short)");
                transitionTo(STATE_DEVICE_MENU);
            }
            else {              // Long press
                Serial.println("[BTN] Button A long press ignored in this state");
            }
            break;

        case STATE_DEVICE_MENU:  
            if (!longPress) {   // Short press   
                Serial.println("[ACTION] Screensaver <- Menu (Button A short)");
                transitionTo(STATE_DEVICE_SCREENSAVER);
            }
            else {              // Long press
                Serial.println("[BTN] Button A long press ignored in this state");
            }
            break;   
        case STATE_COLOR_FLOOD:
            if (!longPress) {   // Short press
                Serial.println("[ACTION] Menu <- Color Flood (Button A short)");
                transitionTo(STATE_DEVICE_MENU);
            }
            else                // Long press
                Serial.println("[BTN] Button A long press ignored in this state");
            break;

        case STATE_FALLING_PIXEL:
            if(!longPress) {    // Short press
                Serial.println("[BTN] Button A short press ignored in this state");
            }
            else                // Long press
                Serial.println("[ACTION] Menu <- Falling Pixel (Button A long)");
                transitionTo(STATE_DEVICE_MENU);
        default:
            Serial.println("[BTN] Button A ignored in this state");
            break;
    }
}

// Button B : All state transistions are mentioned here
void LumaFSM::onButtonBPressed(bool longPress) {
    Serial.print("[BTN] Button B ");
    Serial.println(longPress ? "LONG PRESS" : "SHORT PRESS");
    
    // Button B behavior varies by state
    switch (currentState) {

        case STATE_DEVICE_SCREENSAVER:  
            if (!longPress) {   // Short press
                Serial.println("[ACTION] Orb hit it will explode");
                phaseStart = millis();
                phase = VIBRATE;
            }
            else {              // Long press
                Serial.println("[BTN] Button B long press ignored in this state");
            }
            break;

        case STATE_DEVICE_MENU:
            if (!longPress) {   // Short Press B to Cycle menu option
                selectedMenuOption = (MenuOption)((selectedMenuOption + 1) % MENU_COUNT);
                Serial.print("[ACTION] Menu cycled -> ");
                Serial.println(menuNames[selectedMenuOption]);
            } 
            else {              // Long press B to select the menu option
                if (selectedMenuOption == MENU_COLOR_FLOOD) {
                    Serial.println("[ACTION] Menu selected Color Flood");
                    transitionTo(STATE_COLOR_FLOOD);
                } else if (selectedMenuOption == MENU_FALLING_PIXELS) {
                    Serial.println("[ACTION] Menu selected Falling Pixels");
                    transitionTo(STATE_FALLING_PIXEL);
                }
            }
            break;   
        case STATE_COLOR_FLOOD:
            if (!longPress) {   // Short press
                ColorFlood_StartNew();   // inject new color
                Serial.println("[ACTION] New Color Flood Injected");
            } 
            else {              // long press
                Serial.println("[BTN] Button B long press ignored in this state");
            }
            break;
            
        case STATE_FALLING_PIXEL:
            if(!longPress) {    // short press Button B
                FallingPixel_Spawn(1);  // drops 1 pixel
                Serial.println("[ACTION] Drops One Pixel");
                }
            else {              // long press Button B
                uint8_t numPixels = random(8, 10);   // drops 8-10 pixel so that it could fill fasters, testing phase
                FallingPixel_Spawn(numPixels);
                Serial.println("[ACTION] Drops Multiple Pixels");
            }
            break;
        default:
            Serial.println("[BTN] Button B ignored in this state");
            break;
    }
}

// ==================== State Handlers (These are functions that run) ====================
/**
 * @brief Description of the start up animation TV bars
 * 
 * This animation has 3 distinct phase
 * 1. Boot Reveal - TV bars appear column by column alternate directions per column (scanline feel)
 * 2. Signal Hold - Pattern stays visible briefly 
 * 3. Signal Collapse - Glitch exit Random pixel turn off, image collapse organically  
 * 
 */
void LumaFSM::handleState_DeviceOn() {
    unsigned long elapsed = getStateElapsedTime();    // Starts the state timer
    
    const unsigned long STEP_MS = 250;                // Time alloted per column | Every 250ms next column starts
    const unsigned long ROW_MS  = 150;                // Time per row within column | Controls vertical sweep feel

    int colStep = elapsed / STEP_MS;                  //  Counts the Columns (phase 1)

    static unsigned long holdStart = 0;               // variable for holding signal (phase 2) 
    
    static bool collapse_start = false;               // flag variable for collapse exit (phase 3)
    static uint8_t offCount = 0;                      // holds count of off leds (phase 3)

    uint32_t bars[8] = {                  // Color of the bars
        matrix.ColorHSV(0,     0,   10),  // White
        matrix.ColorHSV(9000,  255, 10),  // Yellow
        matrix.ColorHSV(30000, 255, 10),  // Cyan
        matrix.ColorHSV(20000, 255, 10),  // Green
        matrix.ColorHSV(50000, 255, 10),  // Magenta
        matrix.ColorHSV(0,     255, 10),  // Red
        matrix.ColorHSV(42000, 255, 10),  // Blue
        matrix.ColorHSV(0,     0,   10)   // Black
    };

    // Phase 2 and Phase 3
    if (colStep >= WIDTH) {                             // When all the columns are printed completely
        if (!collapse_start) {                          // Hold final image Phase 2
            if (holdStart == 0) holdStart = millis();   // start hold timer

            if (millis() - holdStart < 2000) {          // Holding image for 2s
                matrix.show();
                return;
            }

            // Start Phase 3 collapse exit and resetting counters
            collapse_start = true;
            offCount = 0;
            holdStart = 0;
        }

        // Signal Collapse exit Phase 3
        for (int k = 0; k < 3; k++) {       // each frame kills 3 leds | increase the value for faster collapse
            int r = random(0, HEIGHT);      // randomly selects the row and column   
            int c = random(0, WIDTH);
            int idx = pixelIndex(r, c);

            if (matrix.getPixelColor(idx) != 0) {   // Only turn off if still on
                matrix.setPixelColor(idx, 0);
                offCount++;
            }
        }
        matrix.show();

        // Here either wait for all the 64 leds to turn off as i did below 
        // or multiply (WIDTH * HEIGHT) by 0.9 so around 57 leds are off the state will transition
        if (offCount >= (WIDTH * HEIGHT)) {
            collapse_start = false;
            transitionTo(STATE_DEVICE_SCREENSAVER);
        }
        return;
    }

    // Phase 1
    matrix.clear();     // clears frame buffer before drawing

    // Fully revealed columns - if we remove this loop then the revealed columns won't hold their colors thats why its necessary to redraw all the revealed columns
    for (int c = 0; c < colStep && c < WIDTH; c++) {
        for (int r = 0; r < HEIGHT; r++) {
            matrix.setPixelColor(pixelIndex(r, c), bars[c]);
        }
    }

    if (colStep < WIDTH) {  // Currently revealing column - row progression in column
        int rowStep = ((elapsed % STEP_MS) * HEIGHT) / STEP_MS; // Row progression and it scales from 0...7

        for (int i = 0; i <= rowStep && i < HEIGHT; i++) {
            int r = (colStep % 2 == 0) ? i : (HEIGHT - 1 - i);  // for even columns the row progressions starts from top to bottom and for odd bottom to top
            matrix.setPixelColor(pixelIndex(r, colStep), bars[colStep]);
        }
    }
    matrix.show();
}

void LumaFSM::handleState_DeviceScreensaver() {

    // Infinite state, waiting for button press
    // Button B short press -> make orb explode
    // Button A short press -> MENU (handled in onButtonAPressed)
    // Button B/A long press -> No action

    static unsigned long lastStep = 0;
    static int pxRow = HEIGHT / 2;          // the pixel orb sstarts at (4,0)
    static int pxCol = 0;
    static uint32_t pxColor = matrix.ColorHSV(40000, 255, 10);  // pixel orb color

    const unsigned long MOVE_MS    = 100;  // controls the speed of pixel in idle animation  low value -> higher speed and vice versa
    const unsigned long VIBRATE_MS = 300;  // this controls the vibrate phase

    // === MOVE ===
    if (phase == MOVE) {    // Moving animation loop

        if (millis() - lastStep < MOVE_MS) return;  // Frame Control Rate 1000ms / 100ms -> 10FPS

        lastStep = millis();

        matrix.clear();
        matrix.setPixelColor(pixelIndex(pxRow, pxCol), pxColor);    // start pixel at 4,0
        matrix.show();

        pxCol++;            // Move the orb to the right column
        if (pxCol >= WIDTH) // If the orb reached the end wrap around
            pxCol = 0;      // start from column 0

        return;
    }

    // === VIBRATE ===
    if (phase == VIBRATE) { // feedback phase triggered by Button B short press micro anticipation before the explosion

        if (millis() - phaseStart > VIBRATE_MS) {   // if vibrate phase done then explode
            startPixelExplosion(pxRow, pxCol);
            phase = EXPLODE;
            return;
        }

        // in vibrate phase creating micro jitter to build for explosion 
        matrix.clear();
        int vr = pxRow + random(-1, 2); // this micro anticipation jitter will be of 3x3 matrix -1,0,1
        int vc = pxCol + random(-1, 2);

        if (vr >= 0 && vr < HEIGHT && vc >= 0 && vc < WIDTH) {  // Bounding checking vr and vc
            matrix.setPixelColor(pixelIndex(vr, vc), pxColor);
        }
        matrix.show();
        return;
    }

    // === EXPLODE ===
    if (phase == EXPLODE) {             // Explosion 

        updatePixelExplosion();         // Continously update the Explosion animation
        if (isExplosionDone()) {        // if explosion done choose new starting point
            pxRow = random(0, HEIGHT);  // choose random origin coordinates
            pxCol = random(0, WIDTH);

            pxColor = matrix.ColorHSV(random(0, 65535), 255, 10); // change color
            phase = MOVE;
        }
    }
}

void LumaFSM::handleState_DeviceMenu() {
    // Infinite state, showing current menu option
    // Button B short press -> cycle menu
    // Button B long press -> select menu
    // Button A short press -> back to screensaver
    // Button A long press unused

    // Optional: Print current selection periodically (every 2 seconds)
    unsigned long elapsed = getStateElapsedTime();
    if (elapsed % 2000 < 20) { // Trigger once per 2 seconds
        Serial.print("[STATE_MENU] Selected: ");
        Serial.println(menuNames[selectedMenuOption]);
    }

    switch (selectedMenuOption)     // switch Menu Options
    {
        case MENU_COLOR_FLOOD:
        drawMenu_ColorFlood();      // color flood preview
        break;

        case MENU_FALLING_PIXELS:
        drawMenu_FallingPixel();    // falling pixels preview
        break;

        default:
        break;
    }

}

// ===== Color Flood State Handlers =====
static bool colorFloodInit = false; // Color Flood Initializing Flag for first entry
void LumaFSM::handleState_ColorFlood() {

    if (!colorFloodInit) {      // Initialize Color Flood
        ColorFlood_Init();
        colorFloodInit = true;  
    }

    ColorFlood_Update();    // Animation Engine of Color Flood Called every 20ms according to the main FSM
}

// ===== Falling Pixel State Handlers =====
static bool fallingpixel_init = false;  // Falling Pixel Initializing Flag on first entry
void LumaFSM::handleState_FallingPixel() {

    if (!fallingpixel_init) {   // Initialize Falling Pixels
        FallingPixel_Init();
        fallingpixel_init = true;
    }

    FallingPixel_Update();  // Animation Engine of Falling Pixel Called every 20ms according to the main FSM  

    if (FallingPixel_IsFull()) {    // If the matrix full proceed to Explosion 
        FallingPixel_Explosion();
    }
}

// ==================== Transition Handler ====================
void LumaFSM::transitionTo(LumaState newState) {
    if (newState != currentState) {
        currentState = newState;
    }

    // Commenting this below part so that user can resume the falling pixel page from where it left from
    // when leaving falling pixel page, reset the flag so the user start with clear matrix
    // if (currentState == STATE_FALLING_PIXEL) { 
    //     fallingpixel_init = false;
    // }

    if (currentState != STATE_COLOR_FLOOD) {    // Color Flood will start from fresh if you left the page and came back
        colorFloodInit = false;
    }
}

// ==================== Utilities (Used for Serial Debugging) ====================
unsigned long LumaFSM::getStateElapsedTime() const {
    return millis() - stateStartTime;
}

void LumaFSM::logStateTransition(LumaState from, LumaState to) {    
    const char* stateNames[] = {
        "DEVICE_ON",
        "SCREENSAVER",
        "MENU",
        "SELECT_TIME",
        "COUNTDOWN",
        "RUNNING",
        "PAUSED",
        "FINISHED",
        "THEMES",
        "FALLING PIXEL",
        "ERROR"
    };
    
    Serial.print("[TRANSITION] ");
    Serial.print(stateNames[from]);
    Serial.print(" -> ");
    Serial.println(stateNames[to]);
}


