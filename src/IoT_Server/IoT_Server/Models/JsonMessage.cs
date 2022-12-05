namespace IoT_Server.Models
{
    /// <summary>
    /// Wrapping class for JSON Formatting
    /// </summary>
    class JsonMessage
    {
        public string source { get; set; }
        public string id { get; set; }
        public DataMessage data { get; set; }
    }
}
