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

#include "OV7670.h"

#define IMAGE_WIDTH  320
#define IMAGE_HEIGHT 240
#define IMAGE_SIZE   (IMAGE_WIDTH * IMAGE_HEIGHT)  // Total bytes

// OV7670 camera pins (Pico 2W)
#define PCLK_PIN   4  // Pixel clock (INPUT)
#define VSYNC_PIN  2  // Frame sync (INPUT)
#define HREF_PIN   3  // Row sync (INPUT)
#define DATA_BASE  6  // GP6-GP13 (8-bit parallel data INPUT)

#define OV7670_I2C_WADDR 0x42  // OV7670 default I2C address (write)
#define OV7670_I2C_ADDR (0x42 >> 1)  // Use 7-bit address

static void init_pwm_pio(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &pwm_generator_program);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

    pio_sm_config c = pwm_generator_program_get_default_config(offset);
    sm_config_set_clkdiv(&c, 3.125f);  

    sm_config_set_set_pins(&c, pin, 1);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

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
void ov7670_write_reg(i2c_inst_t *i2c, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(i2c, OV7670_I2C_ADDR, data, 2, false);
}

void ov7670_config(i2c_inst_t *i2c, const uint8_t* config) {
    int i = 0;
    while (config[i] != 0xFF) {  // Check for end marker
        uint8_t reg = config[i];
        uint8_t val = config[i + 1];
        ov7670_write_reg(i2c, reg, val);
        i += 2;
    }
}

void ov7670_config_reg(i2c_inst_t *i2c, const struct regval_list reglist[]) {
    uint8_t reg_addr, reg_val;
    const struct regval_list *next = reglist;
    while ((reg_addr != 0xff) | (reg_val != 0xff)) {
        reg_addr = next->reg_num;
        reg_val = next->value;
        ov7670_write_reg(i2c, reg_addr, reg_val);
        next++;
    }
}

// Set up the PIO program
void ov7670_pio_init() {
    PIO pio = pio1;
    uint sm = 0;
    
    uint offset = pio_add_program(pio1, &ov7670_qvga_565_program);
    
    // Configure PIO state machine
    pio_sm_config c = ov7670_qvga_565_program_get_default_config(offset);
    
    // Map pixel data (GP6-GP13) as input
    sm_config_set_in_pins(&c, DATA_BASE);
    
    // Map PCLK, VSYNC, and HREF as inputs
    sm_config_set_jmp_pin(&c, HREF_PIN);  // JMP on HREF for row loop

    sm_config_set_in_shift(&c, true, true, 32);  // Auto-Push, shift-right, threshold 8 bits

    // GP18 test
    pio_sm_set_consecutive_pindirs(pio, sm, 18, 1, true);
    pio_gpio_init(pio1, 18);
    sm_config_set_set_pins(&c, 18, 1);

    // init signal pins
    pio_gpio_init(pio1, PCLK_PIN);
    pio_gpio_init(pio1, VSYNC_PIN);
    pio_gpio_init(pio1, HREF_PIN);

    for (int i = 0; i < 8; i++) {
        pio_gpio_init(pio1, DATA_BASE + i);
        gpio_set_function(DATA_BASE + i, GPIO_FUNC_PIO1);
        gpio_set_pulls(DATA_BASE + i, false, false);
    }
    
    // Set up state machine
    pio_sm_init(pio, sm, offset, &c);

    // enable PIO
    //pio_sm_set_enabled(pio1, 0, true);
}

int dma_chan;

// DMA interrupt handler (optional, can be used for debugging or triggering next frame)
void dma_handler() {
    dma_hw->ints0 = 1u << dma_chan;  // Clear the interrupt
    // Process completed frame here if needed
}

// Init DMA to transfer image data = 32-bit words from PIO RX FIFO
void dma_init(uint8_t* image_buffer) {
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);

    uint8_t px = 0xab;
    
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);  // Transfer 4 bytes at a time
    channel_config_set_read_increment(&c, false);  // Read from FIFO (fixed address)
    channel_config_set_write_increment(&c, true);  // Write incrementing in buffer
    channel_config_set_dreq(&c, pio_get_dreq(pio1, 0, false));  // PIO RX request

    //channel_config_set_ring(&c, false, 0);  // No ring buffer

    dma_channel_configure(
        dma_chan,
        &c,
        image_buffer,          // Destination buffer
        &pio1->rxf[0],         // Source: PIO RX FIFO
        //&px,
        IMAGE_SIZE / 2,        // Transfer 2*320*240 / 4 (since we're using 32-bit transfers)
        false                  // Don't start immediately
    );

    // Enable DMA interrupt (optional)
    //dma_channel_set_irq0_enabled(dma_chan, true);
    //irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    //irq_set_enabled(DMA_IRQ_0, true);
}


void ov7670_init(uint8_t* buffer)
{
    init_pwm_pio(pio0, 0, 5);  // Use PIO0, state machine 0, GP5

    // Reset/PWR sequence

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

#if 0
    // set input pins
    int pin_pclk = 4;
    gpio_init(pin_pclk);
    gpio_set_dir(pin_pclk, GPIO_IN);

    // Configure pin directions
    gpio_set_dir(PCLK_PIN, GPIO_IN);
    gpio_set_dir(VSYNC_PIN, GPIO_IN);
    gpio_set_dir(HREF_PIN, GPIO_IN);
    for (int i = 0; i < 8; i++) {
        gpio_set_dir(DATA_BASE + i, GPIO_IN);
    }
#endif 

    // i2c init
    i2c_init(i2c0, 100 * 1000);

    // scan
    //i2c_scan();

    // reset 
    ov7670_write_reg(i2c0, 0x12, 0x80);
    sleep_ms(100);

    // OV7670 config
    //ov7670_config(i2c0, ov7670_qvga_rgb565);
    //ov7670_config(i2c0, working_config);

    ov7670_config_reg(i2c0, ov7670_default_regs);
    ov7670_write_reg(i2c0, REG_COM10, 32); // PCLK doesn't toggle on HREF
    ov7670_write_reg(i2c0, REG_COM3, 4); // REG_COM3 enable scaling
    ov7670_config_reg(i2c0, qvga_ov7670);
    ov7670_config_reg(i2c0, yuv422_ov7670);
    ov7670_write_reg(i2c0, 0x11, 12); //Earlier it had the value of 10

    // init PIO for OV7670 data
    ov7670_pio_init();

    // init DMA
    dma_init(buffer);
}


void ov7670_grab_frame()
{
    // run DMA 
    dma_channel_start(dma_chan);

    // enable PIO
    pio_sm_set_enabled(pio1, 0, true);

    pio_sm_put_blocking(pio1, 0, 639);
    
    // wait 
    dma_channel_wait_for_finish_blocking(dma_chan);

    // disable PIO
    pio_sm_set_enabled(pio1, 0, false);

}