
/**********************************************************************************************/
// FOR TESTING & DEBUGGING:
/**********************************************************************************************/
// #define SIKTEC_EPD_DEBUG
#define DECLARE_WITH_SRAM true
#define TEST_WITH_SD_CARD true
#define QC_PRINT_TEXT
#define QC_COLOR_MAP
#define QC_SAND_CIRCLES

/**********************************************************************************************/
// TEST WITH TIS BOARD:
/**********************************************************************************************/
// #define SIKTEC_BOARD_G4
// #define SIKTEC_BOARD_3CU
#define SIKTEC_BOARD_3CS


/**********************************************************************************************/
// LIB INCLUDES:
/**********************************************************************************************/
#include <Arduino.h>
#include <Adafruit_I2CDevice.h>
#include <SIKTEC_EPD.h>
#include <SdFat.h>                // SD card & FAT filesystem library

using namespace SIKtec;

/**********************************************************************************************/
// PIN DEFINITION:
/**********************************************************************************************/
#define EPD_CS      16
#define EPD_DC      4
#if DECLARE_WITH_SRAM
    #define SRAM_CS 17
#else
    #define SRAM_CS -1
#endif
#define EPD_RESET   13 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    15 // can set to -1 to not use a pin (will wait a fixed delay)
#define SD_FAT_TYPE 0 // SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h, 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_CS       14
#define SD_LINE_READER_LEN  128

#define SW_INT  36
#define SW_PUSH 39
#define SW_CCW  34
#define SW_CW   35

/**********************************************************************************************/
// DECLARE OBJECTS:
/**********************************************************************************************/
#ifdef SIKTEC_BOARD_G4
    SIKTEC_EPD_G4 *board;
#endif
#ifdef SIKTEC_BOARD_3CU
    SIKTEC_EPD_3CU *board;
#endif
#ifdef SIKTEC_BOARD_3CS
    SIKTEC_EPD_3CS *board;
#endif

#if TEST_WITH_SD_CARD
    const char USE_SD_FILE[] = "show.txt";
    SdFat sd;
    SdFile myFile;
    bool sdLoaded = false;
    char sdLineBuffer[SD_LINE_READER_LEN];
    size_t sdLines = 0;
    #define SD_SPI_SPEED SD_SCK_MHZ(20) // Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur Safe is 4-8.
    size_t countLines(SdFile&);
#endif 

void draw_text(int16_t, int16_t, uint16_t, const char[]);
void color_map(const int16_t, const int16_t, const int16_t, const int16_t, const int16_t);
void sand_circles(const int16_t, const int16_t, const int16_t);

