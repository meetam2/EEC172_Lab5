# EEC 172 RhythmDance
Melanie Tam Brian Chen Group 6

## Project Description
The objective of the final lab was to develop a Rhythm Dance game where a player taps colored arrows at a certain time, which count towards a score after each correct tap timing, leading up to a score. There are stationary arrows on the bottom of the screen and arrows that fall from top to bottom. Once the arrow is overlapping with the stationary arrow,s then you tap but if you don’t tap in time, then you miss the arrow.

## List of Materials
 <ul>
  <li>Texas Instruments CC3200-LAUNCHXL</li>
  <li>Adafruit SSD1351 OLED 16-bit Color 1.5</li>
  <li>TSOP31138 IR Sensor</li>
  <li>AT&T S10-S3 Remote TV code:1010</li>
  <li>x1 100 Ω Resistor</li>
<li>x1 100uF Capacitor</li>
</ul>

## Pin Connections

### SSD1351 OLED Display

| Signal         | Pin          | Header      | Notes                   |
|----------------|--------------|-------------|--------------------------|
| **MOSI (SI)**  | P7           | P2 header   | Serial data input        |
| **SCK (CL)**   | P5           | P1 header   | Serial clock             |
| **DC**         | P62          | P1 header   | Data/Command select      |
| **RESET (R)**  | P59          | P1 header   | Reset                    |
| **OLEDCS (OC)**| P61          | P1 header   | OLED Chip Select         |
| **SDCS (SC)**  | _n.c._       | —           | Not connected            |
| **MISO (SO)**  | _n.c._       | —           | Not connected            |
| **CD**         | _n.c._       | —           | Not connected            |
| **3V**         | _n.c._       | —           | Not connected            |
| **Vin (+)**    | 3.3V         | —           | Power supply             |
| **GND (G)**    | GND          | —           | Ground                   |

### Additional Peripherals

| Signal        | Pin  | Header    | Notes           |
|---------------|------|-----------|------------------|
| **IR SENSOR** | P63  | P3 header |                 |
| **GPIO**      | P8   | P2 header | Game Input Button |

### Internal GPIO Connections

| Button | Pin | GPIO Base   | Pin Mask |
|--------|-----|-------------|----------|
| SW2    | P15 | GPIOA2_BASE | `0x40`   |
| SW3    | P4  | GPIOA1_BASE | `0x20`   |

### I2C Connections

| Signal | Pin |
|--------|-----|
| SCL    | P3  |
| SDA    | P6  |


## Video Demo
[![RhythmDance demo](https://img.youtube.com/vi/VCMMGPpP9-g/0.jpg)](https://www.youtube.com/watch?v=VCMMGPpP9-g)
