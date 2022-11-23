// See https://aka.ms/new-console-template for more information
using System.Globalization;
using System.IO.Ports;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks.Dataflow;
using System.Xml;

using Newtonsoft.Json;


Logger.Log(LogType.MAIN,$"Booting up...");
if (args.Length > 0)
{
    Logger.Log(LogType.MAIN,$"{args.Length} Arguments");
    Logger.Log(LogType.MAIN,$"[{string.Join(" | ", args)}]");
}

//DEFAULT port sur /dev/ttyUSB0
string SERIALPORT = "/dev/ttyUSB0";

//DEFAULT port pour UDP
int UDP_PORT = 10000;

//DEFAULT baudrate pour le micro:bit Gateway
int BAUDRATE = 115200;

//STARTING UDP SERVER
UDPSocket s = new UDPSocket();
//Setup du port UDP
if (args.Contains("-port"))
{
    var i = Array.IndexOf(args, "-port");
    var port = args[i + 1];
    if (int.TryParse(port, out int udpPort))
    {
        Logger.Log(LogType.UDP, $"PORT OVERRIDE : {port}");
        UDP_PORT = udpPort; 
    }
    else
    { Logger.Error(LogType.UDP, $"PORT FORMAT WAS WRONG. DEFAULTING TO {UDP_PORT}"); }

}
else
{ Logger.Log(LogType.UDP, $"USING DEFAULT PORT {UDP_PORT}"); }
s.Server(UDP_PORT);

//Récupération des ports UART
List<string> ports;
try
{ ports = SerialPort.GetPortNames().ToList(); }
catch (Exception e)
{
    Logger.Log(LogType.UART, $"\t{e}");
    Logger.Log(LogType.UART, $"Defaulting to {SERIALPORT}");
    ports = new() { SERIALPORT };
}
Logger.Log(LogType.UART,$"AVAILABLE PORTS");
foreach (var port in ports)
{ Logger.Log(LogType.UART, $"\t{port}"); }

if(ports.Count == 0)
{
    Logger.Error(LogType.UART, $"NO UART PORT AVAILABLE.");
    Logger.Log(LogType.MAIN, $"SHUT DOWN");
    s.Close();
}
    

//Setup du port UART automatiquement
if (args.Contains("-uart"))
{
    var i = Array.IndexOf(args, "-uart");
    var port = args[i+1];
    if (ports.Contains(port))
    { 
        Logger.Log(LogType.UART, $"UART OVERRIDE : {port}");
        SERIALPORT = args[i+1]; 
    }
    else
    {
        SERIALPORT = ports.Last();
        Logger.Error(LogType.UART, $"{port} UNAVAILABLE. DEFAULTING TO {SERIALPORT}");
    }

}
else
{
    //Le dernier port UART est généralement le port sur lequel on a connecté la gateway
    Logger.Log(LogType.UART, $"NO UART PORT SPECIFIED, DEFAULTING TO LAST AVAILABLE");
    SERIALPORT = ports.Last();
}
    

//SETUP de la communication avec la Gateway
Logger.Log(LogType.UART, $"OPENING {SERIALPORT} FOR UART COMMUNICATION");
Globals._serialPort = new SerialPort(SERIALPORT, BAUDRATE, Parity.None, 8, StopBits.One);
Globals._serialPort.ReadTimeout = -1;       // |Pas de timeout
Globals._serialPort.WriteTimeout = -1;      // |
Globals._serialPort.Handshake = Handshake.None;
Globals._serialPort.DataReceived += _serialPort_DataReceived;


//Connexion au port UART
try
{ 
    Globals._serialPort.Open();
    Logger.Log(LogType.UART, $"{SERIALPORT} OPENED");
}
catch (Exception e)
{ 
    Logger.Error(LogType.UART, e.ToString());
    throw; 
}




