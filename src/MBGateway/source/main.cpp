#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;                          
MicroBitSerial serial(USBTX, USBRX);    // Serial Port initialization
const int KEY_CRYPT = 8;                // Encryption key
int KEY_UNCRYPT = (KEY_CRYPT * -1);     // Decryption key

ManagedString SerialRec; // Last message received on port UART USB 

// Stand-by animation
MicroBitImage load("0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0\n0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

// Data sent/received animation
MicroBitImage down("0,0,255,0,0\n0,0,0,255,0\n255,255,255,255,255\n0,0,0,255,0\n0,0,255,0,0\n");

/// @brief ASCII-Shift encoding
/// Each letter in the message is replaced by a character [key] number of position further in the ascii alphabet
/// @param key Encryption key
/// @param text about to be encrypted
/// @return Encrypted text
ManagedString shift(int key, ManagedString text) {
    ManagedString result = "";
    char ascii;

    for (int i = 0; i<text.length(); i++) {
        ascii = (text.charAt(i) + key - 32 ) % 94 + 32;
        result = result + ascii;
    }
    return result;
}

/// @brief Sends data to the sensor module
/// @param message Data to send to the module
void sendToMBSensor(ManagedString message) {
    uBit.display.scrollAsync(message, 40);                  // Scrolls message to send on Microbit Gateway
    ManagedString messageCrypt = shift(KEY_CRYPT, message); // Encrypts message before sending it
    ManagedString toSend(messageCrypt);                     // Store message in a ManagedString
    uBit.radio.datagram.send(toSend);                       // Sends message to sensor module
}

/// @brief Event when a micro:bit button is pressed
/// This function is not needed for the project but comes in handy fairly often when debugging, to avoid going through the whole process
/// @param  
void onButton(MicroBitEvent)
{   
    ManagedString message;
    if (uBit.buttonA.isPressed()) { 
        message = "PT"; // Sensors PRESSURE/TEMPERATURE mode
    }
    if (uBit.buttonB.isPressed()) { 
        message = "TH"; // Sensors TEMPERATURE/HUMIDITY mode
    }
    sendToMBSensor(message);
}


/// @brief Data received from Radio Receiver event 
/// @param  
void onData(MicroBitEvent) {
    ManagedString s = uBit.radio.datagram.recv();           // Reception of radio datagram package
    uBit.display.stopAnimation();                           // Stops current display to show "data received animation"
    uBit.display.rotateTo(MICROBIT_DISPLAY_ROTATION_90);    // Animation goes from pins to usb
    uBit.display.scrollAsync(down, 25 , 1);                 
    ManagedString toSend(s + "\n");                         // Parse string to send
    serial.send(toSend);                                    // Sends string through Serial UART USB
}

/// @brief Data received from Serial UART USB event
/// @param  
void onSerial(MicroBitEvent) {
    SerialRec = serial.readUntil(ManagedString ("\r\n"),ASYNC); // Reads message from UART until \r or \n is received
    uBit.display.stopAnimation();                               // Stops current display to show "data received animation"
    uBit.display.rotateTo(MICROBIT_DISPLAY_ROTATION_270);       // Animation goes from pins to usb
    uBit.display.scrollAsync(down, 25 , 1);
    serial.send(SerialRec+" ACK\n");                            // Sends received message back to Serial + ACK
    sendToMBSensor(SerialRec);                                  // Sends received message to sensor module
}

/// @brief Entry point
/// @return Error code
int main() { 
    uBit.init();                    // Initialize micro:bit services
    serial.baud(115200);            // Initialize UART port with set maximum baudrate
    serial.send("Loading...\n");    // Sends serial message to signal beginning of setup sequence
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);         // Radio data received event
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);               // A button pushed event (debug)
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);               // B button pushed event (debug)
    uBit.messageBus.listen(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH, onSerial);  // UART data received event

    uBit.radio.enable();                            // Enable radio
    uBit.radio.setGroup(8);                         // Connection to group 8 for Radio
    serial.eventOn(ManagedString("\n") , ASYNC);    // Launch MICROBIT_SERIAL_EVT_DELIM_MATCH event whenever \n is read on UART
    serial.read(ASYNC);                             // Allow micro:bit to read on UART port (Otherwise eventOn is never reached)
    serial.send("micro:bit READY\n");               // Sends serial message to signal micro:bit is ready
    while(1) 
    { uBit.display.animate(load, 50 , 5); }         // Idle animation "awaiting message..."
    release_fiber();                                // Closes all threads
}
