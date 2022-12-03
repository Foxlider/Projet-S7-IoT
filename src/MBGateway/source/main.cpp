#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "MicroBit.h"

MicroBit uBit;                          //Bah uBit, quoi...
MicroBitSerial serial(USBTX, USBRX);    //Initialisation du port Serial
const int KEY_CRYPT = 8;                //Clé de chiffrage
int KEY_UNCRYPT = (KEY_CRYPT * -1);     //Clé de déchiffrage

ManagedString SerialRec; //Dernier message recu par le port UART USB 

//Animation de stand-by
MicroBitImage load("0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0\n0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0\n0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");

//Animation de reception/envoi de données
MicroBitImage down("0,0,255,0,0\n0,0,0,255,0\n255,255,255,255,255\n0,0,0,255,0\n0,0,255,0,0\n");

/// @brief Encodage ASCII-Shift
/// @param key Clé d'encodage
/// @param text Texte a encoder
/// @return Texte encodé
ManagedString shift(int key, ManagedString text) {
    ManagedString result = "";
    char ascii;

    for (int i = 0; i<text.length(); i++) {
        ascii = (text.charAt(i) + key - 32 ) % 94 + 32;
        result = result + ascii;
    }
    return result;
}

/// @brief Envoi de données au capteurs micro:bit
/// @param message Message à envoyer
void sendToMBSensor(ManagedString message) {
    uBit.display.scrollAsync(message, 40);                  //Affichage du message envoyé
    ManagedString messageCrypt = shift(KEY_CRYPT, message); //Chiffrage du message
    ManagedString toSend(messageCrypt);                     
    uBit.radio.datagram.send(toSend);                       //Envoi du message par Radio Tranceiver
}

/// @brief Evenement lors de l'appui sur un des deux boutons micro:bit.  
///Il s'agit de debug et failsafe en cas de probleme d'envoi de données aux capteurs
/// @param  
void onButton(MicroBitEvent)
{   
    ManagedString message;
    if (uBit.buttonA.isPressed()) { 
        message = "PT"; //Force l'affichage des capteurs en mode PRESSURE/TEMPERATURE
    }
    if (uBit.buttonB.isPressed()) { 
        message = "TH"; //Force l'affichage des capteurs en mode TEMPERATURE/HUMIDITY
    }
    sendToMBSensor(message);
}


/// @brief Evenement de réception de données via le Radio Receiver 
/// @param  
void onData(MicroBitEvent) {
    ManagedString s = uBit.radio.datagram.recv();           //Reception du packet de données radio
    uBit.display.stopAnimation();                           //Affiche l'animation de reception (pins vers usb)
    uBit.display.rotateTo(MICROBIT_DISPLAY_ROTATION_90);
    uBit.display.scrollAsync(down, 25 , 1);
    // uBit.display.scrollAsync(s, 40);
    ManagedString toSend(s + "\n");                         //Parse le string a envoyer
    serial.send(toSend);                                    //Envoi de données par Serial UART USB
}

/// @brief Evenement de réception de données via la connexion Serial UART USB
/// @param  
void onSerial(MicroBitEvent) {
    SerialRec = serial.readUntil(ManagedString ("\r\n"),ASYNC); //Reception du message par l'UART jusqu'a obtenir un des caractères parmis \r et \n
    uBit.display.stopAnimation();                               //Animation de réception (usb vers pins)
    uBit.display.rotateTo(MICROBIT_DISPLAY_ROTATION_270);
    uBit.display.scrollAsync(down, 25 , 1);
    serial.send(SerialRec+" ACK\n");                            //Renvoi en Serial + ACK
    sendToMBSensor(SerialRec);                                  //Envoi de la commande aux capteurs micro:bit 
}

/// @brief Point d'entrée du programme
/// @return Code d'erreur
int main() { 
    uBit.init();                    //Init des services micro:bit
    serial.baud(115200);            //Init du port UART avec un baudrate maximum
    serial.send("Loading...\n");    //Envoi d'un message part UART
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);         //Event de réception de message radio
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);               //Event d'appui sur le bouton A
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);               //Event d'appui sur le bouton B
    uBit.messageBus.listen(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH, onSerial);  //Event de réception de message UART

    uBit.radio.enable();                            //Activation du module radio
    uBit.radio.setGroup(8);                         //Utilisation du groupe 8 pour la radio
    serial.eventOn(ManagedString("\n") , ASYNC);    //Declenchement d'un event MICROBIT_SERIAL_EVT_DELIM_MATCH lors de la réception d'un caractère \n sur l'UART
    serial.read(ASYNC);                             //Active la lecture sur le port UART (sans cette ligne eventOn n'est jamais déclenché)
    serial.send("micro:bit READY\n");               //La Gateway est enfin prete à l'utilisation
    while(1) 
    { uBit.display.animate(load, 50 , 5); }         //Animation idle indiquant que la Gateway attend un message
    release_fiber();                                //Fermeture de toutes les threads
}