#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;
MicroBitStorage storage;
MicroBitThermometer thermometer(storage);
MicroBitSerial serial(USBTX, USBRX);

int tempC = 0;

void onButton(MicroBitEvent e)
{    
    if (uBit.buttonA.isPressed())
    { 

        uBit.radio.datagram.send("send");
        uBit.display.scroll("send", 50);
    }

    
    uBit.sleep(100);

}


void onData(MicroBitEvent e)
{
    ManagedString s = uBit.radio.datagram.recv();

    serial.send(s);

    uBit.display.scroll("ok", 200);
}


int main() { 

    uBit.init();
    serial.baud(115200);
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);

    uBit.radio.enable();
    uBit.radio.setGroup(2);
    
    while(1) 
    {
        uBit.sleep(100);
    }
}