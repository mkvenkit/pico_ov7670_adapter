/*
    Interface to the OV7670 camera.

    Mahesh Venkitachalam
    electronut.n 
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "pwm.pio.h"
#include "ov7670_qvga_565.pio.h"

#include "ov7670_linux.h"

#define IMAGE_WIDTH  320
#define IMAGE_HEIGHT 240
#define IMAGE_SIZE   (IMAGE_WIDTH * IMAGE_HEIGHT)  // Total bytes

// OV7670 camera pins (Pico 2W)
#define PCLK_PIN   4  // Pixel clock (INPUT)
#define VSYNC_PIN  2  // Frame sync (INPUT)
#define HREF_PIN   3  // Row sync (INPUT)
#define DATA_BASE  6  // GP6-GP13 (8-bit parallel data INPUT)

#define OV7670_I2C_ADDR (0x42 >> 1)  // Use 7-bit address for Pico C SDK

// PIO used 
PIO pio = pio0;
uint sm = 0;

// Init PWM to GP5 - PWM channel 2B
static void init_pwm()
{
    // allocate pin to PWM
    gpio_set_function(5, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 
    uint slice_num = pwm_gpio_to_slice_num(5);

    // 150 MHz / 5 = 30 MHz
    pwm_set_clkdiv(slice_num, 5);           
    // 30 MHz / 2 = 15 MHz
    pwm_set_wrap(slice_num, 1);   
    // 50% Duty cycle
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1);

    // Set the PWM running
    pwm_set_enabled(slice_num, true);
}


// Do an I2C scan on the bus
static void i2c_scan() {

    printf("Scanning I2C...\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        uint8_t data;
        int result = i2c_read_blocking(i2c0, addr, &data, 1, false);
        if (result >= 0) {
            printf("Found device at 0x%02X\n", addr);
        }
    }
    printf("Scan complete.\n");
}

// Function to write a single register to OV7670
static void ov7670_write_reg(i2c_inst_t *i2c, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(i2c, OV7670_I2C_ADDR, data, 2, false);
}

// Send a set of registers 
static void ov7670_config(i2c_inst_t *i2c, const uint8_t* config) {
    int i = 0;
    while (config[i] != 0xFF) {  // Check for end marker
        uint8_t reg = config[i];
        uint8_t val = config[i + 1];
        ov7670_write_reg(i2c, reg, val);
        i += 2;
    }
}

/*
 * Write a list of register settings; ff/ff stops the process.
 */
static void ov7670_write_array(i2c_inst_t *i2c, struct regval_list *vals)
{
	while (vals->reg_num != 0xff || vals->value != 0xff) {
        ov7670_write_reg(i2c, vals->reg_num, vals->value);
		vals++;
	}
}


// Set up the PIO program
void ov7670_pio_init() {
    
    uint offset = pio_add_program(pio, &ov7670_qvga_565_program);
    
    // Configure PIO state machine
    pio_sm_config c = ov7670_qvga_565_program_get_default_config(offset);
    
    // Map pixel data (GP6-GP13) as input
    sm_config_set_in_pins(&c, DATA_BASE);
    
    sm_config_set_in_shift(&c, true, true, 32);  // Auto-Push, shift-right, threshold 32 bits


    // init signal pins - this was needed 
    pio_gpio_init(pio, PCLK_PIN);
    pio_gpio_init(pio, VSYNC_PIN);
    pio_gpio_init(pio, HREF_PIN);

    // Init D7-D0 - not clear if all of his is needed 
    for (int i = 0; i < 8; i++) {
        pio_gpio_init(pio, DATA_BASE + i);
        gpio_set_function(DATA_BASE + i, GPIO_FUNC_PIO0); // required
        gpio_set_pulls(DATA_BASE + i, false, false);
    }
    
    // Set up state machine
    pio_sm_init(pio, sm, offset, &c);

}

int dma_chan;

// Init DMA to transfer image data = 32-bit words from PIO RX FIFO
void dma_init(uint8_t* image_buffer) {
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    
    channel_config_set_write_increment(&c, true);  // Write incrementing in buffer
    channel_config_set_read_increment(&c, false);  // Read from FIFO (fixed address)
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));  // PIO RX request
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);  // Transfer 4 bytes at a time

    //channel_config_set_ring(&c, false, 0);  // No ring buffer

    dma_channel_configure(
        dma_chan,
        &c,
        (uint32_t *)image_buffer,          // Destination buffer
        &pio->rxf[sm],         // Source: PIO RX FIFO
        IMAGE_SIZE / 2,        // Transfer 2*320*240 / 4 (since we're using 32-bit transfers)
        false                  // Don't start immediately
    );
}

