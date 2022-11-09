package fr.cpe.nstouls.ledsbattle;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity implements SensorEventListener{

    private Button btnPlayer1;
    private Button btnPlayer2;
    private Button startBtn;
    private TextView tvData;
    private TextView playerDisp;
    private TextView resultTV;
    private EditText edIP;
    private EditText edPort;
    private InetAddress address;
    private int port ;

    private DatagramSocket socket=null;
    private DatagramPacket packet=null;

    @Override
    protected void onCreate(Bundle savedInstanceState)  {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnPlayer1 = (Button)   findViewById(R.id.playerOneBtn);
        btnPlayer2 = (Button)   findViewById(R.id.playerTwoBtn);
        tvData     = (TextView) findViewById(R.id.tvData);
        edIP       = (EditText) findViewById(R.id.IP);
        edPort     = (EditText) findViewById(R.id.Port);
        playerDisp = (TextView) findViewById(R.id.dispPlayer);
        startBtn   = (Button)   findViewById(R.id.startBtn);
        resultTV   = (TextView) findViewById(R.id.resultTV);

        btnPlayer1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    msg="(1)".getBytes();
                    playerDisp.setText("Player 1 !!! Les meilleurs");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        btnPlayer2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    msg="(2)".getBytes();
                    playerDisp.setText("Player 2 !!! C'est bien quand même O:-)");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

        startBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                MainActivity.this.start();
            }
        });


        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    public void start() {
        if (msg == null) {
            Toast.makeText(MainActivity.this,"Veuillez choisir le player d'abord",Toast.LENGTH_LONG).show();
        } else {
            try {
                address = InetAddress.getByName(edIP.getText().toString());
                port = Integer.parseInt(edPort.getText().toString());
                socket = new DatagramSocket();
                btnPlayer1.setEnabled(false);
                btnPlayer2.setEnabled(false);
                startBtn.setEnabled(false);
                edPort.setEnabled(false);
                edIP.setEnabled(false);

                (new ResultListener()).execute();

                (new Thread() {
                    @Override
                    public void run() {
                        byte[] buffer = "(0)".getBytes(); // Reset du serveur
                        packet = new DatagramPacket(buffer, buffer.length, address, port);
                        try {
                            socket.send(packet);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }).start();
            } catch(UnknownHostException e){
                e.printStackTrace();
            } catch(SocketException e){
                e.printStackTrace();
            } catch(Exception e){
                e.printStackTrace();
            }
        }
    }

    class ResultListener extends AsyncTask<Void,String,Void> {
        @Override
        protected Void doInBackground(Void... voids) {
            try{
                Log.d("Réception","Go !");
                byte[] buffer = new byte[3];
                if(socket != null) {
                    DatagramPacket p = new DatagramPacket(buffer, buffer.length);
                    socket.receive(p);
                    Log.d("Réception","J'ai reçu un truc !! :D");
                    String msg = (new String(buffer,0,p.getLength())).trim();

                    if(msg.length()>0) {
                        publishProgress(msg);
                    } else {
                        Log.d("Réception","Message : "+msg);
                    }
                }
                Log.d("Réception","Pré-fin légitime");
            } catch(Exception e) {
                Log.d("Réception","Erreur");
                publishProgress("...une erreur ...");
            }
            Log.d("Réception","Fin");

            return null;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            super.onProgressUpdate(values);
            if(values.length>=1 && values[0]!= null && values[0].equals("(1)")) {
                resultTV.setTextColor(getResources().getColor(R.color.colorWin));
                resultTV.setText("You win !");
            } else if(values.length>=1 && values[0]!= null && values[0].equals("(0)")) {
                resultTV.setTextColor(getResources().getColor(R.color.colorLoose));
                resultTV.setText("You loose !");
            } else {
                resultTV.setText("WTF ? " + values[0]);
            }
            btnPlayer1.setEnabled(true);
            btnPlayer2.setEnabled(true);
            startBtn.setEnabled(true);
            edPort.setEnabled(true);
            edIP.setEnabled(true);
            socket.close();
            socket=null;
        }
    }


    private SensorManager sm;
    protected void onResume() {
        super.onResume();
        sm = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        sm.registerListener(
                this,
                sm.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_GAME);
    }

    protected void onPause() {
        super.onPause();
        sm.unregisterListener(this);
        sm = null;
    }


    private boolean joue=false;
    private byte[] msg=null;
    private final double SEUIL=3;
    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        if(socket!=null) {
            float v =sensorEvent.values[1];
            final String s = String.format("%010f", v);;

            if(v>SEUIL) {
                if(!joue) {
                    tvData.setText(s);
                    (new Thread() {
                        @Override
                        public void run() {
                            //byte[] buffer = s.getBytes();
                            packet = new DatagramPacket(msg, msg.length, address, port);
                            try {
                                socket.send(packet);
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                    }).start();
                    joue=true;
                }
                Log.v("CAPTEUR",v+" "+joue+" positif");
            } else if(v<0) {
                joue=false;
                Log.v("CAPTEUR",v+" "+joue+" négatif");
            } else {
                Log.v("CAPTEUR",v+" "+joue+ " angle mort ...");
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }
}