/// <summary>
/// Reception d'un message par UART
/// </summary>
void _serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
{
    string data = Globals._serialPort.ReadLine();
    Logger.Log(LogType.UART,$"{data}");
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




class JsonMessage
{
    public string source { get; set; }
    public string id { get; set; }
    public DataMessage data { get; set; }
}
class DataMessage
{
    public string format;
    public float temperature;
    public int humidity;
    public int pressure;
    public int luminosity;
    public int uv;
    public int ir;
}

/// <summary>
/// Reception et Transmission de données vers un Contoleur par messages UDP
/// </summary>
public class UDPSocket
{
    public Socket _socket;
    private const int bufSize = 8 * 1024;
    private State state = new State();
    private EndPoint epFrom = new IPEndPoint(IPAddress.Any, 0);
    private AsyncCallback recv = null;

    public class State
    {
        public byte[] buffer = new byte[bufSize];
    }

    /// <summary>
    /// Initialisation du serveur UDP
    /// </summary>
    /// <param name="port">Port de connexion UDP</param>
    public void Server(int port)
    {
        _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        _socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.ReuseAddress, true);

        try
        { _socket.Bind(new IPEndPoint(IPAddress.Any, port)); }
        catch (Exception ex) 
        { 
            Logger.Error(LogType.UDP, ex.ToString());
            return;
        }

        Logger.Log(LogType.UDP, $"CONNECTED TO {IPAddress.Any}:{port}");
        Receive();
    }

    public void Close()
    {
        _socket.Shutdown(SocketShutdown.Both);
        _socket.Close();
    }

    /// <summary>
    /// Pas utilisé actuellement
    /// </summary>
    /// <param name="port"></param>
    public void Client(int port)
    {
        _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        _socket.Connect(IPAddress.Any, port);
        Receive();
    }

    /// <summary>
    /// Pas utilisé actuellement
    /// </summary>
    /// <param name="text"></param>
    public void Send(string text)
    {
        byte[] data = Encoding.UTF8.GetBytes(text);
        _socket.BeginSend(data, 0, data.Length, SocketFlags.None, (ar) =>
        {
            State so = (State)ar.AsyncState;
            int bytes = _socket.EndSend(ar);
            Logger.Log(LogType.UDP, $"SEND: {bytes}, {text}");
        }, state);
    }

    /// <summary>
    /// Envoi de message a un Controleur
    /// </summary>
    /// <param name="ep">Endpoint pour le Controleur</param>
    /// <param name="text">Texte a envoyer</param>
    public void Respond(IPEndPoint ep, string text)
    {
        byte[] data = Encoding.UTF8.GetBytes(text);
        using var client = new UdpClient();
        try
        {
            client.Send(data, data.Length, ep.Address.ToString(), ep.Port);
            Logger.Log(LogType.UDP, $"SENT: {ep.Address}:{ep.Port} : {text}");
        }
        catch (Exception e)
        { Logger.Error(LogType.UDP, e.ToString()); }
    }

    /// <summary>
    /// Reception de données depuis la connexion UDP
    /// </summary>
    private void Receive()
    {
        _socket.BeginReceiveFrom(state.buffer, 0, bufSize, SocketFlags.None, ref epFrom, recv = (ar) =>
        {
            try
            {
                State so = (State)ar.AsyncState;
                int bytes;
                try
                { bytes = _socket.EndReceiveFrom(ar, ref epFrom); }
                catch (ObjectDisposedException)
                { 
                    Logger.Log(LogType.UDP, "Socket was closed");
                    return;
                }
                catch (Exception e)
                {
                    Logger.Error(LogType.UDP, e.ToString());
                    return;
                }

                _socket.BeginReceiveFrom(so.buffer, 0, bufSize, SocketFlags.None, ref epFrom, recv, so);
                string output = Encoding.UTF8.GetString(so.buffer, 0, bytes);
                Logger.Log(LogType.UDP,$"RECV: {((IPEndPoint)epFrom).Address}: {output}");

                //C'est vraiment moche... Mais osef
                switch (output)
                {
                    //Demande du Controleur pour recevoir les valeurs des capteurs
                    case string s when s.StartsWith("getValue()"):
                        Logger.Log(LogType.UDP, $"Sending {Globals.LAST_DATA} to Controller App Client");
                        Respond((IPEndPoint)epFrom, Globals.DataToJson());
                        break;

                    //Reception d'une commande par le Controleur a destination des Capteurs
                    case string s when s.Contains('L'):
                        Logger.Log(LogType.UDP, $"Sending Command to micro:bit Gateway");
                        Globals._serialPort.WriteLine(s);
                        break;
                    //Message ACK du Controleur pour notifier de la reception de données
                    case string s when s.StartsWith("Données reçues"):
                        Logger.Log(LogType.UDP, $"Controller App Client ACK");
                        break;
                    //Echo pour le debug
                    case string s when s.StartsWith("echo"):
                        Logger.Log(LogType.UDP, $"ECHO");
                        Respond((IPEndPoint)epFrom, output);
                        break;
                    default:
                        Logger.Log(LogType.UDP, $"UNKNOWN MESSAGE {output}");
                        break;
                }
            }
            catch (Exception e) 
            { Logger.Error(LogType.UDP, e.ToString()); }

        }, state);
    }
}

/// <summary>
/// Classe globale pour mettre des variables globales bien sale
/// </summary>
public static class Globals
{
    /// <summary>
    /// Dernieres données du capteur micto:bit
    /// </summary>
    public static string LAST_DATA = "21.88|3577|983|125|14|0";

    /// <summary>
    /// Connexion actuelle au port UART
    /// </summary>
    public static SerialPort _serialPort;

    public static string DataToJson()
    {
        var values = LAST_DATA.Split('|');
        
        var data = new DataMessage();
        float.TryParse(values[0], NumberStyles.Any, CultureInfo.InvariantCulture, out data.temperature);
        int.TryParse(values[1], out data.humidity);
        int.TryParse(values[2], out data.pressure);
        int.TryParse(values[3], out data.luminosity);
        int.TryParse(values[4], out data.ir);
        int.TryParse(values[5], out data.uv);
        var obj = new JsonMessage { id = "8-4F", source = "MBG", data = data };
        string output = JsonConvert.SerializeObject(obj, Newtonsoft.Json.Formatting.Indented); ;
        return output;
    }
}


public enum LogType
{ MAIN, UART, UDP }

public static class Logger
{
    private static Dictionary<LogType, ConsoleColor> colors = new() 
    { 
        { LogType.MAIN, ConsoleColor.White },
        { LogType.UART, ConsoleColor.Green }, 
        { LogType.UDP, ConsoleColor.Blue }
    };


    public static void Log(LogType type, string message)
    {
        var lines = message.Split('\n');
        foreach (var line in lines)
        {
            Console.ForegroundColor = colors[type];
            Console.Write(Enum.GetName(type).PadRight(4));
            Console.ResetColor();
            Console.WriteLine($"| {DateTime.Now} >> {line}");
        }
    }

    public static void Error(LogType type, string message)
    {
        var lines = message.Split('\n');
        foreach (var line in lines)
        {
            Console.ForegroundColor = colors[type];
            Console.Write(Enum.GetName(type).PadRight(4));
            Console.ResetColor();
            Console.Write($"| {DateTime.Now} "); 
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($">> {line}");
            Console.ResetColor();
        }
    }
}