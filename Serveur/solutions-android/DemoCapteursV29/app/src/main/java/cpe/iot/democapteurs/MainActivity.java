package cpe.iot.democapteurs;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity implements SensorEventListener {
    private TextView tvX;
    private TextView tvY;
    private TextView tvZ;
    private TextView tvCapteur;
    private SensorManager sm;
    private int sensType;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvX = (TextView) findViewById(R.id.valX);
        tvY = (TextView) findViewById(R.id.valY);
        tvZ = (TextView) findViewById(R.id.valZ);
        tvCapteur = (TextView) findViewById(R.id.capteur);

        // Choix du capteur à utiliser
        sensType = Sensor.TYPE_ACCELEROMETER;
    }

    protected void onResume() {
        super.onResume();
        sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        Sensor sens = sm.getDefaultSensor(sensType);
        sm.registerListener(
                this,
                sens,
                SensorManager.SENSOR_DELAY_UI);
        tvCapteur.setText(sens.getName());
    }

    protected void onPause() {
        super.onPause();
        sm.unregisterListener(this);
        sm = null;
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        tvX.setText("" + event.values[0]);
        tvY.setText("" + event.values[1]);
        tvZ.setText("" + event.values[2]);
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

}