#if 0
static int ov7670_apply_fmt(struct v4l2_subdev *sd)
{
	struct ov7670_info *info = to_state(sd);
	struct ov7670_win_size *wsize = info->wsize;
	unsigned char com7, com10 = 0;
	int ret;

	/*
	 * COM7 is a pain in the ass, it doesn't like to be read then
	 * quickly written afterward.  But we have everything we need
	 * to set it absolutely here, as long as the format-specific
	 * register sets list it first.
	 */
	com7 = info->fmt->regs[0].value;
	com7 |= wsize->com7_bit;
	ret = ov7670_write(sd, REG_COM7, com7);
	if (ret)
		return ret;

	/*
	 * Configure the media bus through COM10 register
	 */
	if (info->mbus_config & V4L2_MBUS_VSYNC_ACTIVE_LOW)
		com10 |= COM10_VS_NEG;
	if (info->mbus_config & V4L2_MBUS_HSYNC_ACTIVE_LOW)
		com10 |= COM10_HREF_REV;
	if (info->pclk_hb_disable)
		com10 |= COM10_PCLK_HB;
	ret = ov7670_write(sd, REG_COM10, com10);
	if (ret)
		return ret;

	/*
	 * Now write the rest of the array.  Also store start/stops
	 */
	ret = ov7670_write_array(sd, info->fmt->regs + 1);
	if (ret)
		return ret;

	ret = ov7670_set_hw(sd, wsize->hstart, wsize->hstop, wsize->vstart,
			    wsize->vstop);
	if (ret)
		return ret;

	if (wsize->regs) {
		ret = ov7670_write_array(sd, wsize->regs);
		if (ret)
			return ret;
	}

	/*
	 * If we're running RGB565, we must rewrite clkrc after setting
	 * the other parameters or the image looks poor.  If we're *not*
	 * doing RGB565, we must not rewrite clkrc or the image looks
	 * *really* poor.
	 *
	 * (Update) Now that we retain clkrc state, we should be able
	 * to write it unconditionally, and that will make the frame
	 * rate persistent too.
	 */
	ret = ov7670_write(sd, REG_CLKRC, info->clkrc);
	if (ret)
		return ret;

	return 0;
}
#endif 

// Initialize OV7670 
void ov7670_init(uint8_t* buffer)
{
    // init PWM on XCLK - or module won't start working 
    init_pwm();
 
    // ****************************************
    // Start of Reset/PWR sequence
    // ****************************************

    int pin_pwdn = 14;
    int pin_rst = 15;

    // set PWDN to 0
    gpio_init(pin_pwdn);
    gpio_set_dir(pin_pwdn, GPIO_OUT);
    gpio_put(pin_pwdn, 0);

    // set RESET to 0
    gpio_init(pin_rst);
    gpio_set_dir(pin_rst, GPIO_OUT);
    gpio_put(pin_rst, 0);
    // wait
    sleep_ms(10);
    // set RESET to 1
    gpio_put(pin_rst, 1);
    // wait
    sleep_ms(10);

    // ****************************************
    // END of Reset/PWR sequence
    // ****************************************

    // i2c init
    i2c_init(i2c0, 100 * 1000);

    // I2C scan - for testing 
    //i2c_scan();

    // reset OV7670 using reg 
    ov7670_write_reg(i2c0, 0x12, 0x80);
    // wait 
    sleep_ms(300);

    // set default 
    //ov7670_write_array(i2c0, ov7670_default_regs);

    // send OV7670 config
    ov7670_write_array(i2c0, ds_qvga_yuv_config2);

#if 0 // linux
    // set default 
    ov7670_write_array(i2c0, ov7670_default_regs);

    // COM7 - wsize
    ov7670_write_reg(i2c0, REG_COM7, 0x10);

    // COM10 - COM10_VS_NEG, etc

    // fmt->regs + 1
    ov7670_write_array(i2c0, ov7670_fmt_yuv422 + 1);

    // hstart/stop etc

    /* 
        .hstart		= 168,	
		.hstop		=  24,
		.vstart		=  12,
		.vstop		= 492,
    */
    ov7670_write_reg(i2c0, REG_HSTART, 0x16);	
    ov7670_write_reg(i2c0, REG_HSTOP, 0x04);
    ov7670_write_reg(i2c0, REG_VSTART, 0x02);
    ov7670_write_reg(i2c0, REG_VSTOP, 0x7a);

    // wsize->regs is NULL for QVGA

    // REG_CLKRC
    ov7670_write_reg(i2c0, REG_CLKRC, 0x01);
#endif 

    // init PIO for OV7670 data
    ov7670_pio_init();

    // init DMA
    dma_init(buffer);
}

// Grab a 320x420 frame 
void ov7670_grab_frame()
{
    // start DMA 
    dma_channel_start(dma_chan);

    // enable PIO
    pio_sm_set_enabled(pio, 0, true);

    // put (2*width - 1) into TX FIFO which will push auto-pulled to ISR
    pio_sm_put_blocking(pio, 0, 639);
    
    // wait for DMA to finish 
    dma_channel_wait_for_finish_blocking(dma_chan);

    // disable PIO
    pio_sm_set_enabled(pio, 0, false);

}