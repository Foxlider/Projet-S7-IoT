/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include "MicroBit.h"
#include "ssd1306.h"
#include "bme280.h"
#include "veml6070.h"
#include "tsl256x.h"

MicroBit uBit;
MicroBitI2C i2c(I2C_SDA0,I2C_SCL0);
MicroBitPin P0(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_DIGITAL_OUT);

ManagedString order = "T";

const int KEY_CRYPT = 8;
int KEY_UNCRYPT = (KEY_CRYPT * -1);
const uint8_t NUMBER_OF_LINE_SCREEN = 8;
const uint8_t NUMBER_OF_COL_SCREEN = 16;

ManagedString shift(int key, ManagedString text) {
    ManagedString result = "";
    char ascii;

    for (int i = 0; i<text.length(); i++) {
        ascii = (text.charAt(i) + key - 32 ) % 94 + 32;
        result = result + ascii;
    }
    return result;
}

void onData(MicroBitEvent)
{
    order = shift(KEY_UNCRYPT, uBit.radio.datagram.recv());
}

void displayLine(ssd1306* screen, uint8_t line, uint8_t col, const char* text)
{
    if (line < NUMBER_OF_LINE_SCREEN && line >= 0 && col < NUMBER_OF_COL_SCREEN && col >= 0){
        screen->display_line(line, col, text);
    }
}

void clearScreen(ssd1306* screen){
    for (int i = 0; i < NUMBER_OF_LINE_SCREEN; i++){
        displayLine(screen, i, 0, "                ");
    }
}

int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);

    uBit.radio.enable();
    uBit.radio.setGroup(8);

    char tmp[10];

    ManagedString temperature;
    ManagedString humidity;
    ManagedString pressure;
    ManagedString uv;
    ManagedString ir;
    ManagedString lux;

    ManagedString allValues;

    ssd1306 screen(&uBit, &i2c, &P0);
    bme280 bme(&uBit,&i2c);
    uint32_t presValue = 0;
    int32_t tempValue = 0;
    uint16_t humiValue = 0;
    
    veml6070 veml(&uBit,&i2c);
    uint16_t uvValue = 0;

    tsl256x tsl(&uBit,&i2c);
    uint16_t combValue =0;
    uint16_t irValue = 0;
    uint32_t luxValue = 0;
    
    while(1){
        bme.sensor_read(&presValue, &tempValue, &humiValue);
        sprintf(tmp, "%d.%d", bme.compensate_temperature(tempValue) / 100, bme.compensate_temperature(tempValue) % 100);
        temperature = ManagedString(tmp); 
        sprintf(tmp, "%d", bme.compensate_humidity(humiValue));
        humidity = ManagedString(tmp);
        sprintf(tmp, "%d", bme.compensate_pressure(presValue)/100);
        pressure = ManagedString(tmp);
        
        veml.sensor_read(&uvValue);
        uv = ManagedString((int)uvValue);

        tsl.sensor_read(&combValue, &irValue, &luxValue);
        ir = ManagedString((int)irValue);
        lux = ManagedString((int)luxValue);
        
        allValues = temperature + "|" + humidity + "|" + pressure + "|" + lux + "|" + ir + "|" + uv;
        uBit.radio.datagram.send(allValues);

        int j = 0;
        temperature = temperature + " C";
        humidity = humidity + "";
        pressure = pressure + " hPa";
        uv = uv + "";
        ir = ir + "";
        lux = lux + "";

        clearScreen(&screen);
        for (int i = 0; i < order.length(); i++){
            switch (order.charAt(i))
            {
            case 'T':
                displayLine(&screen, j++, 0, "Temperature: ");
                displayLine(&screen, j++, 0, temperature.toCharArray());
                break;
            case 'H':
                displayLine(&screen, j++, 0, "Humidity: ");
                displayLine(&screen, j++, 0, humidity.toCharArray());
                break;
            case 'P':
                displayLine(&screen, j++, 0, "Pressure: ");
                displayLine(&screen, j++, 0, pressure.toCharArray());
                break;
            case 'U':
                displayLine(&screen, j++, 0, "UV: ");
                displayLine(&screen, j++, 0, uv.toCharArray());
                break;
            case 'I':
                displayLine(&screen, j++, 0, "IR: ");
                displayLine(&screen, j++, 0, ir.toCharArray());
                break;
            case 'L':
                displayLine(&screen, j++, 0, "Luminosity: ");
                displayLine(&screen, j++, 0, lux.toCharArray());
                break;
            default:
                break;
            }
        }
        if (j > NUMBER_OF_LINE_SCREEN){
            clearScreen(&screen);
            displayLine(&screen, 0, 0, "Format too long");
        }
        screen.update_screen();
        uBit.sleep(1000);
    }
    uBit.radio.disable();

    release_fiber();
}

