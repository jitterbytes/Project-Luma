/**
 * @file animations.h
 * @author sarvesh
 * @brief Animations and visual effects
 * This file declares all animation primitives, effects and visual state machines used by the application
 * @version 1.0
 * @date 2025-12-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "stdint.h"
#include "ws2812b.h"

// ==================== Screensaver Animation ====================
enum SaverPhase {   // States of the moving orb
    MOVE,           // normal motion
    VIBRATE,        // anticipation    
    EXPLODE         // explosion
};

void startPixelExplosion(int row, int col); // Starts pixel explosion
bool isExplosionDone();                     // checks if explosion is active
void updatePixelExplosion();                // updating explosion animation

// ==================== Menu Preview - Color flood & Falling pixel ====================
void drawMenu_ColorFlood();     // Menu Preview Animation for Color Flood 
void drawMenu_FallingPixel();   // Menu Preview Animation for Falling Pixel

// ******************** Color Fade Page ******************** 
void ColorFlood_Init();
void ColorFlood_StartNew();
void ColorFlood_Update();

// ******************** Faliing Pixels Page ******************** 
void FallingPixel_Init();
void FallingPixel_Spawn(uint8_t count);
void FallingPixel_Update();
bool FallingPixel_IsFull();
void FallingPixel_Explosion();


#endif