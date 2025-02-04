/*

    OV7670.h 

    Interface to the OV7670 camera.

    Mahesh Venkitachalam
    electronut.n 
*/

#pragma once

#define REG_COM7        0x12  // Control register 7
#define REG_RGB444      0x8C  // RGB 444 control
#define REG_COM1        0x04  // Common control 1
#define REG_COM15       0x40  // Common control 15
#define REG_COM9        0x14  // Gain ceiling control
#define REG_COM10       0x15  // Common control 10
#define REG_HSTART      0x17  // Horizontal start high bits
#define REG_HSTOP       0x18  // Horizontal stop high bits
#define REG_HREF        0x32  // HREF control
#define REG_VSTART      0x19  // Vertical start high bits
#define REG_VSTOP       0x1A  // Vertical stop high bits
#define REG_VREF        0x03  // Vertical control reference
#define REG_COM3        0x0C  // Common control 3
#define REG_COM14       0x3E  // Common control 14
#define REG_SCALING_DCWCTR 0x72 // Downsampling control
#define REG_SCALING_PCLK_DIV 0x73 // DSP clock scaling
#define REG_COM13       0x3D  // Common control 13

#define REG_SCALING_XSC 0x70
#define REG_SCALING_YSC 0x71

#define COM7_RGB        0x04  // Selects RGB output format
#define COM7_QVGA       0x10  // Enables QVGA (320x240) resolution
#define COM7_CBAR       0x02  // Enables color bar
#define COM15_RGB565    0x10  // Selects RGB565 output format
#define COM15_R00FF     0xC0  // Full 0-255 range for RGB
#define COM10_PCLK_HREF 0x20  // PCLK does not toggle on HREF
#define COM3_DCWEN      0x04  // Enable downsampling
#define COM14_DCWEN     0x10  // Enable downsampling and scaling
#define COM13_GAMMA     0x80  // Enable gamma correction
#define COM13_UVSAT     0x40  // UV auto-adjustment


// For QVGA RGB565
static const uint8_t ov7670_qvga_rgb565[] = {
    REG_COM7, 0x80, // reset 
    REG_COM7, 0x80, // reset 
    REG_COM7, COM7_RGB | COM7_QVGA ,          // Select RGB and QVGA mode
    REG_RGB444, 0x00,                         // Disable RGB444
    REG_COM1, 0x00,                           // No CCIR601
    REG_COM15, COM15_RGB565 | COM15_R00FF,    // RGB565 with full range
    REG_COM9, 0x6A,                           // Set AGC gain ceiling
    REG_COM10, COM10_PCLK_HREF,               // PCLK toggles on HREF
    REG_HSTART, 0x16,                         // Horizontal start
    REG_HSTOP, 0x04,                          // Horizontal stop
    REG_HREF, 0x24,                           // HREF control
    REG_VSTART, 0x02,                         // Vertical start
    REG_VSTOP, 0x7A,                          // Vertical stop
    REG_VREF, 0x0A,                           // Vertical reference
    REG_COM3, COM3_DCWEN,                     // Enable downsampling
    REG_COM14, 0x19,                   // Enable downsampling and scaling
    REG_SCALING_XSC, 0x3a,
    REG_SCALING_YSC, 0x35,
    REG_SCALING_DCWCTR, 0x11,                 // Downsampling by 2
    REG_SCALING_PCLK_DIV, 0xF2,               // DSP scaling
    REG_COM13, COM13_GAMMA | COM13_UVSAT,     // Enable gamma and UV saturation
    0xFF, 0xFF  // End marker
};

// test 
static const uint8_t ov7670_config1[] = {
    REG_COM7, 0x80, // reset 
    REG_COM7, 0x80, // reset 
    REG_COM3, 0x04, 
    REG_COM14, 0x19,
    REG_HSTART, 0x16,
    REG_HSTOP, 0x04,
    REG_HREF, 0x24,
    REG_VSTART, 0x02,
    REG_VSTOP, 0x7a,
    REG_VREF, 0x0a,
    REG_SCALING_DCWCTR, 0x11,
    REG_SCALING_PCLK_DIV, 0xf1,
    0xFF, 0xFF  // End marker
};

// test 
static const uint8_t ov7670_config2[] = {
    REG_COM7, 0x80, // reset 
    REG_COM7, 0x80, // reset 
    //REG_COM3, 0x08,   // Enable scaling
    //REG_COM10, 0x00,  // Ensure no forced PCLK behavior
   
    REG_COM14, 0x19,  // Enable downscaling and PCLK scaling
    REG_SCALING_PCLK_DIV, 0x02,
    0xFF, 0xFF  // End marker
};

// test 
static const uint8_t minimal_config[] = {
    REG_COM7, 0x80, // reset 
    REG_COM7, 0x80, // reset 
    
    REG_COM7, 0x14, // QVGA + RGB
    REG_COM15, COM15_RGB565 | COM15_R00FF,    // RGB565 with full range

    REG_COM10, COM10_PCLK_HREF,               // PCLK toggles on HREF

    REG_COM14, 0x19,  // Enable downscaling and PCLK scaling
    REG_SCALING_PCLK_DIV, 0x02,
    0xFF, 0xFF  // End marker
};

void ov7670_init(uint8_t* buffer);

void ov7670_grab_frame();