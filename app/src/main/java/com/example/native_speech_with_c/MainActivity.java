package com.example.native_speech_with_c;


import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.Image;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.ShortBuffer;
import java.nio.channels.FileChannel;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;


public class MainActivity extends AppCompatActivity {
    private static final int SAMPLING_RATE_IN_HZ = 16000;

    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_IN_MONO;

    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;

    private static final int BUFFER_SIZE_FACTOR = 2;

    private int sid;
    private int imageId;
    private static final int BUFFER_SIZE = AudioRecord.getMinBufferSize(SAMPLING_RATE_IN_HZ,
            CHANNEL_CONFIG, AUDIO_FORMAT) * BUFFER_SIZE_FACTOR;

    private final AtomicBoolean recordingInProgress = new AtomicBoolean(false);
    private static final int REQUEST_RECORD_AUDIO_PERMISSION = 200;
    private AudioRecord recorder = null;
    private boolean permissionToRecordAccepted = false;
    private boolean isPermissionToWrite = false;
    private String [] permissions = {Manifest.permission.RECORD_AUDIO,Manifest.permission.WRITE_EXTERNAL_STORAGE};
    private Thread recordingThread = null;
    private Thread stop_thread = null;

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode){
            case REQUEST_RECORD_AUDIO_PERMISSION:
                permissionToRecordAccepted  = grantResults[0] == PackageManager.PERMISSION_GRANTED;
                break;

        }
        //if (!permissionToRecordAccepted ) finish();

    }
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ActivityCompat.requestPermissions(this, permissions, REQUEST_RECORD_AUDIO_PERMISSION);
        ActivityCompat.requestPermissions(this,
                permissions,
                205);

        final ImageButton i1 = (ImageButton)findViewById(R.id.imageButton);

        i1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {


                startRecording();
                i1.setEnabled(false);
              //  i1.setBackgroundColor(Color.GREEN);
                try {
                    stop_runnable();

                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        });

        final ImageButton b5 = (ImageButton) findViewById(R.id.button);
        b5.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = getIntent();
                finish();
                startActivity(intent);
            }
        });


     final ImageView iv = (ImageView)findViewById(R.id.imageView);
     //String path = "/storage/emulated/0/Android/data/com.example.native_speech_with_c/files";
        int set[][] = { {R.drawable.google,R.drawable.im_2,R.drawable.im_7},{R.drawable.down,R.drawable.im_0,R.drawable.im_9},
                {R.drawable.alexa,R.drawable.cow,R.drawable.im_6},{R.drawable.google,R.drawable.im_16,R.drawable.im_17},
                {R.drawable.im_14,R.drawable.im_15,R.drawable.hello},{R.drawable.risk,R.drawable.hello,R.drawable.im_19},
                {R.drawable.risk,R.drawable.cow,R.drawable.google},{R.drawable.alexa,R.drawable.im_4,R.drawable.im_5},
                {R.drawable.cow,R.drawable.im_11,R.drawable.im_18},{R.drawable.im_3,R.drawable.im_6,R.drawable.hello}};

        final Random rnd = new Random();
        final Random rind = new Random();
      //  int imageId = (int)(Math.random() * images.length);
        imageId = rnd.nextInt(3);
        sid = rind.nextInt(10);

