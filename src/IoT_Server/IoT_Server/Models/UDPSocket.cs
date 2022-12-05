using Newtonsoft.Json;

using System.Net;
using System.Net.Sockets;
using System.Text;

namespace IoT_Server.Models
{
    /// <summary>
    /// Reception and Transmission of fata towards the Controller app through UDP packets
    /// </summary>
    public class UDPSocket
    {
        public Socket _socket;                                      //Current connected socket
        private const int bufSize = 8 * 1024;                       //Buffer size of 8kb
        private State state = new();                                //Current State of the socker connexion
        private EndPoint epFrom = new IPEndPoint(IPAddress.Any, 0); //Current set endpoint
        private AsyncCallback recv = null;                          //UDP callback

        /// <summary>
        /// Socket State containing the buffer of the packet
        /// </summary>
        public class State
        {
            public byte[] buffer = new byte[bufSize];
        }

        /// <summary>
        /// Initialization the UDP server
        /// </summary>
        /// <param name="port">UDP connexion port</param>
        public void Server(int port)
        {
            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);   //Create a new socket
            _socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.ReuseAddress, true);     //Setup the socket

            try
            { _socket.Bind(new IPEndPoint(IPAddress.Any, port)); }  //Open the socket
            catch (Exception ex)
            {
                Logger.Error(LogType.UDP, ex.ToString());   //On error, pront
                return;
            }

            Logger.Log(LogType.UDP, $"CONNECTED TO {IPAddress.Any}:{port}");    //Print out success
            Receive();  //Listen on the socket
        }


        /// <summary>
        /// Close the socket and shutdown the service
        /// </summary>
        public void Close()
        {
            _socket.Shutdown(SocketShutdown.Both);
            _socket.Close();
        }

        /// <summary>
        /// Unused
        /// </summary>
        /// <param name="port"></param>
        public void Client(int port)
        {
            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            _socket.Connect(IPAddress.Any, port);
            Receive();
        }

        /// <summary>
        /// Unused
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
        /// Sending a message to the Controller
        /// </summary>
        /// <param name="ep">Controller Endpoint</param>
        /// <param name="text">Text to send</param>
        public void Respond(IPEndPoint ep, string text)
        {
            byte[] data = Encoding.UTF8.GetBytes(text); //Get the bytes of the message encoded from UTF-8
            using var client = new UdpClient();         //Make a new UdpCLient of the message (will be destroyed after exiting this method)
            try
            {
                client.Send(data, data.Length, ep.Address.ToString(), ep.Port);     //Send the data to the IP and Port of the endpoint
                Logger.Log(LogType.UDP, $"SENT: {ep.Address}:{ep.Port} : {text}");  //PRONT
            }
            catch (Exception e)
            { Logger.Error(LogType.UDP, e.ToString()); }                            //On error, PRONT
        }

        /// <summary>
        /// Reception of data through UDP Packet
        /// </summary>
        private void Receive()
        {
            _socket.BeginReceiveFrom(state.buffer, 0, bufSize, SocketFlags.None, ref epFrom, recv = (ar) => //Set up the buffer code
            {
                try
                {
                    State so = (State)ar.AsyncState;
                    int bytes;
                    try
                    { bytes = _socket.EndReceiveFrom(ar, ref epFrom); } //Receive all the bytes
                    catch (ObjectDisposedException)
                    {
                        Logger.Log(LogType.UDP, "Socket was closed");   //Closed socket should be ignored
                        return;
                    }
                    catch (Exception e)
                    {
                        Logger.Error(LogType.UDP, e.ToString());        //On error, PRONT and quit
                        return;
                    }

                    _socket.BeginReceiveFrom(so.buffer, 0, bufSize, SocketFlags.None, ref epFrom, recv, so); //Start receive the next message


                    string output = Encoding.UTF8.GetString(so.buffer, 0, bytes);                   //Get the string from the UTF-8 encoded bytes
                    Logger.Log(LogType.UDP, $"RECV: {((IPEndPoint)epFrom).Address}: {output}");     //PRONT received text

                    //Damn that's ugly... But... Oh welll...
                    switch (output)
                    {
                        //In case the Controller wants the latest values from the Sensors
                        case string s when s.StartsWith("getValue()"):
                            Logger.Log(LogType.UDP, $"Sending {Globals.LAST_DATA} to Controller App Client");
                            Respond((IPEndPoint)epFrom, Globals.DataToJson());  //Parse the current data to a JSON format
                            break;

                        //In case the Controller sends a command to the sensors
                        case string s when s.Contains("format"):
                            Logger.Log(LogType.UDP, $"Sending Command to micro:bit Gateway");
                            DataMessage msg = JsonConvert.DeserializeObject<DataMessage>(s);    //Convert the JSON string to an object

                            Globals._serialPort.WriteLine(msg.format);  //Write the format to the Serial UART Port
                            break;
                        //Reception of an ACK message from the Controller
                        case string s when s.StartsWith("Données reçues"):
                            Logger.Log(LogType.UDP, $"Controller App Client ACK");
                            break;
                        //Echo pour le debug
                        case string s when s.StartsWith("echo"):
                            Logger.Log(LogType.UDP, $"ECHO");
                            Respond((IPEndPoint)epFrom, output);    //Send back the data that was received
                            break;
                        //Unknown and unhandled UDP packet
                        default:
                            Logger.Log(LogType.UDP, $"UNKNOWN MESSAGE {output}");
                            break;
                    }
                }
                catch (Exception e)
                { Logger.Error(LogType.UDP, e.ToString()); }    //On error, PRONT

            }, state);
        }
    }
}
