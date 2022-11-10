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

ManagedString order = "";

const int KEY_CRYPT = 8;
int KEY_UNCRYPT = (KEY_CRYPT * -1);

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
    ManagedString light;

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
        sprintf(tmp, "%d", ((double)bme.compensate_temperature(tempValue))/100);
        temperature = ManagedString(tmp); 
        sprintf(tmp, "%d", bme.compensate_humidity(humiValue));
        humidity = ManagedString(tmp);
        sprintf(tmp, "%d", bme.compensate_pressure(presValue)/100);
        pressure = ManagedString(tmp);
        
        veml.sensor_read(&uvValue);
        
        tsl.sensor_read(&combValue, &irValue, &luxValue);
        
        // allValues = "{\"source\":\"MBS\",\"id\":\"8-A0\",\"data\":{";
        // allValues = allValues + "\"temperature\":" + "" + ",";
        // allValues = allValues + "\"humidity\":" + "" + ",";
        // allValues = allValues + "\"pressure\":" + "" + "";
        // allValues = allValues + "}}";
        uBit.radio.datagram.send("bite");

        int j = 0;
        for (int i = 0; i < order.length(); i++){
            switch (order.charAt(i))
            {
            case 'T':
                temperature = "Temperature: " + temperature + " C";  
                screen.display_line(j++,0,temperature.toCharArray());
                break;
            case 'H':
                humidity = "Humidity: " + humidity;
                screen.display_line(j++,0,humidity.toCharArray());
                break;
            case 'P':
                pressure = "Pressure: " + pressure + " hPa";
                screen.display_line(j++,0,pressure.toCharArray());
                break;
            case 'U':
                break;
            case 'I':
                break;
            case 'L':
                break;
            default:
                break;
            }
        }
        screen.update_screen();
        uBit.sleep(1000);
    }
    uBit.radio.disable();

    release_fiber();
}

