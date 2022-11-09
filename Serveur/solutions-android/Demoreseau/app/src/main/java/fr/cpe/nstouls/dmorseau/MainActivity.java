package fr.cpe.nstouls.dmorseau;

import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class MainActivity extends AppCompatActivity {

    private TextView tvReceived;
    private EditText etToSend;
    private Button btnSend;

    private final int PORT = 10000;
//    private final String IP = "192.168.1.97";
    private final String IP = "192.168.1.5";
    private InetAddress address;
    private DatagramSocket UDPSocket;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvReceived = (TextView) findViewById(R.id.received);
        etToSend = (EditText) findViewById(R.id.testToSend);
        btnSend = (Button) findViewById(R.id.btnSend);
        btnSend.setText("Send to "+IP+":"+PORT);
        btnSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                send(etToSend.getText().toString().getBytes());
            }
        });

        try {
            address = InetAddress.getByName(IP);
            UDPSocket = new DatagramSocket();
        } catch (Exception e) {
            e.printStackTrace();
        }

        // lancement processus réception
        (new ReceiverTask()).execute();
    }

    private void send(final byte[] buffer) {
        (new Thread() {
            public void run() {
                try {
                    DatagramPacket packet= new DatagramPacket(buffer, buffer.length, address, PORT);
                    UDPSocket.send(packet);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }


    private class ReceiverTask extends AsyncTask<Void, byte[], Void> {
        protected Void doInBackground(Void... rien) {
            while(true){
                try {
                    byte[] data = new byte [1024]; // Espace de réception des données.
                    DatagramPacket packet = new DatagramPacket(data, data.length);
                    UDPSocket.receive(packet);
                    int size = packet.getLength();
                    publishProgress(java.util.Arrays.copyOf(data, size));
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        protected void onProgressUpdate(byte[]... data) {
            tvReceived.setText(new String(data[0]));
        }
    }
}
