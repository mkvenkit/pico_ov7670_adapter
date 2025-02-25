/*

    OV7670.h 

    Interface to the OV7670 camera.

    Contains registers and settings.

    Mahesh Venkitachalam
    electronut.n 
*/

#pragma once

//#define USE_ARDUINO_REGS
#ifndef USE_ARDUINO_REGS

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
#define REG_SCALING_PCLK_DELAY 0xA2
#define REG_COM13       0x3D  // Common control 13
#define REG_CLKRC        0x11  // clock 
#define REG_SCALING_XSC 0x70
#define REG_SCALING_YSC 0x71

#define COM7_RGB        0x04  // Selects RGB output format
#define COM7_QVGA       0x10  // Enables QVGA (320x240) resolution
#define COM7_CBAR       0x02  // Enables color bar
#define COM15_RGB565    0x08  // Selects RGB565 output format
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
    REG_COM7, COM7_RGB | COM7_QVGA,          // Select RGB and QVGA mode
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


static const uint8_t minimal_config[] = {
    //REG_COM7, 0x80, // reset 
    REG_COM7, COM7_RGB | COM7_QVGA,          // Select RGB and QVGA mode
    REG_COM3, COM3_DCWEN,                     // Enable downsampling
    REG_COM14, 0x19,                          // Enable downsampling and scaling
    REG_COM10, COM10_PCLK_HREF,               // PCLK toggles on HREF
    REG_COM15, COM15_RGB565 | COM15_R00FF,    // RGB565 with full range
    REG_HSTART, 0x16,                         // Horizontal start
    REG_HSTOP, 0x04,                          // Horizontal stop
    REG_HREF, 0x24,                           // HREF control
    REG_VSTART, 0x02,                         // Vertical start
    REG_VSTOP, 0x7A,                          // Vertical stop
    REG_VREF, 0x0A,                           // Vertical reference
    REG_SCALING_DCWCTR, 0x11,                 // Downsampling by 2
    REG_SCALING_PCLK_DIV, 0xF1,               // DSP scaling
    0xFF, 0xFF  // End marker
};

#define CMATRIX_LEN 6
#define REG_BRIGHT      0x55    /* Brightness */
#define REG_REG76       0x76    /* OV's name */
#define R76_BLKPCOR     0x80    /* Black pixel correction enable */
#define R76_WHTPCOR     0x40    /* White pixel correction enable */
#define R444_ENABLE     0x02    /* Turn on RGB444, overrides 5x5 */
#define R444_RGBX       0x01    /* Empty nibble at end */
#define REG_HAECC1      0x9f    /* Hist AEC/AGC control 1 */
#define REG_HAECC2      0xa0    /* Hist AEC/AGC control 2 */
#define REG_BD50MAX     0xa5    /* 50hz banding step limit */
#define REG_HAECC3      0xa6    /* Hist AEC/AGC control 3 */
#define REG_HAECC4      0xa7    /* Hist AEC/AGC control 4 */
#define REG_HAECC5      0xa8    /* Hist AEC/AGC control 5 */
#define REG_HAECC6      0xa9    /* Hist AEC/AGC control 6 */
#define REG_HAECC7      0xaa    /* Hist AEC/AGC control 7 */
#define REG_BD60MAX     0xab    /* 60hz banding step limit */
#define REG_TSLB	0x3a

#define MTX1            0x4f    /* Matrix Coefficient 1 */
#define MTX2            0x50    /* Matrix Coefficient 2 */
#define MTX3            0x51    /* Matrix Coefficient 3 */
#define MTX4            0x52    /* Matrix Coefficient 4 */
#define MTX5            0x53    /* Matrix Coefficient 5 */
#define MTX6            0x54    /* Matrix Coefficient 6 */
#define REG_CONTRAS     0x56    /* Contrast control */
#define MTXS            0x58    /* Matrix Coefficient Sign */
#define AWBC7           0x59    /* AWB Control 7 */
#define AWBC8           0x5a    /* AWB Control 8 */
#define AWBC9           0x5b    /* AWB Control 9 */
#define AWBC10          0x5c    /* AWB Control 10 */
#define AWBC11          0x5d    /* AWB Control 11 */
#define AWBC12          0x5e    /* AWB Control 12 */
#define REG_GFIX        0x69    /* Fix gain control */
#define GGAIN           0x6a    /* G Channel AWB Gain */
#define DBLV            0x6b    
#define AWBCTR3         0x6c    /* AWB Control 3 */
#define AWBCTR2         0x6d    /* AWB Control 2 */
#define AWBCTR1         0x6e    /* AWB Control 1 */
#define AWBCTR0         0x6f    /* AWB Control 0 */

