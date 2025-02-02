#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"

#include "OV7670.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#define IMAGE_WIDTH  320
#define IMAGE_HEIGHT 240
#define IMAGE_SIZE   (IMAGE_WIDTH * IMAGE_HEIGHT)  // Total bytes

void create_test_image(uint8_t* buffer) {
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        for (int x = 0; x < IMAGE_WIDTH; x++) {
            uint16_t color;

            if (y < 80) {
                color = 0xF800;  // Red (RGB565)
            } else if (y < 160) {
                color = 0x07E0;  // Green (RGB565)
            } else {
                color = 0x001F;  // Blue (RGB565)
            }

            int index = (y * IMAGE_WIDTH + x) * 2;
            buffer[index] = color & 0xFF;         // Low byte
            buffer[index + 1] = (color >> 8) & 0xFF; // High byte
        }
    }
}

void send_image(uart_inst_t* uart, uint8_t* buffer) {
    for (int i = 0; i < IMAGE_SIZE * 2; i++) {  // Send all pixels (each pixel = 2 bytes)
        //uart_putc(uart, buffer[i]);
        printf("%c", buffer[i]);
    }
}

int main()
{
    stdio_init_all();    
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    //printf("Grabbing frames!\n");

    uint32_t sys_clk = clock_get_hz(clk_sys);    
    //printf("System Clock: %u Hz\n", sys_clk);

    ov7670_init();

    uint8_t image_buffer[IMAGE_SIZE * 2];
    create_test_image(image_buffer);

 
    sleep_ms(2000);
    send_image(UART_ID, image_buffer);


    while (true) {

        //send_image(UART_ID, image_buffer);

        sleep_ms(2000);
    }
}