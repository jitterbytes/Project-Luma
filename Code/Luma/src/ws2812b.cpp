/**
 * @file ws2812b.cpp
 * @author sarvesh
 * @brief Implementation of ws2812b.h 
 * @version 1.0
 * @date 2025-12-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "ws2812b.h"

// Matrix Instance
Adafruit_NeoPixel matrix(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

/**
 * @brief Convert 2D matrix coordinates to 1D pixel index
 * 
 * @param row Row index (0-7)
 * @param col Column index (0-7)  
 * @return int Linear pixel index
 */
int pixelIndex(int row, int col) {
  return row * WIDTH + col;
}