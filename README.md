# pico_ov7670_adapter

An adapter PCB for using Pico 2W with an OV7670 camera module.


### OV7670 to Pico 2W Pin Mapping

| **OV7670 Pin** | **Function**            | **Pico 2W Pin** | **Notes**                                                   |
|-----------------|-------------------------|------------------|-------------------------------------------------------------|
| D0             | Data bit 0             | GP0              | Start of data bus                                           |
| D1             | Data bit 1             | GP1              |                                                             |
| D2             | Data bit 2             | GP2              |                                                             |
| D3             | Data bit 3             | GP3              |                                                             |
| D4             | Data bit 4             | GP4              |                                                             |
| D5             | Data bit 5             | GP5              |                                                             |
| D6             | Data bit 6             | GP6              |                                                             |
| D7             | Data bit 7             | GP7              | End of data bus                                             |
| XCLK           | External clock         | GP8 (PWM)        | Can generate XCLK using a PWM signal from the Pico          |
| PCLK           | Pixel clock            | GP9              | Used to capture pixel data                                  |
| VSYNC          | Vertical sync          | GP10             | Signals the start of a new frame                            |
| HREF           | Horizontal reference   | GP11             | Indicates valid row data                                    |
| SDA            | I2C data               | GP12 (I2C0 SDA)  | For configuring the OV7670 via I2C                         |
| SCL            | I2C clock              | GP13 (I2C0 SCL)  | For configuring the OV7670 via I2C                         |
| RESET          | Reset                  | GP14             | Optional, can be tied to a GPIO for software reset         |
| PWDN           | Power down             | GP15             | Optional, can be tied to a GPIO or GND for always-on mode  |