#define REG_COM16	0x41	/* Control 16 */
#define COM16_AWBGAIN   0x08    /* AWB gain enable */

static const uint8_t ds_qvga_rgb565_config[] = {
    REG_CLKRC, 0x01,
    REG_COM7, COM7_RGB | COM7_QVGA, 
    REG_RGB444, 0x00,                         // Disable RGB444
    REG_COM1, 0x00,                           // No CCIR601
    REG_COM15, COM15_RGB565 | COM15_R00FF,    // RGB565 with full range

    REG_COM3, COM3_DCWEN,                     // Enable downsampling
    
    //REG_SCALING_XSC, 0x3A, // bar 
    //REG_SCALING_YSC, 0xB5,

    0xFF, 0xFF  // End marker
};

static const uint8_t ds_qvga_yuv_config[] = {
    REG_CLKRC, 0x01,
    REG_COM7, 0x10,
#if 0
    REG_COM3, 0x04,
    REG_COM14, 0x19,
    REG_SCALING_XSC, 0x3A, //0x3A,
    REG_SCALING_YSC, 0x35,
    REG_SCALING_DCWCTR, 0x11,
    REG_SCALING_PCLK_DIV, 0xF1,
    REG_SCALING_PCLK_DELAY, 0x02,
#endif 

    REG_TSLB,0x04,				// 0D = UYVY  04 = YUYV	 - REQUIRED!
    REG_COM13,0x00,			   // connect to REG_TSLB

    REG_RGB444, 0x00,                           // Disable RGB444
    REG_COM1, 0x00,

    //0xB0,0x84, //magic register value (without this, colors are not correct)

    // output window - corrects border
#if 0
    REG_HSTART,0x16,
    REG_HSTOP,0x04,
    REG_HREF,0x24,			
    REG_VSTART,0x02,
    REG_VSTOP,0x7a,
    REG_VREF,0x0a,
#endif 

    MTX1,0xb3,
	MTX2,0xb3,
	MTX3,0x00,
	MTX4,0x3d,
	MTX5,0xa7,
	MTX6,0xe4,
	MTXS,0x9e,

#if 0
    // extra
    	//0x70, 0x3a	   // Scaling Xsc
	//0x71, 0x35	   // Scaling Ysc
	//0xA2, 0x02	   // pixel clock delay
	//Color Settings
	//0,0xFF//set gain to maximum possible
	//0xAA,0x14			// Average-based AEC algorithm
	REG_BRIGHT,0x00,	  // 0x00(Brightness 0) - 0x18(Brightness +1) - 0x98(Brightness -1)
	REG_CONTRAS,0x40,	 // 0x40(Contrast 0) - 0x50(Contrast +1) - 0x38(Contrast -1)
	//0xB1,0xB1			// Automatic Black level Calibration
	0xb1,4,//really enable ABLC
	MTX1,0x80,
	MTX2,0x80,
	MTX3,0x00,
	MTX4,0x22,
	MTX5,0x5e,
	MTX6,0x80,
	MTXS,0x9e,
	AWBC7,0x88,
	AWBC8,0x88,
	AWBC9,0x44,
	AWBC10,0x67,
	AWBC11,0x49,
	AWBC12,0x0e,
	REG_GFIX,0x00,
	//GGAIN,0
	AWBCTR3,0x0a,
	AWBCTR2,0x55,
	AWBCTR1,0x11,
	AWBCTR0,0x9f,
	//0xb0,0x84//not sure what this does
	REG_COM16,COM16_AWBGAIN,//disable auto denoise and edge enhancement
	//REG_COM16,0
	0x4C,0,//disable denoise
	0x76,0,//disable denoise
	0x77,0,//disable denoise
	0x7B,4,//brighten up shadows a bit end point 4
	0x7C,8,//brighten up shadows a bit end point 8
	//0x88,238//darken highlights end point 176
	//0x89,211//try to get more highlight detail
	//0x7A,60//slope
	//0x26,0xB4//lower maximum stable operating range for AEC
	//hueSatMatrix(0,100
	//ov7670_store_cmatrix(
	//0x20,12//set ADC range to 1.5x
	REG_COM9,0x6A, //max gain to 128x
	0x74,16,//disable digital gain
	//0x93,15//dummy line MSB
#endif

    0xFF, 0xFF  // End marker
};

#else 

#include "regs.h"

#define rgb565
#define qvga

