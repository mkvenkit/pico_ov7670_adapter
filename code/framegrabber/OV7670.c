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
#include "OV7670.h"

#define OV7670_I2C_ADDR 0x42  // OV7670 default I2C address (write)

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
void ov7670_write_reg(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(i2c_default, OV7670_I2C_ADDR, data, 2, false);
}

void ov7670_init()
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

    // i2c init
    i2c_init(i2c0, 100 * 1000);

    // scan
    i2c_scan();

    // init registers
    int i = 0;
    while (ov7670_qvga_rgb565[i] != 0xFF) {  // Check for end marker
        uint8_t reg = ov7670_qvga_rgb565[i];
        uint8_t val = ov7670_qvga_rgb565[i + 1];
        ov7670_write_reg(reg, val);  // Write register to OV7670
        i += 2;  // Move to next register-value pair
    }
}

void ov7670_grab_frame(uint8_t* buffer)
{

}