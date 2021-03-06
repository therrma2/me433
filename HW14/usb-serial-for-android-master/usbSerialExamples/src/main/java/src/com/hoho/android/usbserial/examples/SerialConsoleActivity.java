/* Copyright 2011-2013 Google Inc.
 * Copyright 2013 mike wakerly <opensource@hoho.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * Project home page: https://github.com/mik3y/usb-serial-for-android
 */

package com.hoho.android.usbserial.examples;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ScrollView;
import android.widget.TextView;

import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.util.HexDump;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
//import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
// libraries
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import java.io.IOException;
import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;

/**
 * Monitors a single {@link UsbSerialPort} instance, showing all data
 * received.
 *
 * @author mike wakerly (opensource@hoho.com)
 */
public class SerialConsoleActivity extends Activity implements TextureView.SurfaceTextureListener {
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640,480,Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;
    private SeekBar bar1;
    private SeekBar bar2;
    private SeekBar bar3;
    private SeekBar bar4;
    private TextView bar1text;
    private TextView bar2text;
    private TextView bar3text;
    private TextView bar4text;
    private TextView command;
    public int bar1val = 176;
    public int bar2val = 115;
    public int bar3val = 229;
    public int bar4val = 213;
    public int[] pos = new int[4];
    public int[] com = new int[4];
    public int[] ang = new int[3];

    MediaPlayer mySound;


    static long prevtime = 0; // for FPS calculation

    private final String TAG = SerialConsoleActivity.class.getSimpleName();

    /**
     * Driver instance, passed in statically via
     * {@link #show(Context, UsbSerialPort)}.
     *
     * <p/>
     * This is a devious hack; it'd be cleaner to re-create the driver using
     * arguments passed in with the {@link #startActivity(Intent)} intent. We
     * can get away with it because both activities will run in the same
     * process, and this is a simple demo.
     */
    private static UsbSerialPort sPort = null;

