# MicroBit IoT Sensor

> ## __Usage__

> ### __Building__

> External Plug

Add a Microbit Breadboard Adapter to the Microbit.
You'll need five cables for the adapter and four cables for the weather sensor (already plug).
Plug every cables and your OLED 128x64p screen on the breadboard accordinng to the following plan.

| Microbit Breadboard Adapter | Weather Sensor | OLED Screen |
|--|--|--|
| 0V | GND | Gnd |
| 3V | +3.3V | Vin |
| P0 | --- | Rst |
| P19 | SCL | Clk |
| P20 | SDA | Data |

> Code install

After making the Pin-Out you can upload the code on Microbit and supply it.

To load the program on the Microbit use the following commands:
 - make (to create the hex file)
 - make install (to install the hex file on the card)

> ### __Running__

The microbit will handle the communication with the Gateway Microbit and print data on the OLED screen.

> Gateway communication

The Gateway communication happens through the radio module of microbit.

| Format | Data |
|--|--|
| T | Temperature |
| H | Humidity |
| P | Pressure |
| U | Ultraviolet |
| I | Infrared |
| L | Lux |

 - Reception: After Gateway sent a format, it is decrypted and save in Microbit memory. Format example: "HTI"
 - Sent: Every seconds, data from every sensors are sent to the Microbit in this format "T|H|P|L|I|U"

> Print

Every seconds, Microbit display data on OLED screen according to it stored format. 
If the format cannot be printed (too much lines to print) a format error message is printed.

> ## __Development__

> ### __Developpers__

- Girardin Mathis
    - Architecture
    - Radio Reception and Transmission
    - Decryption
    - Screen OLED
    - Plugging
    - Sensor reading
    
> ### __Libraries__

The program uses four C++ specific libraries: 
 - bme280: to read Temperature, Humidity and Pressure from the BME280 sensor.
 - tsl256x: to read Lux and Infrared from TSL256X sensor.
 - veml6070: to read Ultraviolet from VEML6070 sensor.
 - ssd1306: to use the OLED screen.
 
 Every libraries uses I2C functionnalities to work.
 
