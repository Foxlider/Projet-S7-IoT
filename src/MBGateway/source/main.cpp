#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;
MicroBitSerial serial(USBTX, USBRX);
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

void onButton(MicroBitEvent e)
{   
    ManagedString message;
    if (uBit.buttonA.isPressed()) { 
        message = "PT";
    }
    if (uBit.buttonB.isPressed()) { 
        message = "TH";
    }
    sendToMBSensor(message);
}

void sendToMBSensor(ManagedString message) {
    uBit.display.scrollAsync(message, 40);
    ManagedString messageCrypt = shift(KEY_CRYPT, message);
    ManagedString toSend(messageCrypt);
    uBit.radio.datagram.send(toSend);
}


void onData(MicroBitEvent e) {
    ManagedString s = uBit.radio.datagram.recv();
    uBit.display.scrollAsync(s, 40);
    ManagedString toSend(s + "\n");
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