static const uint8_t arduino_config[] = {
    //set up camera
	0x15,32,//pclk does not toggle on HBLANK COM10
	//0x11,32//Register 0x11 is for pixel clock divider
	REG_RGB444, 0x00,// Disable RGB444
	//REG_COM11,226,//enable night mode 1/8 frame rate COM11*/
	//0x2E,63//Longer delay
	REG_TSLB,0x04,				// 0D = UYVY  04 = YUYV	 
 	REG_COM13,0x88,			   // connect to REG_TSLB
	//REG_COM13,0x8			   // connect to REG_TSLB disable gamma
	#ifdef rgb565
		REG_COM7, 0x04,// | 0x02,		   // RGB + color bar disable 
		REG_COM15, 0xD0,		  // Set rgb565 with Full range	0xD0
	#elif defined rawRGB
		REG_COM7,1//raw rgb bayer
		REG_COM15, 0xC0		  //Full range
	#else
		REG_COM7, 0x00		   // YUV
		//REG_COM17, 0x00		  // color bar disable
		REG_COM15, 0xC0		  //Full range
	#endif
	//REG_COM3, 0x04
	#if defined qqvga || defined qvga
		REG_COM3,4,	// REG_COM3 
	#else
		REG_COM3,0	// REG_COM3
	#endif
	//0x3e,0x00		//  REG_COM14
	//0x72,0x11		//
	//0x73,0xf0		//
	//REG_COM8,0x8F		// AGC AWB AEC Unlimited step size
	/*REG_COM8,0x88//disable AGC disable AEC
	REG_COM1, 3//manual exposure
	0x07, 0xFF//manual exposure
	0x10, 0xFF//manual exposure*/
	#ifdef qqvga
		REG_COM14, 0x1a		  // divide by 4
		0x72, 0x22			   // downsample by 4
		0x73, 0xf2			   // divide by 4
		REG_HSTART,0x16
		REG_HSTOP,0x04
		REG_HREF,0xa4		   
		REG_VSTART,0x02
		REG_VSTOP,0x7a
		REG_VREF,0x0a	
	#endif
	#ifdef qvga
	REG_COM14, 0x19,		 
		0x72, 0x11,	
		0x73, 0xf1,
		REG_HSTART,0x16,
		REG_HSTOP,0x04,
		REG_HREF,0x24,			
		REG_VSTART,0x02,
		REG_VSTOP,0x7a,
		REG_VREF,0x0a,
	#else
		0x32,0xF6		// was B6  
		0x17,0x13		// HSTART
		0x18,0x01		// HSTOP
		0x19,0x02		// VSTART
		0x1a,0x7a		// VSTOP
		//0x03,0x0a		// VREF
	    REG_VREF,0xCA//set 2 high GAIN MSB
	#endif

	//0x70, 0x3a	   // Scaling Xsc
	//0x71, 0x35	   // Scaling Ysc
	//0xA2, 0x02	   // pixel clock delay
	//Color Settings
	//0,0xFF//set gain to maximum possible
	//0xAA,0x14			// Average-based AEC algorithm
	REG_BRIGHT,0x00,	  // 0x00(Brightness 0) - 0x18(Brightness +1) - 0x98(Brightness -1)
	REG_CONTRAS,0x40,	 // 0x40(Contrast 0) - 0x50(Contrast +1) - 0x38(Contrast -1)
	//0xB1,0xB1			// Automatic Black level Calibration
	0xb1,4,//really enable ABLC
	MTX1,0x80,
	MTX2,0x80,
	MTX3,0x00,
	MTX4,0x22,
	MTX5,0x5e,
	MTX6,0x80,
	MTXS,0x9e,
	AWBC7,0x88,
	AWBC8,0x88,
	AWBC9,0x44,
	AWBC10,0x67,
	AWBC11,0x49,
	AWBC12,0x0e,
	REG_GFIX,0x00,
	//GGAIN,0
	AWBCTR3,0x0a,
	AWBCTR2,0x55,
	AWBCTR1,0x11,
	AWBCTR0,0x9f,
	//0xb0,0x84//not sure what this does
	REG_COM16,COM16_AWBGAIN,//disable auto denoise and edge enhancement
	//REG_COM16,0
	0x4C,0,//disable denoise
	0x76,0,//disable denoise
	0x77,0,//disable denoise
	0x7B,4,//brighten up shadows a bit end point 4
	0x7C,8,//brighten up shadows a bit end point 8
	//0x88,238//darken highlights end point 176
	//0x89,211//try to get more highlight detail
	//0x7A,60//slope
	//0x26,0xB4//lower maximum stable operating range for AEC
	//hueSatMatrix(0,100
	//ov7670_store_cmatrix(
	//0x20,12//set ADC range to 1.5x
	REG_COM9,0x6A, //max gain to 128x
	0x74,16,//disable digital gain
	//0x93,15//dummy line MSB

	0x11,4,

    0xFF, 0xFF  // End marker
};

#endif 


void ov7670_init(uint8_t* buffer);
void ov7670_grab_frame();
