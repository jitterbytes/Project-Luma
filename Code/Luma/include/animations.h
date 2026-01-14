/**
 * @file animations.h
 * @author sarvesh
 * @brief Animations and visual effects
 * This file declares all animation primitives, effects and visual state machines used by the application
 * @version 1.0
 * @date 2026-1-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "stdint.h"
#include "ws2812b.h"

// ==================== Screensaver Animation ====================
enum SaverPhase {   // States of the moving orb
    MOVE,           // normal motion
    VIBRATE,        // anticipation - when button pressed    
    EXPLODE         // explosion
};

void startPixelExplosion(int row, int col); // Starts pixel explosion
bool isExplosionDone();                     // checks if explosion is active
void updatePixelExplosion();                // updating explosion animation

// ==================== Menu Preview - Color flood & Falling pixel ====================
void drawMenu_ColorFlood();     // Menu Preview Animation for Color Flood 
void drawMenu_FallingPixel();   // Menu Preview Animation for Falling Pixel

// ******************** Color Fade Interaction ******************** 
void ColorFlood_Init();         // Initializes Color Flood Interaction
void ColorFlood_StartNew();     // Spawns a new Color Flood 
void ColorFlood_Update();       // Animation Engine for the Color Flood

// ******************** Faliing Pixels Interaction ******************** 
void FallingPixel_Init();               // Initializes Falling Pixel Interaction
void FallingPixel_Spawn(uint8_t count); // Spawns the pixels according to the button press
void FallingPixel_Update();             // Animation Engine for the Falling Pixel 
bool FallingPixel_IsFull();             // Checks if the column is full
void FallingPixel_Explosion();          // Hihg Level Animation End sequence


#endif