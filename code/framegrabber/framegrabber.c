#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"

#include "hardware/regs/io_bank0.h"
#include "hardware/regs/sio.h"
#include "hardware/structs/sio.h"

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

// QVGA image buffer - RGB565 requires 2 bytes per pixel
uint8_t image_buffer[IMAGE_SIZE * 2];

// for button press
volatile bool button_pressed = false;
volatile uint32_t last_press_time = 0;
volatile bool capturing_frame = false;  
#define BUTTON_PIN 26
#define DEBOUNCE_DELAY_MS 50
#define LED_PIN 28

// For testing RGB565 
static void create_test_image(uint8_t* buffer) {
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

// D0-D7 is connected to GP13-GP6 - so need to reverse bits for each byte
static uint8_t reverse_bits(uint8_t byte) {
    byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4);
    byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2);
    byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1);
    return byte;
}

// send image over UART
static void send_image(uart_inst_t* uart, uint8_t* buffer) {
    for (int i = 0; i < IMAGE_SIZE * 2; i++) {  // Send all pixels (each pixel = 2 bytes)
        //uart_putc(uart, buffer[i]);
        uint8_t val = reverse_bits(buffer[i]);
        printf("%c", val);
    }
}

// Interrupt Handler for Button Press
void button_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Ignore if a frame is currently being captured
    if (capturing_frame) return;

    // Debounce check
    if (current_time - last_press_time < DEBOUNCE_DELAY_MS) return;

    last_press_time = current_time;
    button_pressed = true;  // Mark that button was pressed
}

// OV7670 camera pins (Pico 2W)
#define PCLK_PIN   4  // Pixel clock (INPUT)
#define VSYNC_PIN  2  // Frame sync (INPUT)
#define HREF_PIN   3  // Row sync (INPUT)
#define DATA_BASE  6  // GP6-GP13 (8-bit parallel data INPUT)

// Data pins (D0-D7 mapped to GP6-GP13)
#define DATA_PIN_BASE 6  // First data pin (D0 -> GP6)
#define DATA_MASK (0xFF << DATA_PIN_BASE)  // Mask for GPIO6-GPIO13

void grab_frame()
{
    // Initialize control pins as INPUT
    gpio_init(PCLK_PIN);
    gpio_set_dir(PCLK_PIN, GPIO_IN);
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_IN);
    gpio_init(HREF_PIN);
    gpio_set_dir(HREF_PIN, GPIO_IN);

    // Initialize data pins as INPUT
    for (int pin = DATA_BASE; pin < DATA_BASE + 8; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    uint16_t y, x;
    uint8_t *bufPtr = image_buffer;

    // Wait for VSYNC to go HIGH then LOW (Frame start)
    while (!(gpio_get(VSYNC_PIN)));  // Wait for HIGH
    while (gpio_get(VSYNC_PIN));     // Wait for LOW (Frame start)

    y = 240;
    while (y--) {
        x = 640;
        while (x--) {
            // Wait for PCLK to go LOW
            while (gpio_get(PCLK_PIN));

            // Read 8-bit parallel data in one operation
            //uint32_t gpio_value = gpio_get_all();
            //uint8_t byteData = (gpio_value & DATA_MASK) >> DATA_PIN_BASE;

            // Read 8-bit parallel data in one operation using direct register access
            uint8_t byteData = (sio_hw->gpio_in & DATA_MASK) >> DATA_PIN_BASE;


            // Store the byte in the buffer
            *bufPtr++ = byteData;

            // Wait for PCLK to go HIGH
            while (!gpio_get(PCLK_PIN));
        }
    }
}

void capture_frame()
{
    // turn LED on 
    gpio_put(LED_PIN, 0); // on

    // grab frame
    //ov7670_grab_frame();

    grab_frame();

    // send over uart 
    send_image(UART_ID, image_buffer);

    // LED off 
    gpio_put(LED_PIN, 1); // off 
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

    // Init I2C pins
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);
    gpio_pull_up(0);
    gpio_pull_up(1);

    // Init LED pin 
    gpio_init(LED_PIN);       
    gpio_set_dir(LED_PIN, GPIO_OUT);  
    gpio_put(LED_PIN, 1); // off 

    // Configure button pin as input with pull-up
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  // External pull-up exists, but safe to enable

    // Attach interrupt on falling edge (button press)
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // init OV7670
    ov7670_init(image_buffer);

    sleep_ms(1000);
    capture_frame();

    //  main loop 
    while (true) {
        if (button_pressed) {
            button_pressed = false;  // Reset flag
            capturing_frame = true;  // Mark as busy

            capture_frame();

            capturing_frame = false;  // Mark as ready for next press
            
        }
        sleep_ms(100);
    }
}