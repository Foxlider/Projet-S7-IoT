using Newtonsoft.Json;

using System.Globalization;
using System.IO.Ports;

namespace IoT_Server.Models
{
    /// <summary>
    /// Ugly global class to store global data across the application
    /// </summary>
    public static class Globals
    {
        /// <summary>
        /// Last data sent by the micro:bit sensors (filled with default data for debug purposes)
        /// </summary>
        public static string LAST_DATA = "21.88|3577|983|125|14|0";

        /// <summary>
        /// UART Serial connexion
        /// </summary>
        public static SerialPort _serialPort;

        /// <summary>
        /// Parsing the received data from our format to JSON
        /// </summary>
        /// <returns></returns>
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
            string output = JsonConvert.SerializeObject(obj, Formatting.Indented);
            return output;
        }
    }
}
