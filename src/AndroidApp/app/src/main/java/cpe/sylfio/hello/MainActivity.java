package cpe.sylfio.hello;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;

import org.json.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class MainActivity extends AppCompatActivity {

    private String IP="192.168.137.1"; // Remplacer par l'IP de votre interlocuteur
    private int PORT=10000; // Port du serveur
    private InetAddress address; // Structure Java décrivant une adresse résolue
    private DatagramSocket UDPSocket; // Structure Java permettant d'accéder au réseau (UDP)

    private BlockingQueue<String> dataQueue = new ArrayBlockingQueue<String>(100);
    private BlockingQueue<String> askQueue = new ArrayBlockingQueue<String>(100);
    private ReceptionThread receptionThread = new ReceptionThread(askQueue);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); //Permet de garder l'écran allumé

        //Définition des composants de l'application
        EditText ipText = findViewById(R.id.IPtextEdit);
        EditText portText = findViewById(R.id.portTextEdit);
        EditText showText = findViewById(R.id.affichageTextEdit);
        Button btClear = findViewById(R.id.btnClear);
        Button btModify = findViewById(R.id.modifyButton);

        //Définition des boutons de choix d'affichage
        Button btTemp = findViewById(R.id.btnTemp);
        Button btLum = findViewById(R.id.btnLum);
        Button btHum = findViewById(R.id.btnHum);
        Button btPres = findViewById(R.id.btnPres);
        Button btUV = findViewById(R.id.btnuv);
        Button btIR = findViewById(R.id.btninf);

        ipText.setText(IP);
        portText.setText(Integer.toString(PORT));

        btClear.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showText.setText("");
            }
        });

        btModify.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                IP = ipText.getText().toString();
                PORT = Integer.parseInt(portText.getText().toString());
                try {address = InetAddress.getByName(IP);} catch (UnknownHostException e) {e.printStackTrace();}
                dataQueue.add(showText.getText().toString());
                askQueue.add("ask");
            }
        });

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

        //Démarrage du thread
        EnvoiThread envoiThread = new EnvoiThread(dataQueue);
        envoiThread.start();
    }

    protected void onResume() {
        super.onResume();
        try {
            UDPSocket = new DatagramSocket();
            address = InetAddress.getByName(IP);
            receptionThread.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    protected void onPause() {
        super.onPause();
        UDPSocket.close();
        receptionThread.interrupt();
    }

    public class EnvoiThread extends Thread {

        private BlockingQueue<String> queueEnvoi;
        public EnvoiThread(BlockingQueue<String> queue) {
            this.queueEnvoi = queue;
        }
        public void run() {
            while(true){
                try {
                    JSONObject obj = new JSONObject();
                    String chaine = queueEnvoi.take();
                    obj.put("format", chaine);

                    byte[] b = obj.toString().getBytes();
                    DatagramPacket packetEnvoi = new DatagramPacket(b,b.length, address, PORT);
                    UDPSocket.send(packetEnvoi);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public class ReceptionThread extends Thread {

        private BlockingQueue<String> queueRecep;
        public ReceptionThread(BlockingQueue<String> queue) {this.queueRecep = queue;}
        public void run(){
            while(true){
                try {
                    queueRecep.take();
                    String chaine = "getValue()";
                    byte[] b = chaine.getBytes();
                    DatagramPacket packet = new DatagramPacket(b,b.length, address, PORT);
                    UDPSocket.send(packet);

                    byte[] recept = new byte[1024];
                    DatagramPacket receptPacket = new DatagramPacket(recept, recept.length, address, PORT);
                    UDPSocket.receive(receptPacket);

                    chaine = "Données reçues";
                    b = chaine.getBytes();
                    packet = new DatagramPacket(b,b.length, address, PORT);
                    UDPSocket.send(packet);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
}