    private TextView mTitleTextView;
    private TextView mDumpTextView;
    private ScrollView mScrollView;
    private CheckBox chkDTR;
    private CheckBox chkRTS;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    private SerialInputOutputManager mSerialIoManager;

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {

        @Override
        public void onRunError(Exception e) {
            Log.d(TAG, "Runner stopped.");
        }

        @Override
        public void onNewData(final byte[] data) {
            SerialConsoleActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    SerialConsoleActivity.this.updateReceivedData(data);
                }
            });
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.serial_console);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        bar1 = (SeekBar) findViewById(R.id.bar1);
        bar2 = (SeekBar) findViewById(R.id.bar2);
        bar3 = (SeekBar) findViewById(R.id.bar3);
        bar4 = (SeekBar) findViewById(R.id.bar4);

        bar1.setProgress(bar1val);
        bar2.setProgress(bar2val);
        bar3.setProgress(bar3val);
        bar4.setProgress(bar4val);

        pos[0] = 50;
        pos[1] = 150;
        pos[2] = 250;
        pos[3] = 400;

        bar1text = (TextView) findViewById(R.id.bar1text);
        bar2text = (TextView) findViewById(R.id.bar2text);
        bar3text = (TextView) findViewById(R.id.bar3text);
        bar4text = (TextView) findViewById(R.id.bar4text);

        bar1text.setText(""+bar1val);
        bar2text.setText("Red > "+bar2val);
        bar3text.setText("Green < "+bar3val);
        bar4text.setText("Blue < "+bar4val);

        mySound = MediaPlayer.create(this,R.raw.jurassic_clip);
        mySound.start();



        command = (TextView) findViewById(R.id.command);
        //command.setText("Hard Right!");


        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        mSurfaceHolder = mSurfaceView.getHolder();

        mTextureView = (TextureView) findViewById(R.id.textureview);
        mTextureView.setSurfaceTextureListener(this);

        mTextView = (TextView) findViewById(R.id.cameraStatus);

        paint1.setColor(0xffff0ff0); // red
        paint1.setTextSize(24);


        setBar1Listener();
        setBar2Listener();
        setBar3Listener();
        setBar4Listener();

        mTitleTextView = (TextView) findViewById(R.id.demoTitle);
        mDumpTextView = (TextView) findViewById(R.id.consoleText);
        mScrollView = (ScrollView) findViewById(R.id.demoScroller);
        chkDTR = (CheckBox) findViewById(R.id.checkBoxDTR);
        chkRTS = (CheckBox) findViewById(R.id.checkBoxRTS);

        chkDTR.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    sPort.setDTR(isChecked);
                }catch (IOException x){}
            }
        });

        chkRTS.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                try {
                    sPort.setRTS(isChecked);
                }catch (IOException x){}
            }
        });

    }


    @Override
    protected void onPause() {
        super.onPause();
        stopIoManager();
        if (sPort != null) {
            try {
                sPort.close();
            } catch (IOException e) {
                // Ignore.
            }
            sPort = null;
        }
        finish();
    }

    void showStatus(TextView theTextView, String theLabel, boolean theValue){
        String msg = theLabel + ": " + (theValue ? "enabled" : "disabled") + "\n";
        theTextView.append(msg);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "Resumed, port=" + sPort);
        if (sPort == null) {
            mTitleTextView.setText("No serial device.");
        } else {
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

            UsbDeviceConnection connection = usbManager.openDevice(sPort.getDriver().getDevice());
            if (connection == null) {
                mTitleTextView.setText("Opening device failed");
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

                showStatus(mDumpTextView, "CD  - Carrier Detect", sPort.getCD());
                showStatus(mDumpTextView, "CTS - Clear To Send", sPort.getCTS());
                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
                showStatus(mDumpTextView, "DTR - Data Terminal Ready", sPort.getDTR());
                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
                showStatus(mDumpTextView, "RI  - Ring Indicator", sPort.getRI());
                showStatus(mDumpTextView, "RTS - Request To Send", sPort.getRTS());

            } catch (IOException e) {
                Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                mTitleTextView.setText("Error opening device: " + e.getMessage());
                try {
                    sPort.close();
                } catch (IOException e2) {
                    // Ignore.
                }
                sPort = null;
                return;
            }
            mTitleTextView.setText("Serial device: " + sPort.getClass().getSimpleName());
        }
        onDeviceStateChange();
    }

    private void stopIoManager() {
        if (mSerialIoManager != null) {
            Log.i(TAG, "Stopping io manager ..");
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if (sPort != null) {
            Log.i(TAG, "Starting io manager ..");
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange() {
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
//        final String message = "Read " + data.length + " bytes: \n"
//                + HexDump.dumpHexString(data) + "\n\n";
//        mDumpTextView.append(message);
//        mScrollView.smoothScrollTo(0, mDumpTextView.getBottom());
//        //byte[] sData = {'a',0}; try { sPort.write(sData, 10); } catch (IOException e) { }
//        command.setText(""+data.length);
    }

    /**
     * Starts the activity, using the supplied driver instance.
     *
     * @param context
     * @param driver
     */
    static void show(Context context, UsbSerialPort port) {
        sPort = port;
        final Intent intent = new Intent(context, SerialConsoleActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NO_HISTORY);
        context.startActivity(intent);
    }




    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setColorEffect(Camera.Parameters.EFFECT_NONE); // black and white
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }
    @Override
    public void onDestroy(){
        mySound.stop();
        mySound.release();
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }
    private void setBar1Listener(){
        bar1.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                bar1val = progress;
                bar1text.setText(""+progress);

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }
    private void setBar2Listener(){
        bar2.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                bar2text.setText("Red > "+progress);
                bar2val = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }
    private void setBar3Listener(){
        bar3.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                bar3text.setText("Green < "+progress);
                bar3val = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }
    private void setBar4Listener(){
        bar4.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
            int progressChanged = 0;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                bar4text.setText("Blue < "+progress);
                bar4val = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // Invoked every time there's a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();





        if (c != null) {

            for (int j=0; j<4;j++) {

                int[] pixels = new int[bmp.getWidth()];
                int startY = j; // which row in the bitmap to analyse to read
                // only look at one row in the image
                bmp.getPixels(pixels, 0, bmp.getWidth(), 0, pos[j], bmp.getWidth(), 1); // (array name, offset inside array, stride (size of row), start x, start y, num pixels to read per row, num rows to read)

                // pixels[] is the RGBA data (in black an white).
                // instead of doing center of mass on it, decide if each pixel is dark enough to consider black or white
                // then do a center of mass on the thresholded array
                int[] thresholdedPixels = new int[bmp.getWidth()];
                int wbTotal = 0; // total mass
                int wbCOM = 0; // total (mass time position)
                for (int i = 0; i < bmp.getWidth(); i++) {
                    // sum the red, green and blue, subtract from 255*3 to get the darkness of the pixel.
                    // if it is greater than some value (600 here), consider it black
                    // play with the 600 value if you are having issues reliably seeing the line
                    //if (255*3-(red(pixels[i])+green(pixels[i])+blue(pixels[i])) > 600) {
                    //    thresholdedPixels[i] = 255*3;
                    //}
                    if ((red(pixels[i]) > bar2val) && (green(pixels[i]) < bar3val) && (blue(pixels[i]) < bar4val)) {
                        thresholdedPixels[i] = 255 * 3;
                        bmp.setPixel(i, pos[j], 0xff000000);

                    } else {
                        thresholdedPixels[i] = 0;
                    }
                    wbTotal = wbTotal + thresholdedPixels[i];
                    wbCOM = wbCOM + thresholdedPixels[i] * i;
                }
                int COM;
                //watch out for divide by 0
                if (wbTotal <= 0) {
                    COM = bmp.getWidth() / 2;
                    canvas.drawText("NO RED DETECTED", 10, pos[j], paint1);
                    c.drawBitmap(bmp, 0, 0, null);
                    com[j] = 0;

                } else {
                    COM = wbCOM / wbTotal;
                    // draw a circle where you think the COM is
                    canvas.drawCircle(COM, pos[j], 5, paint1);

                    // also write the value as text
                    canvas.drawText("COM = " + COM, 10, pos[j], paint1);
                    c.drawBitmap(bmp, 0, 0, null);
                    com[j] = COM;

                }


            }


            //ang[0] = com[2]-com[3];
            //ang[1] = com[1]-com[2];
            //ang[2] = com[0]-com[1];

            int dat1= 0;
            int dat2 = 0;

//            if ((com[1]>= 180) && (com[1]<= 460)){ //forward particle is centered
//
//                //full speed ahead!
//                //command.setText("Full Speed Ahead!");
//
//                dat1 = 412000;
//                dat2 = 612000;
//
//            }
//
//            if (com[1]> 460){ //forward particle is right
//
//                dat1 = 412000;
//                dat2 = 602000;
////                if (com[3] <280) {//back particle is left
////                    //command.setText("Hard Right!");
////                    dat1 = 412000;
////                    dat2 = 600000;
////                }
////                if (com[3] > 360){//back particle is right
////                    //command.setText("Slowly Veer Right");
////                    dat1 = 412000;
////                    dat2 = 610000;
////                }
////                if ((com[3] >=280)&&(com[3]<=360)){//back particle is centered
////                    //command.setText("Normal Turn Right");
////                    dat1 = 412000;
////                    dat2 = 604000;
////                }
//            }
//
//            if (com[1]< 180){ //forward particle is left
////                if (com[3] <280) {//back particle is left
////                    //command.setText("Slowly Veer Left");
////                    dat1 = 410000;
////                    dat2 = 612000;
////                }
////                if (com[3] > 360){//back particle is right
////                    //command.setText("Hard Left!");
////                    dat1 = 400000;
////                    dat2 = 612000;
////                }
////                if ((com[3] >=280)&&(com[3]<=360)){//back particle is centered
////                    //command.setText("Normal Turn Left");
////                    dat1 = 404000;
////                    dat2 = 612000;
////                }
//                dat1 = 402000;
//                dat2 = 612000;
//            }
//            int dist;
            if (com[1]<=320){
                dat1 = com[1]*12000/320+400000;
                dat2 = 12000+600000;
                //dist = 320-com[1];

            }
            if (com[1]>320){
                dat1 = 12000 + 400000;
                dat2 = (640-com[1])*12000/320+600000;
                //dist = com[1]-320;

            }
//            dat1 = dat1*dist/320
//            if (dat1<400000){
//                dat1= 400000;
//            }
//
//            if (dat2<600000){
//                dat2= 600000;
//            }
            if (com[1]>0) {


                String sendString1 = String.valueOf(dat1) + "\n";
                String sendString2 = String.valueOf(dat2) + "\n";

                try {
                    sPort.write(sendString1.getBytes(), 10);
                } catch (IOException e) {
                }
                try {
                    sPort.write(sendString2.getBytes(), 10);
                } catch (IOException e) {
                }


                int dat1val = dat1 - 400000;
                int dat2val = dat2 - 600000;
                command.setText(dat1val + "     -     " + dat2val);
            }

            mSurfaceHolder.unlockCanvasAndPost(c);

            // calculate the FPS to see how fast the code is running
            long nowtime = System.currentTimeMillis();
            long diff = nowtime - prevtime;
            mTextView.setText("FPS " + 1000 / diff);
            prevtime = nowtime;
        }

    }
}