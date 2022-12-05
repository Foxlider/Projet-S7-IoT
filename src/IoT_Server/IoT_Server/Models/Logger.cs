namespace IoT_Server.Models
{
    /// <summary>
    /// Enumeration of the different types of 
    /// </summary>
    public enum LogType
    { MAIN, UART, UDP }

    /// <summary>
    /// Logger factory to print out logs to the console (or more)
    /// </summary>
    public static class Logger
    {
        private static Dictionary<LogType, ConsoleColor> colors = new()
        {
            { LogType.MAIN, ConsoleColor.White },
            { LogType.UART, ConsoleColor.Green },
            { LogType.UDP, ConsoleColor.Blue }
        };

        /// <summary>
        /// Print out the log to the Console
        /// </summary>
        /// <param name="type">Where the Log was from</param>
        /// <param name="message">Message to print</param>
        public static void Log(LogType type, string message)
        {
            var lines = message.Split('\n');    //Each line of the message is formatted
            foreach (var line in lines)
            {
                Console.ForegroundColor = colors[type];             //Change color of the console for the LogType
                Console.Write(Enum.GetName(type).PadRight(4));      //Print out the log type on 4 characters
                Console.ResetColor();                               //Reset color of the Console
                Console.WriteLine($"| {DateTime.Now} >> {line}");   //Print out the date and the line of log
            }
        }

        /// <summary>
        /// Print out an error to the Console
        /// </summary>
        /// <param name="type">Where the error was from</param>
        /// <param name="message">Error message</param>
        public static void Error(LogType type, string message)
        {
            var lines = message.Split('\n');
            foreach (var line in lines)
            {
                Console.ForegroundColor = colors[type];         //Change the console color to the logtype
                Console.Write(Enum.GetName(type).PadRight(4));  //Print out the Logtype
                Console.ResetColor();                           //Reset the color prompt
                Console.Write($"| {DateTime.Now} ");            //Print out the date of the log
                Console.ForegroundColor = ConsoleColor.Red;     //Change color to RED for errors
                Console.WriteLine($">> {line}");                //Print out the log line
                Console.ResetColor();                           //Reset the color of the prompt
            }
        }
    }
}
