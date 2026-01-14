/**
 * @file animations.cpp
 * @author sarvesh
 * @brief Implementation of animations.h
 * @version 1.0
 * @date 2026-1-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "animations.h"
#include "ws2812b.h"

// ==================== Screensaver Animation ====================
static unsigned long explosionStart = 0;    // explosion start timer 
static bool active = false;                 // explosion state running or not
static int baseRow, baseCol;                // explosion origin coordinates
static uint32_t explosionColor;             // color of the explosion

/**
 * @brief Starts pixel explosion at the given position 
 * 
 * Locks the explosion origin, initializes the explosion timer & state 
 * and selects a random explosion color 
 * 
 * @param row Row index of explosion origin
 * @param col Column index of explosion origin
 */
void startPixelExplosion(int row, int col) {
    baseRow = row;
    baseCol = col;
    explosionStart = millis();
    active = true;

    explosionColor = matrix.ColorHSV(random(0, 65535), 255, 12);
}

/**
 * @brief Checks if explosion is done
 * 
 * @return true if done
 * @return false if not done 
 */
bool isExplosionDone() {
    return !active;
}

/**
 * @brief If explosion is active then this function will update the explosion
 * 
 */
void updatePixelExplosion() {
    if (!active) return;    // if explosion not active do nothing

    unsigned long t = millis() - explosionStart;

    matrix.clear();     // clearing matrix so explosion is dominant

    if (t < 500) {      // explosion lasts ~500ms
        // The main logic of explosion 
        for (int i = 0; i < 7; i++) {   // draws 6 pixel spark per frame  and with this logic max the explosion can have 4x4 radius
            int r = baseRow + random(-2, 2);    // -2 -1 0 1 -> 4x4 matrix
            int c = baseCol + random(-2, 2);    // Increasing the range increases the matrix size

            if (r >= 0 && r < HEIGHT && c >= 0 && c < WIDTH) {  // adding guards for r & c so they do not go out of range
                matrix.setPixelColor(pixelIndex(r, c), explosionColor);
            }
        }
    } else {
        active = false; // clearing the active flag after explosion
    }
    matrix.show();  // displaying matrix
}

 
// ==================== Menu Preview - Color flood & Falling pixel ====================
/**
 * @brief Menu Preview animation for Color flood
 * 
 */
void drawMenu_ColorFlood() {

    static unsigned long lastUpdate = 0;    // stores when the last frame was drawn
    static int radius = 0;                  // radius of expanding color flood

    static int cx = random(0, WIDTH);       // center of ripple (x coordinate)
    static int cy = random(0, HEIGHT);      // center of ripple (y coordinate)

    static uint16_t baseHue = 0;            // base color

    // frame rate control [animation fps control]
    // smaller value -> smoother & faster | larger value -> slower and choppy
    if (millis() - lastUpdate < 55) return; // 1000ms / 55ms -> ~18FPS

    lastUpdate = millis();  // updates the current time as the last frame time

    // matrix.clear();  // Commented as i wanted the next color to overlap the current color 

    uint32_t floodColor = matrix.ColorHSV(baseHue, 255, 90);    // Prepares the floor color for the flood

    // Soft Ripple Effect - Slightly diming the current color before new color comes on
    // Loops every pixel and extracts the color and dims the color before the new color is applied   
    for (int i = 0; i < WIDTH * HEIGHT; i++) {  
        uint32_t c = matrix.getPixelColor(i);

        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = c & 0xFF;

        r = (r * 9) / 10;   // leds having 90% of its brightness
        g = (g * 9) / 10;
        b = (b * 9) / 10;

        matrix.setPixelColor(i, r, g, b);
    }

    // Addig new color from random centers
    for (int y = 0; y < HEIGHT; y++) {  // loop over all the 64 leds
        for (int x = 0; x < WIDTH; x++) {

            int dist = abs(x - cx) + abs(y - cy);   // calculating manhattan distance from center 
            // can use euclidean distance formula above to get circular expansion 
            // but since i have just 8x8 i did not choose circular expansion
            
            // radius at the begining will be 0 say origin is 3,3 so in first frame only 3,3 will lit up 
            // Next frame radius is 1 then the 4 pixel on each side of 3,3 will lit up and so on so forth..
            
            if (dist <= radius) {   // only pixels within the current radius are affected in each frame | creating expanding effect
                uint16_t hue = baseHue + dist * 200;    // pixels farther from center get slighter diff hues
                floodColor = matrix.ColorHSV(hue,255,90);   
                matrix.setPixelColor(pixelIndex(y, x), matrix.gamma32(floodColor)); // giving gradient / ripple color look
            }
        }
    }
    radius++;   // expand the radius for next frame

    if (radius > WIDTH + HEIGHT) {      // when ripple is done
        radius = 0;                     // reset the radius
        cx = random(0, WIDTH);          // pick random origin
        cy = random(0, HEIGHT);
        baseHue += 4000;                // gently shifting hue
    }

    matrix.show();
}

