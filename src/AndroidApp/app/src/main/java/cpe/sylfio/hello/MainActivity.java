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

    private String IP="192.168.137.1"; // Remplacer par l'IP de votre interlocuteur
    private int PORT=10000; // Port du serveur
    private InetAddress address; // Structure Java décrivant une adresse résolue
    private DatagramSocket UDPSocket; // Structure Java permettant d'accéder au réseau (UDP)

    private BlockingQueue<String> dataQueue = new ArrayBlockingQueue<String>(100);
    private DemandeThread askThread = new DemandeThread();
    private ReceptionThread receptionThread = new ReceptionThread();

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

        //Définition des textes à afficher
        TextView temText = findViewById(R.id.tempText);
        TextView lumText = findViewById(R.id.lumText);
        TextView humText = findViewById(R.id.humText);
        TextView presText = findViewById(R.id.presText);
        TextView uvText = findViewById(R.id.uvText);
        TextView irText = findViewById(R.id.irText);

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
            askThread.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    protected void onPause() {
        super.onPause();
        UDPSocket.close();
        askThread.interrupt();
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

    public class DemandeThread extends Thread {

        public DemandeThread() {}
        public void run(){
            String chaine = "getValue()";
            byte[] b = chaine.getBytes();
            DatagramPacket packet = new DatagramPacket(b,b.length, address, PORT);

            while(true){
                try {
                    UDPSocket.send(packet);
                    Thread.sleep(3000);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public class ReceptionThread extends Thread {

        public ReceptionThread() {}
        public void run(){
            byte[] recept;
            DatagramPacket receptPacket;

            TextView temText = findViewById(R.id.tempText);
            TextView lumText = findViewById(R.id.lumText);
            TextView humText = findViewById(R.id.humText);
            TextView presText = findViewById(R.id.presText);
            TextView uvText = findViewById(R.id.uvText);
            TextView irText = findViewById(R.id.irText);

            String chaine2 = "Données reçues";
            byte[] b = chaine2.getBytes();
            String receveidChaine = "";
            DatagramPacket packet = new DatagramPacket(b,b.length, address, PORT);
            while(true){
                try {
                    recept = new byte[1024];
                    receptPacket = new DatagramPacket(recept, recept.length, address, PORT);
                    UDPSocket.receive(receptPacket);

                    receveidChaine = new String(receptPacket.getData(), java.nio.charset.StandardCharsets.UTF_8);

                    JSONObject json = new JSONObject(receveidChaine);

                    temText.setText("Température : " + json.getJSONObject("data").getString("temperature") + "°C");
                    lumText.setText("Luminosité : " + json.getJSONObject("data").getString("luminosity") + "lm");
                    humText.setText("Humidité : " + json.getJSONObject("data").getString("humidity"));
                    presText.setText("Pression : " + json.getJSONObject("data").getString("pressure") + "hPa");
                    uvText.setText("Ultraviolet : " + json.getJSONObject("data").getString("uv"));
                    irText.setText("Infrarouge : " + json.getJSONObject("data").getString("ir"));

                    UDPSocket.send(receptPacket);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
}