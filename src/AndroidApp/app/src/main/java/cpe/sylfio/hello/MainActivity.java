package cpe.sylfio.hello;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import org.json.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class MainActivity extends AppCompatActivity {

    private String IP="192.168.137.1"; // Replace with the IP of your interlocutor
    private int PORT=10000; // Server port
    private InetAddress address; // Java structure describing a resolved address
    private DatagramSocket UDPSocket; // Java framework for network access (UDP)

    private BlockingQueue<String> dataQueue = new ArrayBlockingQueue<String>(100);
    private DemandeThread askThread = new DemandeThread();
    private ReceptionThread receptionThread = new ReceptionThread();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); //Keeps the screen on

        //Definition of the interactive components of the application
        EditText ipText = findViewById(R.id.IPtextEdit);
        EditText portText = findViewById(R.id.portTextEdit);
        EditText showText = findViewById(R.id.affichageTextEdit);
        Button btClear = findViewById(R.id.btnClear);
        Button btModify = findViewById(R.id.modifyButton);

        //Defining the texts to display
        TextView temText = findViewById(R.id.tempText);
        TextView lumText = findViewById(R.id.lumText);
        TextView humText = findViewById(R.id.humText);
        TextView presText = findViewById(R.id.presText);
        TextView uvText = findViewById(R.id.uvText);
        TextView irText = findViewById(R.id.irText);

        //Definition of display choice buttons
        Button btTemp = findViewById(R.id.btnTemp);
        Button btLum = findViewById(R.id.btnLum);
        Button btHum = findViewById(R.id.btnHum);
        Button btPres = findViewById(R.id.btnPres);
        Button btUV = findViewById(R.id.btnuv);
        Button btIR = findViewById(R.id.btninf);
        
        //Default value display
        ipText.setText(IP);
        portText.setText(Integer.toString(PORT));

        //Clear from the config string
        btClear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText("");
            }
        });

        //Added Listener click on send button
        btModify.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                IP = ipText.getText().toString();
                PORT = Integer.parseInt(portText.getText().toString());
                try {address = InetAddress.getByName(IP);} catch (UnknownHostException e) {e.printStackTrace();}
                dataQueue.add(showText.getText().toString());
            }
        });

        //Added view button clicked to config string
        btTemp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText(showText.getText().toString()+"T");
            }
        });
        btLum.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText(showText.getText().toString()+"L");
            }
        });
        btHum.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText(showText.getText().toString()+"H");
            }
        });
        btPres.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText(showText.getText().toString()+"P");
            }
        });
        btUV.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText(showText.getText().toString()+"U");
            }
        });
        btIR.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText(showText.getText().toString()+"I");
            }
        });

        //Starting the configuration sending thread
        EnvoiThread envoiThread = new EnvoiThread(dataQueue);
        envoiThread.start();
    }

    protected void onResume() {
        super.onResume();
        try {
            //Launch threads when opening the application
            UDPSocket = new DatagramSocket();
            address = InetAddress.getByName(IP);
            receptionThread.start();
            askThread.start();
            envoiThread.start()
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    protected void onPause() {
        //Interrupting Threads on Application Close
        super.onPause();
        UDPSocket.close();
        askThread.interrupt();
        receptionThread.interrupt();
        envoiThread.interrupt();
    }

    //Thread sending configuration data to the server
    public class EnvoiThread extends Thread {
        private BlockingQueue<String> queueEnvoi;
        public EnvoiThread(BlockingQueue<String> queue) {
            this.queueEnvoi = queue;
        }
        public void run() {
            while(true){
                try {
                    //Waiting for config data from queue
                    JSONObject obj = new JSONObject();
                    String chaine = queueEnvoi.take();
                    obj.put("format", chaine);

                    //Sending config data
                    byte[] b = obj.toString().getBytes();
                    DatagramPacket packetEnvoi = new DatagramPacket(b,b.length, address, PORT);
                    UDPSocket.send(packetEnvoi);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    //Thread sending data request to server
    public class DemandeThread extends Thread {
        public DemandeThread() {}
        public void run(){
            String chaine = "getValue()";
            byte[] b = chaine.getBytes();
            DatagramPacket packet = new DatagramPacket(b,b.length, address, PORT);
            while(true){
                try {
                    //Send request every 3 seconds
                    UDPSocket.send(packet);
                    Thread.sleep(3000);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    //Thread receiving data
    public class ReceptionThread extends Thread {
        public ReceptionThread() {}
        public void run(){
            byte[] recept;
            DatagramPacket receptPacket;

            //Retrieving Data View Components
            TextView temText = findViewById(R.id.tempText);
            TextView lumText = findViewById(R.id.lumText);
            TextView humText = findViewById(R.id.humText);
            TextView presText = findViewById(R.id.presText);
            TextView uvText = findViewById(R.id.uvText);
            TextView irText = findViewById(R.id.irText);

            String receveidChaine = "";
            while(true){
                try {
                    //Receiving a data packet from the server
                    recept = new byte[1024];
                    receptPacket = new DatagramPacket(recept, recept.length, address, PORT);
                    UDPSocket.receive(receptPacket);

                    //Displaying this data in display components
                    receveidChaine = new String(receptPacket.getData(), java.nio.charset.StandardCharsets.UTF_8);
                    JSONObject json = new JSONObject(receveidChaine);
                    temText.setText("Température : " + json.getJSONObject("data").getString("temperature") + "°C");
                    lumText.setText("Luminosité : " + json.getJSONObject("data").getString("luminosity") + "lm");
                    humText.setText("Humidité : " + json.getJSONObject("data").getString("humidity"));
                    presText.setText("Pression : " + json.getJSONObject("data").getString("pressure") + "hPa");
                    uvText.setText("Ultraviolet : " + json.getJSONObject("data").getString("uv"));
                    irText.setText("Infrarouge : " + json.getJSONObject("data").getString("ir"));

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
}