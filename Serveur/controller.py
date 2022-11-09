# Program to control passerelle between Android application
# and micro-controller through USB tty
import time
import argparse
import signal
import sys
import socket
import glob
import socketserver
import serial
import threading
from datetime import datetime

HOST           = "0.0.0.0"
UDP_PORT       = 10000
MICRO_COMMANDS = ["TL" , "LT"]
FILENAME        = "values.txt"
LAST_VALUE      = ""

class ThreadedUDPRequestHandler(socketserver.BaseRequestHandler):

    def handle(self):
        data = self.request[0].strip()
        socket = self.request[1]
        current_thread = threading.current_thread()
        print("UDP >> {}: client: {}, wrote: {}".format(current_thread.name, self.client_address, data))
        if data != "":
                        if data in MICRO_COMMANDS: # Send message through UART
                                print(f"UDP >> sending {data} to micro:bit Gateway")
                                sendUARTMessage(data)
                                
                        elif data == "getValues()": # Sent last value received from micro-controller
                                print(f"UDP >> Sending {LAST_VALUE} to COntroller App Client")
                                socket.sendto(LAST_VALUE, self.client_address) 
                                # TODO: Create last_values_received as global variable      
                        else:
                                print("UDP >> Unknown message: ",data)

class ThreadedUDPServer(socketserver.ThreadingMixIn, socketserver.UDPServer):
    pass


# send serial message 
SERIALPORT = "/dev/ttyUSB0"
BAUDRATE = 115200
ser = serial.Serial()

def initUART():        
        # ser = serial.Serial(SERIALPORT, BAUDRATE)
        ser.port=SERIALPORT
        ser.baudrate=BAUDRATE
        ser.bytesize = serial.EIGHTBITS #number of bits per bytes
        ser.parity = serial.PARITY_NONE #set parity check: no parity
        ser.stopbits = serial.STOPBITS_ONE #number of stop bits
        ser.timeout = None          #block read

        # ser.timeout = 0             #non-block read
        # ser.timeout = 2              #timeout block read
        ser.xonxoff = False     #disable software flow control
        ser.rtscts = False     #disable hardware (RTS/CTS) flow control
        ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
        #ser.writeTimeout = 0     #timeout for write
        print('UART>> Starting Up Serial Monitor')
        try:
                ser.open()
        except serial.SerialException:
                print("UART>> Serial {} port not available".format(SERIALPORT))
                exit()

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def sendUARTMessage(msg):
    ser.write(msg)
    print("UART>> Message <" + msg + "> sent to micro-controller." )


# Main program logic follows:
if __name__ == '__main__':
        SerialPorts = serial_ports()
        prevTimestamp = datetime.now()
        timeStamp = datetime.now()
        print(SerialPorts)
        if (len(SerialPorts) == 0):
                print("MAIN>> No serial port available. Closing.")
                exit(0)
        SERIALPORT = SerialPorts[-1]
        initUART()
        f= open(FILENAME,"a")
        print ('MAIN>> Press Ctrl-C to quit.')

        server = ThreadedUDPServer((HOST, UDP_PORT), ThreadedUDPRequestHandler)

        server_thread = threading.Thread(target=server.serve_forever)
        server_thread.daemon = True

        try:
                server_thread.start()
                print("MAIN>> Server started at {} port {}".format(HOST, UDP_PORT))
                while ser.isOpen() : 
                    data_str = ser.readline().decode("utf-8")
                    f.write(data_str)
                    LAST_VALUE = data_str
                    prevTimestamp = timeStamp
                    timeStamp = datetime.now()
                    timedelta = round((timeStamp-prevTimestamp).total_seconds() * 1000)
                    print(f"SER >> {timeStamp} ({str(timedelta).rjust(5)}ms) >{data_str}", end='')
                        # time.sleep(1) # comment this maybe I dunno
                        # if (ser.inWaiting() > 0): # if incoming bytes are waiting 
                        #         data_str = ser.read(ser.inWaiting()) 
                        #         f.write(data_str)
                        #         LAST_VALUE = data_str
                        #         print(data_str)
        except (KeyboardInterrupt, SystemExit):
                server.shutdown()
                server.server_close()
                f.close()
                ser.close()
                exit()