/**
 * @brief Menu Preview animation for Falling pixel
 * 
 */
void drawMenu_FallingPixel() {

    static unsigned long lastUpdate = 0;        // stores when the last frame was drawn
    static unsigned long gravityPauseUntil = 0; // stores until what time the animation should pause

    static int x1 = 0;                          // fixed row as the pixel will fall from top
    static int y1 = random(0, 8);               // pixel can fall through random column
    static uint32_t color1 = 0;                 // stores the pixel color

    // frame rate control [animation fps control]
    // smaller value -> smoother & faster | larger value -> slower and choppy
    if (millis() - lastUpdate < 90) return; // 1000ms / 90ms -> ~11FPS

    // Gravity pause handling - adds a lil weight to the motion
    if (millis() < gravityPauseUntil) return;

    lastUpdate = millis();  // updates the current time as the last frame time

    // Trail logic
    for (int i = 0; i < 64; i++) {  // loop through all 64 leds each led fades a lil every frame
        uint32_t c = matrix.getPixelColor(i);   // reads the color from pixel

        // extracting rgb values from color
        uint8_t r = (c >> 16) & 0xFF;
        uint8_t g = (c >> 8) & 0xFF;
        uint8_t b = c & 0xFF;

        // adding exponential fading to it | Changing the below value will affect the trail brightness  
        r = (r * 6) / 10;   // leds have 60% of brightness and loses 40% of brightness in each frame
        g = (g * 6) / 10;
        b = (b * 6) / 10;

        matrix.setPixelColor(i, r, g, b);  
    }

    matrix.setPixelColor(pixelIndex(x1, y1), matrix.gamma32(color1));
    x1++;   // move pixel one row down

    // Bottom detection & respawn 
    if (x1 >= 8) {          // x1 = 8 means its at the bottom 
        x1 = 0;             // again going back to top row
        y1 = random(0, 8);  // random column chosen

        color1 = matrix.ColorHSV(random(0, 65535), random(180, 255), random(50, 100));  // random color
        gravityPauseUntil = millis() + random(40, 80); // pause animation when it hit the ground
    }

    matrix.show();
}


// ==================== Color flood Interaction ====================

#define MAX_FLOODS 5    // Limits the no: of simultaneous floods to prevent unexpected crashes/resets due to unknown memory access

struct ColorFlood {     // Defines one color flood  
    int cx;             // center of flood coordinates
    int cy;             // center of flood coordinates
    int radius;         // how far the flood expands
    uint16_t baseHue;   // base color of the flood
    bool active;        // is the flood active or not
};

static ColorFlood floods[MAX_FLOODS];   // pool of animation instances
static unsigned long lastUpdate = 0;    // frame rate control for the flood

/**
 * @brief Prepares the matrix for color flood
 * clears all the flood states, clears the matrix 
 * 
 */
void ColorFlood_Init() {
    memset(floods, 0, sizeof(floods));
    matrix.clear();
    matrix.show();
}

/**
 * @brief Spawns a new color flood 
 * 
 */
