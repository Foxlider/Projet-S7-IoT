# MicroBit Gateway

> Plug the Gateway Microbit and enjoy.


> ## __Usage__


> ### __Building__

To load the program on the Microbit use the following commands:
 - make (to create the hex file)
 - make install (to install the hex file on the card)

> ### __Running__

Plug-In the micro:bit controller with an USB-B to micro-usb cable to a computer running the ` IoT Server` 

When the Gateway is ready, the display should show a spinning animation showing that it's awaiting data from either the USB UART Connector or the Radio Receiver. A message is also sent to the UART Port to notify the server that the Gateway started.

On receiving a message from the server via the UART Serial Port it will display an arrow going down. The message will be sent back to serial with `ACK` and to the Radio Tranceiver.

```
╭──┸──╮
│  🠟  │
╰─────╯
```


On receiving a message from the Radio Receiver, the Gateway will display an arrow going up. The message will then be sent to the Serial UART Port.
```
╭──┸──╮
│  🠝  │
╰─────╯
```

> ## __Development__

> ### __Developpers__

- LARUE Raphael
    - Architecture
    - Makefile
    - Radio Reception and Transmission
    - Encryption
    - Bases of UART Communication
- LONCHAMBON Alexis
    - UART Reception and Transmission
    - Animations

> ### __Chaotic Trials__

This part of the project was rather difficult. To overcome most obstacles, trial and error was used.
- Programming several modules at the same time made it tricky for testing, hence why a lot of debug methods were used (one still being in final code)
- Encryption was another tricky question, we ended up using a rather simple method of encryption, mostly because the data isn't sensible enough to warrant a stronger level of protection that would make the whole process slower

> #### __UART Hell__
One of the major issues encountered while developing the Gateway was the UART Reception :   
The lack of documentation, examples and outdated libraries made very difficult to find answers to our issues. 
After many tries, the gatewway code was unable to receive any message from the UART Port.
- We considered switching to MicroPython but it did not allowed us to use events.
- We considered using MakeCode but the lack of custom code made it impossible to encrypt data for the Sensors.
- We considered using the recent online development tools such as [Keil Studio](https://www.keil.arm.com/) but micro:bit doesn't support mbed-os 5 or 6
- We considered using the older and retiring [MBed Compiler](http://developer.mbed.org) advised by Lancaster University but due to the fact that the Lancaster Library is outdated since 2018, it was unable to compile successfuly.
- We considered strenthening our minds and go on with C++ by trial and error

After much failure and many headaches, we successfully fixed our UART Reception :  
For a successful UART Connexion you need
- `uBit.init();` to start the services
- `serial.baud(115200);` to set the fixed baudrate
- `uBit.messageBus.listen(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH, onSerial);`  to set up the event
- `serial.eventOn(ManagedString("\n") , ASYNC);` to fire the event on the selected character reception
- `serial.read(ASYNC);` to read from the UART Port (ASYNC to read without blocking the main thread)
- `SerialRec = serial.readUntil(ManagedString ("\r\n"),ASYNC);` to read the data from the UART Rx buffer. 
