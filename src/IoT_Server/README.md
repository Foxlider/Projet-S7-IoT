# Mini-projet for 4IRC students at CPE Lyon

> ## __Usage__


> ### __Installation__

Requirements :   
Make sure you have a working [dotnet 6.0](https://dotnet.microsoft.com/en-us/download/dotnet/6.0) environment on the machine.
Compatible OS : 
- [Windows](https://learn.microsoft.com/en-us/dotnet/core/install/windows?tabs=net60) (recommended) 
- [Raspberry Pi](https://learn.microsoft.com/en-us/dotnet/iot/deployment) (ARM)
- [Linux](https://learn.microsoft.com/en-us/dotnet/core/install/linux-ubuntu) (Debian or Ubuntu) :warning: [Might not work](https://github.com/dotnet/runtime/issues/79196) on some systems :warning:

Navigate to the folder containing the `.csproj` file and run the following commands
```sh
dotnet clean
dotnet restore
dotnet build
```

You can specify the [runtime identifier](https://learn.microsoft.com/en-us/dotnet/core/rid-catalog) for your syste, among other [options](https://learn.microsoft.com/en-us/dotnet/core/rid-catalog)

> ### __Usage__

Run the project with the `dotnet run` command or use the compiled binaries directly from the bin folder.

`dotnet run` has similar options that `dotnet build`

The console should display the output of the different services with different colors : 
- MAIN  (white) is the main thread informations
- UART  (green) is for the Serial logs
- UDP   (blue)  is for the Wi-Fi logs

Errors Logs are shown in red

> ## __Development__

> ### __Developpers__

- LONCHAMBON Alexis
    - Initial Python code
    - Initial despair
    - C# Rework with .NET 6.0
    - UART Communication
    - UDP Communication
    - Basic Logging System

> ### __Issues__

> #### __Python__

The initial Python code had a tendency to burn my CPU by running one of the cores to 100% and making it reach a temperature over 95°C in less time that it took to read that.

More edits only added more complexity to a code that I did not know well enough and more issues. 

I had very little knowledge with Python, UART, UDP and chose to swith to a language that I enjoy.

While I had never used UART or IP Communication in C#, I was eager to learn and set up a server quite quickly, allowing me to help the other devs on their projects to improve compatibility

> #### __System.IO.Ports__

The initial dotnet rework was made on Windows and worked out of the box.  
While testing the program on my own linux system, I hit an exception : `System.PlatformNotSupportedException: System.IO.Ports is currently only supported on Windows.`

After much sadness, I tested it successfully on my Raspberry Pi and on a Debian Virtual Machine. It seems that this error is thrown only on ArchLinux. The issue has been reported on dotnet's GitHub page.
> ## __Improvements__


> ### __Cleanup__
Can that code be improved ?  
YES

This server code look more like a script that an actual server application.  

It can be improved with a Service System (native to dotnet) to better isolate the UART Service and UDP Service.

> ### __Database__
With dotnet, it becomes very easy to design and deploy a database with EntityFramework. It supports natively many Database Providers such as Postgres, MicrosoftSQL, MySQL, SQLite or even a connexion to an Asure Database or an Azure Blob.

The `Logger` class can be easily edited to add each line of logs to the database.
Received Data can also be integrated to the database (`IoTContext.SensorData.Add(data)`).   
Data will be retreived via a Context class like so : 
`IoTContext.SensorData.Last()`

> ### __Website UI__
For a more complete solution, a Blazor website can be made to display realtime logs from each sensor, control the sensors format directly from the web UI or visualise the Android Apps connected to the server's WiFI network.