void ColorFlood_StartNew() {
    for (int i = 0; i < MAX_FLOODS; i++) {          // iterates through all the 5 Flood slots 
        if (!floods[i].active) {                    // uses the Flood active flag to check for empty slots
            floods[i].cx = random(0, WIDTH);        // picks random column
            floods[i].cy = random(0, HEIGHT);       // picks random row
            floods[i].radius = 0;                   // starts flood as single pixel
            floods[i].baseHue = random(65535);      // each flood gets random color
            floods[i].active = true;                // setting the flag high
            break;
        }
    }
}

/**
 * @brief This is a helper function used in color flood interaction to slowly decay the flood after button presses
 * 
 * @param fadeAmount Controls how quicly pixels dim | Higher Value -> Slower Fade and vice versa
 */
void fadeMatrix(uint8_t fadeAmount) {
    for (int i = 0; i < 64; i++) {              // iterates to all the 64 leds
        uint32_t c = matrix.getPixelColor(i);   // gets the color of the pixel

        uint8_t r = (c >> 16) & 0xFF;           // extracting indivual colors using bit masking
        uint8_t g = (c >> 8)  & 0xFF;
        uint8_t b = c & 0xFF;

        /* scale brightness down | acts as fixed point brightness multiplier
           fadeAmount value ranges from 0 - 255 and wth >> 8 the overall gets divided by 256
           so with fadeamount value 255 the multiplier to r,g,b value is 255/256 = 0.996 which is like almost 
           no fade. So with lower value say like 128 the multiplier is 128/256 = 0.5 so directly 
           50% brightness value is lost with each frame.
        */ 
        r = (r * fadeAmount) >> 8;
        g = (g * fadeAmount) >> 8;
        b = (b * fadeAmount) >> 8;

        matrix.setPixelColor(i, r, g, b);
    }
}

/**
 * @brief Animation engine for the flood
 * 
 */
void ColorFlood_Update() {

    // frame rate control - currently set at 1000/55 -> ~18 FPS
    if (millis() - lastUpdate < 55) return;
    lastUpdate = millis();

    // gentle decay so old floods fade
    fadeMatrix(230);   // 230/256 = ~0.90 means with each frame it will lose 10% of it brightness.

    // render all active floods
    for (int i = 0; i < MAX_FLOODS; i++) {
        if (!floods[i].active) continue;    // inactive floods are skipped

        ColorFlood &f = floods[i];  

        for (int y = 0; y < HEIGHT; y++) {                      // iterating over all the leds
            for (int x = 0; x < WIDTH; x++) {

                int dist = abs(x - f.cx) + abs(y - f.cy);       // using manhattan distance formula
                if (dist <= f.radius) {                         // pixels inside the radius will be affected
                    uint16_t hue = f.baseHue + dist * 200;      // pixels further from center will have slightly diff hues
                    uint32_t c = matrix.gamma32(matrix.ColorHSV(hue, 255, 90)); 
                    matrix.setPixelColor(pixelIndex(y, x), c);
                }
            }
        }
        f.radius++; // increasing the radius of flood with each frame

        // when floods covers the matrix
        if (f.radius > WIDTH + HEIGHT) f.active = false;    // reset the flag -> makes the slot free
    }
    matrix.show();
}

 
// ==================== Falling Pixels Interaction ====================

static uint32_t grid[HEIGHT][WIDTH];   // this is settled pixels, stores colors and if the place is occupied or not
static uint8_t columnHeight[WIDTH];    // for each column how many pixels are stacked, is stored in this

#define MAX_FALLING 8   // max simultaneous falling pixels

typedef struct {    // Defines one falling pixel
    bool active;    // if the pixel currently falling
    int x;          // column
    int y;          // row
    uint32_t color; // RGB value
} FallingPixel;

static FallingPixel falling[MAX_FALLING];   // fixed falling limit no dynamic allocation   

static unsigned long lastFall = 0;  // used for frame timing

/**
 * @brief Initializes the Falling Pixel Interaction
 * 
 */
