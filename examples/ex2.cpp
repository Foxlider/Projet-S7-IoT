#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;
ManagedString UwU("UwU");
ManagedString CP("puceau moi ? serieusement ^^ haha on me l avait pas sortie celle la depuis loooongtemps :)");


int offset = 0;
int tempC = 0;

void onButton(MicroBitEvent)
{	
    if (uBit.buttonA.isPressed())
    { 
        offset -= 1;
    }

    else if (uBit.buttonB.isPressed())
    {
        offset += 1;
    }

    uBit.thermometer.setOffset(offset);
    uBit.sleep(100);

}


void onData(MicroBitEvent)
{
    ManagedString s = uBit.radio.datagram.recv();

    uBit.display.scroll(s, 100);
}


int main() { 

    uBit.init();
    uBit.display.scroll(CP, 1);
    //uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);

    //uBit.radio.enable();
    uBit.radio.setGroup(2);


    // Use the highest output put level on the radio, to increase range and reliability.
    //uBit.radio.setTransmitPower(7);

    // Increase the receive buffer size on our serial port, to be at least the same size as
    // a packet. This guarantees correct parsing of packets.
    //uBit.serial.setRxBufferSize(32);

    // Run a short animaiton at power up.
    // uBit.display.animateAsync(radio, 500, 5, 0, 0);
	
    int t = 0;
    while(1) 
    {
        tempC = uBit.thermometer.getTemperature();
        
        uBit.display.scrollAsync(tempC, 50);

        t++;
        if(t%5==0)
        {
            uBit.radio.enable();
            ManagedString t = tempC;
            uBit.radio.datagram.send(t);
            uBit.display.scroll("|", 25);
            uBit.radio.disable();
            t = 0;
        }
        uBit.sleep(1000);
    }
    
}