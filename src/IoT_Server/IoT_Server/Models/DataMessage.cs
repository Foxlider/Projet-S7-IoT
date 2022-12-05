namespace IoT_Server.Models
{
    /// <summary>
    /// Main DataMessage class for jSON Formatting
    /// </summary>
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
}
