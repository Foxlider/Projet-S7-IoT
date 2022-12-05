# Android Management App

> Install the application, connect to the WIFI of the server, launch the application, and have fun!


> ## __Usage__

> ### __Building__

To load the app on your phone:
 - Plug it in USB on the computer containing the code
 - Make sure you are in developer mode on your phone
 - Launch the application code, choosing your phone as the device. The app should automatically open on the device
 - For other times, the application is directly installed on your phone, just click on the icon on it

> ### __Running__
Usage is quite easy. At first, put the IP and the port to reach in the corresponding fields, and press Edit.

To change the order in which data is displayed on the remote screen, you can either type in a configuration string by hand, or use the data buttons to populate that configuration string. When done, tap Edit.

The data displayed in the upper part of the application are the data received from the server, themselves received from the sensors.

> ## __Development__

> ### __Developpers__

- THOMAS Alexis
    - App display
    - Server connection threads
    - Communication in UDP with the server
    - Management of received data
    - Display order configuration


> ### __Difficulties__

> #### __Data reception__

One of the first problems encountered during development was a problem of receiving data. Indeed, when I asked the server to have the data received, despite my expectation, they did not seem to have been received. After various tests, and by first launching the receive thread and making it loop, we realize that the problem came from the fact that the data had been sent, but that at the time of reception, the receive did not was not yet launched. Once launched, it found itself waiting for data that had already arrived.

> #### __Thread management__
At the very beginning, I wanted to use a single Thread to send configuration data, data requests as well as their reception. They had to be separated into three different threads for better logistics, and to avoid blocking processing (receive, wait). So I decided to create one thread per type of interaction with the server.