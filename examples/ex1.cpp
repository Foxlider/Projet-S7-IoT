#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;
ManagedString UwU("UwU");
ManagedString CP("puceau moi ? serieusement ^^ haha on me l avait pas sortie celle la depuis loooongtemps :)");


int tempC = 0;

void onButton(MicroBitEvent e)
{	
    if (uBit.buttonA.isPressed())
    { 
        uBit.radio.datagram.send("Hello World");
        uBit.display.scroll("HW", 50);
    }

    else if (uBit.buttonB.isPressed())
    {
        uBit.radio.datagram.send("UwU");
        uBit.display.scroll(UwU, 50);
    }

    
    uBit.sleep(100);

}


void onData(MicroBitEvent e)
{
    ManagedString s = uBit.radio.datagram.recv();

    uBit.display.scroll(s, 100);
}


int main() { 

    uBit.init();
    uBit.display.scroll(CP, 1);
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);

    uBit.radio.enable();
    uBit.radio.setGroup(2);


    // Use the highest output put level on the radio, to increase range and reliability.
    //uBit.radio.setTransmitPower(7);

    // Increase the receive buffer size on our serial port, to be at least the same size as
    // a packet. This guarantees correct parsing of packets.
    //uBit.serial.setRxBufferSize(32);

    // Run a short animaiton at power up.
    // uBit.display.animateAsync(radio, 500, 5, 0, 0);
	

    while(1) 
    {
        uBit.sleep(100);
    }
    
}


