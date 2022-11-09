#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;
MicroBitSerial serial(USBTX, USBRX);

void onButton(MicroBitEvent e)
{    
    if (uBit.buttonA.isPressed()) { 
        uBit.radio.datagram.send("TL");
        uBit.display.scroll("TL", 40);
    }
    if (uBit.buttonB.isPressed()) { 
        uBit.radio.datagram.send("LT");
        uBit.display.scroll("LT", 40);
    }
}


void onData(MicroBitEvent e) {
    ManagedString s = uBit.radio.datagram.recv();
    uBit.display.scrollAsync(s, 40);
    ManagedString toSend = s + "\n";
    serial.send(toSend);
}


int main() { 
    uBit.init();
    serial.baud(115200);
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);

    uBit.radio.enable();
    uBit.radio.setGroup(8);
    
    while(1) 
    {
        uBit.sleep(100);
    }
}