void setup() {

    //Initialize Serial:
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    delay(3000);

    #if TEST_WITH_SD_CARD
    //Initialize SD Card:
        Serial.println("Loading SD - Card:");
        if (!sd.begin(SD_CS, SD_SPI_SPEED)) {
            Serial.println("SD-Card not intialized !!!");
        } else {
            Serial.println("SD-Card intialized :)");
            if (sd.exists(USE_SD_FILE)) {
                sdLoaded = true;
                sdLines  = countLines(myFile);
                Serial.printf("SD-Card has %d lines to display \n", sdLines);
            } else {
                Serial.println("SD-Card dont have the defined file !!!");
            }
        }
        delay(200);
    #endif

    //Initialize EPD:
    Serial.println("Initialize EPD:");
    #ifdef SIKTEC_BOARD_G4
        board = new SIKTEC_EPD_G4(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
    #endif
    #ifdef SIKTEC_BOARD_3CU
        board = new SIKTEC_EPD_3CU(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
    #endif
    #ifdef SIKTEC_BOARD_3CS
        board = new SIKTEC_EPD_3CS(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);
    #endif

    //Debug some stuff:
    if (board->is_using_sram()) {
        Serial.println("EPD Initialized - USING SRAM");
    } else {
        Serial.println("EPD Initialized - NOT USING SRAM");
    }

    //Read temp:
    // board->begin(EPD_MODE_MONO);
    // board->begin(EPD_MODE_TRICOLOR);
    board->begin(EPD_MODE_GRAYSCALE4);

    delay(1000);
}


void loop() {
    
    Serial.println("IN LOOP >>");
    // while (1) { ; };

    delay(1000);
    
    board->clearBuffer();
    board->setTextSize(2);

    #ifdef QC_PRINT_TEXT
        
        if (board->is_using_sram()) {
            draw_text(5, 5, EPD_BLACK, "SIKTEC EPD QC - SRAM Enabled.");
        } else {
            draw_text(5, 5, EPD_BLACK, "SIKTEC EPD QC - SRAM Disabled.");
        }
    #endif

    #ifdef QC_COLOR_MAP
        color_map(50, 35, 10, 300, 40); 
    #endif

    #ifdef QC_SAND_CIRCLES
        sand_circles(240, 6, 10);
    #endif


    #if TEST_WITH_SD_CARD
        board->setTextSize(2);
        if (sdLoaded) {
            draw_text(5, 90,  EPD_BLACK,  " > Reading SD Card:");
            for (size_t i = 0; i < sdLines; i++) {
                if (readLine(myFile, i + 1)) {
                    Serial.printf("Read line %d ! \n", i + 1);
                    Serial.println(sdLineBuffer);
                    draw_text(40,   115 + (25 * i),  EPD_BLACK,  sdLineBuffer);
                } else {
                    Serial.printf("Cant read line %d !\n", i + 1);
                }
            }
        } else {
            draw_text(5, 90,  EPD_BLACK,  " > SD Card Not detected! ");
        }
        
    #endif 

    board->display(true);

    while (1) { ; };
}

void draw_text(int16_t x, int16_t y, uint16_t color, const char str[]) {
    board->setTextColor(color);
    board->setCursor(x, y);
    board->print(str);
}

void sand_circles(const int16_t y, const int16_t middleRep, const int16_t sideRep) {

    for (uint16_t i = 0; i < middleRep; i++) {
        board->drawCircle(200, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
    for (uint16_t i = 0; i < sideRep; i++) {
        board->drawCircle(140, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
    for (uint16_t i = 0; i < sideRep; i++) {
        board->drawCircle(260, y, i * 5, (i % 2 != 0 ? EPD_BLACK : EPD_RED));
    }
}
void color_map(const int16_t x, const int16_t y, const int16_t gutter, const int16_t width, const int16_t height) {
    int16_t cellW = (width - (gutter * EPD_NUM_COLORS - 1)) / EPD_NUM_COLORS;  
    int16_t cellH = height;
    int16_t move  = cellW + gutter;
    for (uint8_t i = 0; i < EPD_NUM_COLORS; ++i) {
        board->drawRect(
            x + (move * i), 
            y, 
            cellW, 
            cellH, 
            EPD_BLACK
        );
        board->fillRect(
            x + (move * i) + 4, 
            y + 4, 
            cellW - 8, 
            cellH - 8, 
            i
        );
    }
}


#if TEST_WITH_SD_CARD

size_t countLines(SdFile &myFile) {
    size_t lines = 0;
    if (!myFile.open(USE_SD_FILE, FILE_READ)) {
        Serial.println("failed to read file on SD Card!");
    } else {
        while (myFile.available()) {
            int16_t n = myFile.fgets(sdLineBuffer, sizeof(sdLineBuffer));
            lines++;
            if (n <= 0) {
                Serial.println("Error while reading file lines");
            } else {
                //sdLineBuffer[n-1] = '\0';
                Serial.println(sdLineBuffer);
            }
        }
        myFile.close();
    }
    return lines;
}

#endif 


#if TEST_WITH_SD_CARD

bool readLine(SdFile &myFile, size_t line) {
    if (!myFile.open(USE_SD_FILE, FILE_READ)) {
        Serial.println("failed to read file on SD Card!");
        return false;
    }
    size_t lines = 1;
    char seek[2];
    while (myFile.available()) {
        int16_t n = myFile.fgets(sdLineBuffer, sizeof(sdLineBuffer));
        if (lines == line) {
            if (n <= 0) {
                Serial.println("Error while reading file lines 1");
            } else {
                myFile.close();
                if (sdLineBuffer[n-1] == '\n') {
                    sdLineBuffer[n-1] = '\0';
                }
                return true;
            }
        } else {
            lines++;
        }
    }
    myFile.close();
    return false;
}

#endif 
