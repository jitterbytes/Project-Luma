/**
 * @file ws2812b.h
 * @author sarvesh
 * @brief WS2812B LED matrix driver utilities
 * @version 1.0
 * @date 2025-12-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef WS2812B_H
#define WS2812B_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Matrix Configurations 
#define DATA_PIN 10                 // WS2812B data line
#define WIDTH 8                     // Matrix width
#define HEIGHT 8                    // Matrix height
#define NUM_LEDS (WIDTH * HEIGHT)   // Total LEDs

// Global Neopixel Matrix Instance shared across the project
extern Adafruit_NeoPixel matrix;    

int pixelIndex(int row, int col);

#endif