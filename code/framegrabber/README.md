# Frame Grabber for Pico 2 OV7670 Adapter

## The Plan 

1. [+] Send a PWM to XCLK and check href, vsync, pclk signals.
2. [+] Hook up I2C and set output to 320 x 240 - check signals for correctness
3. Write PIO code.
3. Set up DMA to transfer to buffer.
4. UART code to send buffer.
5. Python code on PC to read from serial and display image data 