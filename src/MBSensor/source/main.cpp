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
#include "bme280.h"
#include "MicroBit.h"

MicroBitI2C i2c(I2C_SDA0,I2C_SCL0);
MicroBit uBit;
ManagedString order = "";

void onData(MicroBitEvent e)
{
    order = uBit.radio.datagram.recv();
    //uBit.display.scroll(uBit.radio.datagram.recv(), 50);
}


int main()
{


    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);

    uBit.radio.enable();
    uBit.radio.setGroup(8);

    ManagedString temperature;
    ManagedString humidity;
    ManagedString pressure;
    ManagedString uv;
    ManagedString ir;
    ManagedString light;

    ManagedString allValues;
    ManagedString printedValues;

    bme280 bme(&uBit,&i2c);
    uint32_t pression = 0;
    int32_t temp = 0;
    uint16_t humidite = 0;

    while(1){
        allValues = "";
        printedValues = "";
        bme.sensor_read(&pression, &temp, &humidite);
        temperature = (ManagedString)bme.compensate_temperature(temp);
        allValues = allValues + temperature + ";";
        humidity = (ManagedString)bme.compensate_humidity(humidite);
        allValues = allValues + humidity + ";";
        pressure = (ManagedString)(bme.compensate_pressure(pression)/100);
        allValues = allValues + pressure + ";";
        uv = "U";
        allValues = allValues + uv + ";";
        ir = "I";
        allValues = allValues + ir + ";";
        light = "L";
        allValues = allValues + light + ";";

        for (int i = 0; i < order.length(); i++){
            switch (order.charAt(i))
            {
            case 'T':
                printedValues = printedValues + temperature;
                break;
            case 'H':
                printedValues = printedValues + humidity;
                break;
            case 'P':
                printedValues = printedValues + pressure;
                break;
            case 'U':
                printedValues = printedValues + uv;
                break;
            case 'I':
                printedValues = printedValues + ir;
                break;
            case 'L':
                printedValues = printedValues + light;
                break;
            default:
                break;
            }
        }
        uBit.display.scroll(allValues, 50);
        uBit.radio.datagram.send(allValues);
        uBit.sleep(1000);
    }

    uBit.radio.disable();

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}

