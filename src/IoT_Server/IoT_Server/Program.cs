// See https://aka.ms/new-console-template for more information
using IoT_Server.Models;

using System.IO.Ports;


Logger.Log(LogType.MAIN,$"Booting up...");

//Handling arguments
if (args.Length > 0)
{
    Logger.Log(LogType.MAIN,$"{args.Length} Arguments");
    Logger.Log(LogType.MAIN,$"[{string.Join(" | ", args)}]");
}

//DEFAULT port on /dev/ttyUSB0
string SERIALPORT = "/dev/ttyUSB0";

//DEFAULT port for UDP Socket
int UDP_PORT = 10000;

//DEFAULT baudrate for the UART Connexion towards micro:bit Gateway
int BAUDRATE = 115200;

//STARTING UDP SERVER
UDPSocket s = new();
//Setting up the UDP port
if (args.Contains("-port"))
{
    var i = Array.IndexOf(args, "-port");
    var port = args[i + 1];
    if (int.TryParse(port, out int udpPort))    //If a port was specified and valid
    {
        Logger.Log(LogType.UDP, $"PORT OVERRIDE : {port}");
        UDP_PORT = udpPort; 
    }
    else //Else, use DEFAULT port
    { Logger.Error(LogType.UDP, $"PORT FORMAT WAS WRONG. DEFAULTING TO {UDP_PORT}"); }
}
else  //No port specified, use DEFAULT PORT
{ Logger.Log(LogType.UDP, $"USING DEFAULT PORT {UDP_PORT}"); }
s.Server(UDP_PORT);

//Automatic UART Port detection
List<string> ports;
try
{ ports = SerialPort.GetPortNames().ToList(); } //Get available ports (might crash on some Linux systems)
catch (Exception e)
{
    Logger.Log(LogType.UART, $"\t{e}");
    Logger.Log(LogType.UART, $"Defaulting to {SERIALPORT}");
    ports = new() { SERIALPORT };
}
Logger.Log(LogType.UART,$"AVAILABLE PORTS");
foreach (var port in ports)
{ Logger.Log(LogType.UART, $"\t{port}"); }

if(ports.Count == 0)    //If no port available, quit
{
    Logger.Error(LogType.UART, $"NO UART PORT AVAILABLE.");
    Logger.Log(LogType.MAIN, $"SHUT DOWN");
    s.Close();
}
    

//Automatic UART Port selection
if (args.Contains("-uart"))
{
    var i = Array.IndexOf(args, "-uart");
    var port = args[i+1];
    if (ports.Contains(port))   //If port is selected and available
    { 
        Logger.Log(LogType.UART, $"UART OVERRIDE : {port}");
        SERIALPORT = args[i+1]; 
    }
    else    //Else, use DEFAULT port
    {
        SERIALPORT = ports.Last();
        Logger.Error(LogType.UART, $"{port} UNAVAILABLE. DEFAULTING TO {SERIALPORT}");
    }

}
else  //No port specified, use DEFAULT port
{
    //By general rule, the last UART port is the one connected to the gateway
    Logger.Log(LogType.UART, $"NO UART PORT SPECIFIED, DEFAULTING TO LAST AVAILABLE");
    SERIALPORT = ports.Last();
}
    

//Setting up UART Transmission and Reception Gateway
Logger.Log(LogType.UART, $"OPENING {SERIALPORT} FOR UART COMMUNICATION");
Globals._serialPort = new SerialPort(SERIALPORT, BAUDRATE, Parity.None, 8, StopBits.One)
{
    ReadTimeout = -1,       // |No timeout
    WriteTimeout = -1,      // |
    Handshake = Handshake.None
};
Globals._serialPort.DataReceived += _serialPort_DataReceived; //Event handler for data reception


//UART Port Connexion
try
{ 
    Globals._serialPort.Open(); //Might crash on some linux systems
    Logger.Log(LogType.UART, $"{SERIALPORT} OPENED");
}
catch (Exception e)
{ 
    Logger.Error(LogType.UART, e.ToString());
    throw; 
}




/// <summary>
/// UART Message Reception
/// </summary>
void _serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
{
    string data = Globals._serialPort.ReadLine();   //Read the whole line
    Logger.Log(LogType.UART,$"{data}");             //PRONT
    if(data.Contains('|'))                          //If this is data sent by the Sensors, store it
        Globals.LAST_DATA = data;
};


Logger.Log(LogType.MAIN, $"Press any key to initiate shutdown");
//SHUTDOWN SEQUENCE
Console.ReadKey();
Globals._serialPort.Close();
s.Close(); //Fixed closing bug (System.ObjectDisposedException)
           //Bugfix allows to relaunch server
Logger.Log(LogType.MAIN, $"Closed Server \nPress any key to exit");
Console.ReadKey();