// Set the image
       iv.setBackgroundResource(set[sid][imageId]);
      ImageView iv1 = (ImageView)findViewById(R.id.textView2);
      iv1.setImageResource(R.drawable.not_a_robot);
    }

    private void stop_runnable() throws InterruptedException {
        stop_thread = new Thread(new stop_thread_runnable(), "stop thread");
        stop_thread.sleep(3000);
        stopRecording();
        stop_thread.sleep(500);
        String s ="",x="";
        int answer=-1;
        Log.e("Sid:", ""+sid +"");
        Log.e("Imid:", ""+imageId +"");
        Log.e("Answer:", ""+answer +"");
        if(sid==0) {
            s=group1();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);
            if(s.equals(new String("google")))
                answer=0;
            if(s.equals(new String("Two")))
                answer=1;
            if(s.equals(new String("Seven")))
                answer=2;

            Log.e("S::", "" +s);
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==1) {
            s=group2();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);
            if(s.equals(new String("down")))
                answer=0;
            if(s.equals(new String("Zero")))
                answer=1;
            if(s.equals(new String("Nine")))
                answer=2;

            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==2) {
            s=group3();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);

            if(s.equals(new String("alexa")))
                answer=0;
            if(s.equals(new String("cow")))
                answer=1;
            if(s.equals(new String("Six")))
                answer=2;

            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==3) {
            s=group4();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);
            if(s.equals(new String("google")))
                answer=0;
            if(s.equals(new String("Six")))
                answer=1;
            if(s.equals(new String("Seven")))
                answer=2;
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==4) {
            s=group5();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);
            if(s.equals(new String("Four")))
                answer=0;
            if(s.equals(new String("Five")))
                answer=1;
            if(s.equals(new String("hello")))
                answer=2;
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==5) {
            s=group6();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);

            if(s.equals(new String("risk")))
                answer=0;
            if(s.equals(new String("hello")))
                answer=1;
            if(s.equals(new String("Nine")))
                answer=2;
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==6) {
            s=group7();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);

            if(s.equals(new String("risk")))
                answer=0;
            if(s.equals(new String("cow")))
                answer=1;
            if(s.equals(new String("google")))
                answer=2;
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }

        if(sid==7) {
            s=group8();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);

            if(s.equals(new String("alexa")))
                answer=0;
            if(s.equals(new String("Four")))
                answer=1;
            if(s.equals(new String("Five")))
                answer=2;
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }


        if(sid==8) {
            s=group9();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);

            if(s.equals(new String("cow")))
                answer=0;
            if(s.equals(new String("One")))
                answer=1;
            if(s.equals(new String("Eight")))
                answer=2;
            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }


        if(sid==9) {
            s=group10();
            EditText et = (EditText) findViewById(R.id.editText3);
            et.setText(s);
            if(s.equals(new String("Three")))
                answer=0;
            if(s.equals(new String("Six")))
                answer=1;
            if(s.equals(new String("hello")))
                answer=2;

            if(imageId==answer) {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_checked);
            }
            else
            {
                ImageView tv = (ImageView) findViewById(R.id.textView2);
                tv.setImageResource(R.drawable.not_a_robot_crossed);
            }
        }



    }

    private class stop_thread_runnable implements Runnable{
        @Override
        public void run() {

        }
    }
    private void startRecording() {
        recorder = new AudioRecord(MediaRecorder.AudioSource.DEFAULT, SAMPLING_RATE_IN_HZ,
                CHANNEL_CONFIG, AUDIO_FORMAT, BUFFER_SIZE);

        recorder.startRecording();

        recordingInProgress.set(true);

        recordingThread = new Thread(new RecordingRunnable(), "Recording Thread");
        recordingThread.start();

        ImageButton i1 = (ImageButton)findViewById(R.id.imageButton);
        i1.setEnabled(true);
    }

    private void stopRecording() {
        if (null == recorder) {
            return;
        }

        recordingInProgress.set(false);

        recorder.stop();

        recorder.release();

        recorder = null;

        recordingThread = null;
        pcmtotexxt();



    }

    private class RecordingRunnable implements Runnable {

        @Override
        public void run() {

           // final File file = new File(getExternalCacheDir().getAbsolutePath(), "recording.pcm");

            final File file1 =new File("/storage/emulated/0/Android/data/com.example.native_speech_with_c/files", "recording.pcm");
            final ByteBuffer buffer = ByteBuffer.allocateDirect(BUFFER_SIZE);
            try (final FileOutputStream outStream = new FileOutputStream(file1)) {

              /*  if(!file1.exists()) {
                    file1.createNewFile();
                }*/
                //FileWriter fileWritter = new FileWriter(file1.getName(),true);
                //BufferedWriter bw = new BufferedWriter(fileWritter);
                while (recordingInProgress.get()) {
                    int result = recorder.read(buffer, BUFFER_SIZE);
                    if (result < 0) {
                        throw new RuntimeException("Reading of audio buffer failed: " +
                                getBufferReadFailureReason(result));
                    }

                   outStream.write(buffer.array(),0,BUFFER_SIZE);


                    buffer.clear();

                }


//bw.close();
            }
            catch (IOException e) {
                throw new RuntimeException("Writing of recorded audio failed", e);

            }
        }

        private String getBufferReadFailureReason(int errorCode) {
            switch (errorCode) {
                case AudioRecord.ERROR_INVALID_OPERATION:
                    return "ERROR_INVALID_OPERATION";
                case AudioRecord.ERROR_BAD_VALUE:
                    return "ERROR_BAD_VALUE";
                case AudioRecord.ERROR_DEAD_OBJECT:
                    return "ERROR_DEAD_OBJECT";
                case AudioRecord.ERROR:
                    return "ERROR";
                default:
                    return "Unknown (" + errorCode + ")";
            }
        }
    }

    public native void pcmtotexxt();
    public native  String stringFromJNI();
    public native  String group1();
    public native  String group2();
    public native  String group3();
    public native  String group4();
    public native  String group5();
    public native  String group6();
    public native  String group7();
    public native  String group8();
    public native  String group9();
    public native  String group10();





}
