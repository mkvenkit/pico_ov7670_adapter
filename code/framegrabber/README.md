# Frame Grabber for Pico 2 OV7670 Adapter

## The Plan 

1. [+] Send a PWM to XCLK and check href, vsync, pclk signals.
2. [+] Hook up I2C and set output to 320 x 240 - check signals for correctness
3. Write PIO code.
3. Set up DMA to transfer to buffer.
4. UART code to send buffer.
5. Python code on PC to read from serial and display image data 

## Sandeep's PIO Code

```
.program camera_capture
    pull        block      ; Pull OSR value into PIO
    wait 0 pin vsync_pin  ; Wait for VSYNC to go low
    wait 1 pin vsync_pin  ; Wait for VSYNC to go high
    set y, num_border_px-1

border_y_loop:
    wait 1 pin hsync_pin  ; Wait for HSYNC to go high
    wait 0 pin hsync_pin  ; Wait for HSYNC to go low
    jmp  y--, border_y_loop

.wrap_target
    mov x, osr            ; Load OSR into X register
frame_loop:
    wait 1 pin hsync_pin  ; Wait for HSYNC to go high
    set y, num_border_px * num_pclk_per_px - 1

border_x_loop:
    wait 1 pin pclk_pin   ; Wait for PCLK to go high
    wait 0 pin pclk_pin   ; Wait for PCLK to go low
    jmp  y--, border_x_loop

pixel_loop:
    wait 1 pin pclk_pin   ; Wait for PCLK to go high
    in   pins, data_bits  ; Read pixel data
    wait 0 pin pclk_pin   ; Wait for PCLK to go low
    jmp  x--, pixel_loop  ; Loop for pixel read

    wait 0 pin hsync_pin  ; Wait for HSYNC to go low
.wrap
```

## Modified

```
.program camera_capture
    pull        block      ; Pull OSR value into PIO
    wait 0 pin vsync_pin  ; Wait for VSYNC to go low
    wait 1 pin vsync_pin  ; Wait for VSYNC to go high


.wrap_target
    mov x, osr            ; Load OSR into X register
frame_loop:
    wait 1 pin hsync_pin  ; Wait for HSYNC to go high

pixel_loop:
    wait 1 pin pclk_pin   ; Wait for PCLK to go high
    in   pins, data_bits  ; Read pixel data
    wait 0 pin pclk_pin   ; Wait for PCLK to go low
    jmp  x--, pixel_loop  ; Loop for pixel read

    wait 0 pin hsync_pin  ; Wait for HSYNC to go low
.wrap
```