void FallingPixel_Init() {                          // Initilization
    memset(grid, 0, sizeof(grid));                  // clears settled pixels
    memset(columnHeight, 0, sizeof(columnHeight));  // Resets all columns to empty
    memset(falling, 0, sizeof(falling));            // Clears all active falling particles 
    matrix.clear();
    matrix.show();
}

/**
 * @brief This is helper function to check if the column is full with pixels
 * 
 * @param col ,Column Value 0-7 
 * @return true ,if the column is full
 * @return false , if the column is not fully stacked
 */
bool isColumnFull(uint8_t col) {    // this is done to prevent crashes    
    return columnHeight[col] >= HEIGHT; // If column height already as HEIGHT (8) pixels then it cannot accept more
}

/**
 * @brief This functions drops the pixels based on the button press
 * 
 * @param count for short press 1 and long press drops 8-10 
 */
void FallingPixel_Spawn(uint8_t count) {    

    bool colUsed[WIDTH] = { false };   // prevents long press pixels to fall into same column spawns

    for (uint8_t i = 0; i < count; i++) {   // loops to spawn multiple pixels

        int slot = -1;    // represents empty falling slot 
        for (int k = 0; k < MAX_FALLING; k++) {     // Loops through 8 columns to find the inactive column
            if (!falling[k].active) {
                slot = k;                           // slot gets that column value
                break;
            }
        }
        if (slot == -1) return;   // no more slots

        // Temporary list of columns where pixel can spawn
        uint8_t validCols[WIDTH];
        uint8_t validCount = 0;

        for (uint8_t col = 0; col < WIDTH; col++) {     // loops through 8 columns to check which columns are not full or have not been used in this spawn batch
            if (!isColumnFull(col) && !colUsed[col]) {
                validCols[validCount++] = col;
            }
        }

        if (validCount == 0) return;  // no columns available -> stop spawning

        uint8_t col = validCols[random(validCount)];    // picks random column
        colUsed[col] = true;    // marks it as used so next pixel cannot use it

        falling[slot].active = true;    // activate pixel
        falling[slot].x = col;          // start spawning from the column
        falling[slot].y = 0;            // of top row
        falling[slot].color = matrix.gamma32(matrix.ColorHSV(random(65535), 200, 90));  // random colors
    }
}

/**
 * @brief Animation Engine of Falling Pixel Interaction
 * 
 */
void FallingPixel_Update() {

    // this controls the fall speed FPS
    if (millis() - lastFall < 25) return;   // 1000ms / 25ms = 40FPS
    lastFall = millis();

    // ---------- UPDATE PHYSICS ----------
    for (int i = 0; i < MAX_FALLING; i++) {
        if (!falling[i].active) continue;   // processing only the active pixels which is updated spawn function

        int x = falling[i].x;
        int y = falling[i].y;
        int stackTop = HEIGHT - columnHeight[x] - 1;    // calculates where pixel should stop 

        if (y >= stackTop) {    // Pixels have reached the stack 
            if (columnHeight[x] < HEIGHT) {                
                grid[columnHeight[x]][x] = falling[i].color;    // saves pixel into settled grid
                columnHeight[x]++;                              // increase stack height
            }
            falling[i].active = false;  // deactivate falling
        } else {
            falling[i].y++; // continue falling
        }
    }

    // ---------- RENDER ----------
    // matrix.clear();  // This was clearing the matrix with each frame everything was blinking leds falling
    fadeMatrix(150);    // replacing clear with fade 

    // draw settled grid - later need to add something to make this alive
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < columnHeight[x]; y++) {

            uint32_t c = grid[y][x];
            uint8_t r = (c >> 16) & 0xFF;
            uint8_t g = (c >> 8)  & 0xFF;
            uint8_t b = c & 0xFF;

            matrix.setPixelColor(pixelIndex(HEIGHT - 1 - y, x),r, g, b);
        }
    }
    // draw falling pixels
    for (int i = 0; i < MAX_FALLING; i++) {
        if (falling[i].active) {
            matrix.setPixelColor( pixelIndex(falling[i].y, falling[i].x), falling[i].color);
        }
    }
    matrix.show();
}

