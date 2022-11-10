// See https://aka.ms/new-console-template for more information
using System.Net.Sockets;
using System.Net;
using System.Collections;
using System.Text;
using static System.Net.Mime.MediaTypeNames;
using System.IO.Ports;

Console.WriteLine("Booting up...");
string  SERIALPORT = "/dev/ttyUSB0";
int     BAUDRATE = 115200;


UDPSocket s = new UDPSocket();
s.Server(10000);

var ports = SerialPort.GetPortNames();
Console.WriteLine($"UDP | {DateTime.Now} >> AVAILABLE PORTS");
foreach (var port in ports)
{ Console.WriteLine($"UDP | {DateTime.Now} >>\t{port}"); }

SerialPort _serialPort = new SerialPort(ports.Last(), BAUDRATE, Parity.None, 8, StopBits.One);
_serialPort.ReadTimeout = -1;
_serialPort.WriteTimeout = -1;
_serialPort.Handshake = Handshake.None;
_serialPort.DataReceived += _serialPort_DataReceived;


try
{
    _serialPort.Open();
}
catch (Exception)
{
    throw;
}





void _serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
{
    
    string data = _serialPort.ReadLine();
    Console.WriteLine($"UDP | {DateTime.Now} >> {data}");
};

//UDPSocket c = new UDPSocket();
//c.Client("127.0.0.1", 27000);
//c.Send("TEST!");

Console.ReadKey();
s._socket.Close(); //Fixed closing bug (System.ObjectDisposedException)
                   //Bugfix allows to relaunch server
Console.WriteLine("Closed Server \n Press any key to exit");
Console.ReadKey();






















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

    public void Server(int port)
    {
        _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        _socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.ReuseAddress, true);
        _socket.Bind(new IPEndPoint(IPAddress.Any, port));
        Receive();
    }

    public void Client(int port)
    {
        _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        _socket.Connect(IPAddress.Any, port);
        Receive();
    }

    public void Send(string text)
    {
        byte[] data = Encoding.UTF8.GetBytes(text);
        _socket.BeginSend(data, 0, data.Length, SocketFlags.None, (ar) =>
        {
            State so = (State)ar.AsyncState;
            int bytes = _socket.EndSend(ar);
            Console.WriteLine($"UDP | {DateTime.Now} >> SEND: {bytes}, {text}");
        }, state);
    }

    public void Respond(IPEndPoint ep, string text)
    {
        byte[] data = Encoding.UTF8.GetBytes(text);
        using var client = new UdpClient();
        try
        { client.Send(data, data.Length, ep.Address.ToString(), 10000);
            Console.WriteLine($"UDP | {DateTime.Now} >> SENT: {ep.Address}: {text}");
        }
        catch (Exception e)
        { Console.WriteLine(e.ToString()); }
    }

    private void Receive()
    {
        _socket.BeginReceiveFrom(state.buffer, 0, bufSize, SocketFlags.None, ref epFrom, recv = (ar) =>
        {
            try
            {
                State so = (State)ar.AsyncState;
                int bytes = _socket.EndReceiveFrom(ar, ref epFrom);
                _socket.BeginReceiveFrom(so.buffer, 0, bufSize, SocketFlags.None, ref epFrom, recv, so);
                string output = Encoding.UTF8.GetString(so.buffer, 0, bytes);
                Console.WriteLine($"UDP | {DateTime.Now} >> RECV: {((IPEndPoint)epFrom).Address}: {output}");
                
                Respond((IPEndPoint)epFrom, output);
            }
            catch (Exception e) { Console.WriteLine(e.ToString()); }
            
        }, state);
    }
}