/**
 * @brief Checks if he columns is full or not
 * 
 * @return true if full
 * @return false if not full
 */
bool FallingPixel_IsFull() {     
    for (int x = 0; x < WIDTH; x++) {
        if (columnHeight[x] < HEIGHT) return false; // if any column is not full means grid not full
    }
    return true;
}

/**
 * @brief This is the Anticipation Part before the beaming out 
 * Can work on this more better
 * @param durationMs How much you want the anticipation part
 */
void FallingPixel_WarningSparkle(uint16_t durationMs) {    
    unsigned long start = millis();
    uint8_t phase = 0;

    while (millis() - start < durationMs) {

        fadeMatrix(180);   // very gentle decay - light fade 

        for (int x = 0; x < WIDTH; x++) {               // traversing all columns
            for (int y = 0; y < columnHeight[x]; y++) { // traversing all rows

                // retrieving colors
                uint32_t c = grid[y][x];
                uint8_t r = (c >> 16) & 0xFF;
                uint8_t g = (c >> 8) & 0xFF;
                uint8_t b = c & 0xFF;

                int8_t sparkle = (Adafruit_NeoPixel::sine8(phase + x*11 + y*17) >> 6) - 2;  // adding bit of shimmer
                // adding a small sine based shimmer per pixel

                r = constrain(r + sparkle, 0, 255);
                g = constrain(g + sparkle, 0, 255);
                b = constrain(b + sparkle, 0, 255);

                matrix.setPixelColor(pixelIndex(HEIGHT - 1 - y, x), r, g, b);
            }
        }
        // Advance animation smoothly
        phase++;
        matrix.show();
        delay(30);
    }
}

/**
 * @brief This is the Decay part Explosion Pixels beaming out
 * 
 */
void FallingPixel_BeamClear() {     // Decay part pixel beaming out

    int delayTime = 120;   // start slow decay initially

    while (!FallingPixel_IsFull() || true) {    // loop until the grid is empty

        int remaining = 0;  // count remaining pixels
        for (int x = 0; x < WIDTH; x++) remaining += columnHeight[x];
        if (remaining == 0) break;  // exit only when the grid is empty

        // pick random non-empty column
        int col;
        do {
            col = random(WIDTH);
        } while (columnHeight[col] == 0);   // This will stop if the column is empty

        int y = columnHeight[col] - 1;  // beam takes pixel one pixel above to the top with each frame
        uint32_t color = grid[y][col];

        // animate beam upward
        for (int by = HEIGHT - 1 - y; by >= 0; by--) {  // taking the beam to the top most row
            fadeMatrix(200);
            matrix.setPixelColor(pixelIndex(by, col), color);
            matrix.show();
            delay(20);
        }

        // remove pixel from grid
        columnHeight[col]--;

        // redraw reamining pixel in the grid
        matrix.clear();
        for (int x = 0; x < WIDTH; x++) {
            for (int yy = 0; yy < columnHeight[x]; yy++) {
                matrix.setPixelColor(pixelIndex(HEIGHT - 1 - yy, x), grid[yy][x]);
            }
        }
        matrix.show();

        delay(delayTime);

        if (delayTime > 20) delayTime -= 5; // with each iteration delayTime reduces and pixel moves fast
    }
}

/**
 * @brief This is the soft closing final fade
 * 
 */
void FallingPixel_FinalFade() {     
    for (int i = 0; i < 8; i++) {   // This does the gradual Global Fade
        fadeMatrix(120);
        matrix.show();
        delay(60);
    }
    FallingPixel_Init();            //Resets the Falling Pixel Interaction State to start again
}

/**
 * @brief This is the High Level Animation End Sequence
 * 
 */
void FallingPixel_Explosion() {
    FallingPixel_WarningSparkle(5000);    // Anticipation Phase
    FallingPixel_BeamClear();             // Destruction Phase
    FallingPixel_FinalFade();             // Closure Phase
}

