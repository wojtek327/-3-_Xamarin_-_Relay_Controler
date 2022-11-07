using System;
using Android.App;
using Android.Widget;
using Android.OS;
using System.IO;
using Java.Util;
using Android.Bluetooth;
using System.Threading.Tasks;

namespace Relay_Controler
{
    [Activity(Label = "Relay_Controler", MainLauncher = true)]
    public class MainActivity : Activity
    {
        /* 
       * Program will control four relays in two way, there is a posibility to send data with
       * start hour and stop hour,
       * second use is to to turn relay separetly it will override the time settings
       *  */
        ToggleButton connectBtn;

        /* Debug display send data */
        TextView Result;

        /* Check box data */
        CheckBox relay1CheckBox;
        CheckBox relay2CheckBox;
        CheckBox relay3CheckBox;
        CheckBox relay4CheckBox;

        /* Edit text data contain start and end time */
        EditText timeRel1StartTxt;
        EditText timeRel1EndTxt;
        EditText timeRel2StartTxt;
        EditText timeRel2EndTxt;
        EditText timeRel3StartTxt;
        EditText timeRel3EndTxt;
        EditText timeRel4StartTxt;
        EditText timeRel4EndTxt;

        ToggleButton sendRelayInfoBtn;
        ToggleButton relay1OnOffBtn;
        ToggleButton relay2OnOffBtn;
        ToggleButton relay3OnOffBtn;
        ToggleButton relay4OnOffBtn;

        /* String data to send */
        private Java.Lang.String dataToSend;

        /* Variables for bluetooth adapter and socket */
        private BluetoothAdapter mBluetoothAdapter = null;
        private BluetoothSocket btSocket = null;

        /* Output and input streams */
        private Stream outStream = null;
        private Stream inStream = null;

        /* Mac address of bluetooth device */
        private static readonly string BLT_ADDRESS = "20:16:03:21:79:76";

        /* Communication identifier */
        private static readonly UUID UUID_VAR = UUID.FromString("00001101-0000-1000-8000-00805F9B34FB");

        string timeStartRelay1Str;
        string timeEndRelay1Str;
        string timeStartRelay2Str;
        string timeEndRelay2Str;
        string timeStartRelay3Str;
        string timeEndRelay3Str;
        string timeStartRelay4Str;
        string timeEndRelay4Str;

        byte[] receiveBuffer = new byte[25];
        int position = 0;

        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);

            // Set our view from the "main" layout resource
            SetContentView(Resource.Layout.Main);

            connectBtn = FindViewById<ToggleButton>(Resource.Id.toggleButton1);

            Result = FindViewById<TextView>(Resource.Id.textView1);

            relay1CheckBox = FindViewById<CheckBox>(Resource.Id.checkBox1);
            relay2CheckBox = FindViewById<CheckBox>(Resource.Id.checkBox2);
            relay3CheckBox = FindViewById<CheckBox>(Resource.Id.checkBox3);
            relay4CheckBox = FindViewById<CheckBox>(Resource.Id.checkBox4);

            timeRel1StartTxt = FindViewById<EditText>(Resource.Id.editText1);
            timeRel1EndTxt = FindViewById<EditText>(Resource.Id.editText2);
            timeRel2StartTxt = FindViewById<EditText>(Resource.Id.editText3);
            timeRel2EndTxt = FindViewById<EditText>(Resource.Id.editText4);
            timeRel3StartTxt = FindViewById<EditText>(Resource.Id.editText5);
            timeRel3EndTxt = FindViewById<EditText>(Resource.Id.editText6);
            timeRel4StartTxt = FindViewById<EditText>(Resource.Id.editText7);
            timeRel4EndTxt = FindViewById<EditText>(Resource.Id.editText8);

            sendRelayInfoBtn = FindViewById<ToggleButton>(Resource.Id.toggleButton2);
            relay1OnOffBtn = FindViewById<ToggleButton>(Resource.Id.toggleButton3);
            relay2OnOffBtn = FindViewById<ToggleButton>(Resource.Id.toggleButton4);
            relay3OnOffBtn = FindViewById<ToggleButton>(Resource.Id.toggleButton5);
            relay4OnOffBtn = FindViewById<ToggleButton>(Resource.Id.toggleButton6);

            connectBtn.CheckedChange += TgConnect_HandleCheckedChange;
            sendRelayInfoBtn.CheckedChange += SendRelayInfoBtn_HandleCheckedChange;
            relay1OnOffBtn.CheckedChange += Relay1OnOffBtn_HandleCheckedChange;
            relay2OnOffBtn.CheckedChange += Relay2OnOffBtn_HandleCheckedChange;
            relay3OnOffBtn.CheckedChange += Relay3OnOffBtn_HandleCheckedChange;
            relay4OnOffBtn.CheckedChange += Relay4OnOffBtn_HandleCheckedChange;

            ConnectToBtDevice();
        }


        /* Bluetooth veryfication, check if it enable */
        /* #################################################################### */
        private void ConnectToBtDevice()
        {
            /* Write bluetooth device with what there will be communication */
            mBluetoothAdapter = BluetoothAdapter.DefaultAdapter;

            //Check if device is enabled
            if (!mBluetoothAdapter.Enable())
            {
                Toast.MakeText(this, "OK: Bluetooth Disconnected", ToastLength.Short).Show();
            }
            else
            {
                Toast.MakeText(this, "OK: Bluetooth Connected", ToastLength.Short).Show();
            }

            //check if data was write into device
            if (mBluetoothAdapter == null)
            {
                Toast.MakeText(this,
                    "ERROR: Bluetooth don't exist or it is busy", ToastLength.Short)
                    .Show();
            }
        }

        /* Event for change btn for establish/disable connection */
        /* #################################################################### */
        void TgConnect_HandleCheckedChange(object sender, CompoundButton.CheckedChangeEventArgs e)
        {
            if (e.IsChecked)
            {
                /* After btn connect inicialize is connect method */
                try
                {
                    ConnectBlt();
                    Toast.MakeText(this, "Device Connected", ToastLength.Short).Show();
                }
                catch (System.Exception ex)
                {
                    Toast.MakeText(this, "Close Error: " + ex.Message, ToastLength.Short).Show();
                }
            }
            else
            {
                /* After disable btn clicked then disconnect */
                if (btSocket.IsConnected)
                {
                    try
                    {
                        btSocket.Close();
                        Toast.MakeText(this, "Connection Closed", ToastLength.Short).Show();
                    }
                    catch (System.Exception ex)
                    {
                        Toast.MakeText(this, "Close Error" + ex.Message, ToastLength.Short).Show();
                    }
                }
            }
        }

        /* #################################################################### */
        public void ConnectBlt()
        {
            BluetoothDevice device = mBluetoothAdapter.GetRemoteDevice(BLT_ADDRESS);    /* Start connection with HC-05 */

            mBluetoothAdapter.CancelDiscovery();                /* Tell adapter that he is not visible */
            try
            {
                /* Initialize communication socket with device */
                btSocket = device.CreateRfcommSocketToServiceRecord(UUID_VAR);
                btSocket.Connect();                /* Socket Connect */
            }
            catch (System.Exception e)
            {
                /* When error occure then close socket */
                try
                {
                    btSocket.Close();
                }
                catch (System.Exception)
                {
                    Toast.MakeText(this, "Close Error" + e.Message, ToastLength.Short).Show();
                }
            }

            /* After establish connection with blt device we will call method that generate thread 
                for receiving information */
            BeginListenForData();
        }

        /* #################################################################### */
        /* Event that initialize the thread that will listen the bluetooth request */
        public void BeginListenForData()
        {
            /* Extract input stream */
            try
            {
                inStream = btSocket.InputStream;
            }
            catch (System.IO.IOException ex)
            {
                Toast.MakeText(this, "Close Error" + ex.Message, ToastLength.Short).Show();
            }

            /* Create a thread that will be running in background 
             * which will check if there is any data*/
            Task.Factory.StartNew(() => {
                byte[] buffer = new byte[1024];          /* Declare the buffer where we will keep the reading */
                int bytes = 0;                               /* Declete number of received bytes */

                while (true)
                {
                    try
                    {
                        /* Read the input buffer and assign the number of incoming bytes */
                        bytes = inStream.Read(buffer, 0, buffer.Length);

                        if (bytes > 0)
                        {
                            UpdateDataOnScreen(buffer, bytes);
                            RunOnUiThread(() => {
                                /* Add arrival information to text data in screen */
                                Result.Text = "1:" + buffer[0].ToString() + "\n";
                            });
                        }
                    }
                    catch (Java.IO.IOException)
                    {
                        Toast.MakeText(this, "Java.IO.IOException", ToastLength.Short).Show();
                    }
                }
            });
        }

        /* Method use for sending data to BLT device */
        private void WriteData(Java.Lang.String data)
        {
            /* Extrack output stream */
            try
            {
                outStream = btSocket.OutputStream;
            }
            catch (System.Exception e)
            {
                Toast.MakeText(this, "Sending error: " + e.Message, ToastLength.Short).Show();
            }

            /* Create string to send */
            Java.Lang.String message = data;

            /* Convert it into bytes */
            byte[] msgBuffer = message.GetBytes();

            try
            {
                /* write the arrangement that we just generated in the buffer */
                outStream.Write(msgBuffer, 0, msgBuffer.Length);
                Toast.MakeText(this, "Data Sended", ToastLength.Short).Show();
            }
            catch (System.Exception e)
            {
                Toast.MakeText(this, "Sending error" + e.Message, ToastLength.Short).Show();
            }
        }

        private void UpdateDataOnScreen(byte[] buffer, int bytes)
        {
            /* Receive buffer structure */
            /* 
             *  Relay state: 0xA5 - Low State
             *               0xB9 - High State
             *  
             *  buffer[0] - Last State R1
             *  buffer[1] - Last State R2
             *  buffer[2] - Last State R3
             *  buffer[3] - Last State R4

             *  buffer[4] - Relay 1 Time On/Off 0xA5/0xB9
             *  buffer[5] - HourStart Relay 1
             *  buffer[6] - MinutesStart Relay 1
             *  buffer[7] - HourStop Relay 1
             *  buffer[8] - MinutesStop Relay 1
             *  
             *  buffer[9] - Relay 2 Time On/Off 0xA5/0xB9
             *  buffer[10] - HourStart Relay 2
             *  buffer[11] - MinutesStart Relay 2
             *  buffer[12] - HourStop Relay 2
             *  buffer[13] - MinutesStop Relay 2
             *  
             *  buffer[14] - Relay 3 Time On/Off 0xA5/0xB9
             *  buffer[15] - HourStart Relay 3
             *  buffer[16] - MinutesStart Relay 3
             *  buffer[17] - HourStop Relay 3
             *  buffer[18] - MinutesStop Relay 3
             *  
             *  buffer[19] - Relay 4 Time On/Off 0xA5/0xB9
             *  buffer[20] - HourStart Relay 4
             *  buffer[21] - MinutesStart Relay 4
             *  buffer[22] - HourStop Relay 4
             *  buffer[23] - MinutesStop Relay 4
             */

            RunOnUiThread(() => {
                /* Add arrival information to text data in screen */
                Result.Text = "12:" + bytes.ToString() + "\n";
            });

            if (buffer[0] == 0xA5)
            {
                relay1OnOffBtn.Text = "RELAY1 OFF";
            }
            else if (buffer[0] == 0xB9)
            {
                relay1OnOffBtn.Text = "RELAY1 ON";
            }

            if (buffer[1] == 0xA5)
            {
                relay1OnOffBtn.Text = "RELAY2 OFF";
            }
            else if (buffer[1] == 0xB9)
            {
                relay1OnOffBtn.Text = "RELAY2 ON";
            }

            if (buffer[2] == 0xA5)
            {
                relay1OnOffBtn.Text = "RELAY3 OFF";
            }
            else if (buffer[2] == 0xB9)
            {
                relay1OnOffBtn.Text = "RELAY3 ON";
            }

            if (buffer[3] == 0xA5)
            {
                relay1OnOffBtn.Text = "RELAY4 OFF";
            }
            else if (buffer[3] == 0xB9)
            {
                relay1OnOffBtn.Text = "RELAY4 ON";
            }

            if (buffer[4] == 0xA5)
            {
                relay1CheckBox.Checked = true;

                timeRel1StartTxt.Text = buffer[5].ToString("00") + ":" + buffer[6].ToString("00");
                timeRel1EndTxt.Text = buffer[7].ToString("00") + ":" + buffer[8].ToString("00");
            }
            else
            {
                relay1CheckBox.Checked = false;
            }

            if (buffer[9] == 0xA5)
            {
                relay2CheckBox.Checked = true;

                timeRel2StartTxt.Text = buffer[10].ToString("00") + ":" + buffer[11].ToString("00");
                timeRel2EndTxt.Text = buffer[12].ToString("00") + ":" + buffer[13].ToString("00");
            }
            else
            {
                relay2CheckBox.Checked = false;
            }

            if (buffer[14] == 0xA5)
            {
                relay3CheckBox.Checked = true;

                timeRel3StartTxt.Text = buffer[15].ToString("00") + ":" + buffer[16].ToString("00");
                timeRel3EndTxt.Text = buffer[17].ToString("00") + ":" + buffer[18].ToString("00");
            }
            else
            {
                relay3CheckBox.Checked = false;
            }

            if (buffer[19] == 0xA5)
            {
                relay4CheckBox.Checked = true;
                timeRel4StartTxt.Text = buffer[20].ToString("00") + ":" + buffer[21].ToString("00");
                timeRel4EndTxt.Text = buffer[22].ToString("00") + ":" + buffer[23].ToString("00");
            }
            else
            {
                relay4CheckBox.Checked = false;
            }

        }

        /* #################################################################### */
        void SendRelayInfoBtn_HandleCheckedChange(object sender, CompoundButton.CheckedChangeEventArgs e)
        {
            Java.Lang.String dataForBltForSend;
            string prepareString = "Wri";

            if (sendRelayInfoBtn.Text == "READ DATA FROM DEVICE")
            {
                dataToSend = new Java.Lang.String("b" + "ReadData" + "xx" + "\n");
                WriteData(dataToSend);
            }
            else if (sendRelayInfoBtn.Text == "WRITE DATA TO DEVICE")
            {
                prepareString = "Wri";

                /* get data from check box and  */
                if (relay1CheckBox.Checked == true)
                {
                    timeStartRelay1Str = timeRel1StartTxt.Text;
                    timeEndRelay1Str = timeRel1EndTxt.Text;

                    prepareString += "R1" + timeRel1StartTxt.Text.ToString() + ";"
                                            + timeRel1EndTxt.Text.ToString() + ";";
                }
                else
                {
                    prepareString += "R1x;";
                }

                if (relay2CheckBox.Checked == true)
                {
                    timeStartRelay2Str = timeRel2StartTxt.Text;
                    timeEndRelay2Str = timeRel2EndTxt.Text;

                    prepareString += "R2" + timeRel2StartTxt.Text.ToString() + ";"
                                            + timeRel2EndTxt.Text.ToString() + ";";
                }
                else
                {
                    prepareString += "R2x;";
                }

                if (relay3CheckBox.Checked == true)
                {
                    timeStartRelay3Str = timeRel3StartTxt.Text;
                    timeEndRelay3Str = timeRel3EndTxt.Text;

                    prepareString += "R3" + timeRel3StartTxt.Text.ToString() + ";"
                                            + timeRel3EndTxt.Text.ToString() + ";";
                }
                else
                {
                    prepareString += "R3x;";
                }

                if (relay4CheckBox.Checked == true)
                {
                    timeStartRelay4Str = timeRel4StartTxt.Text;
                    timeEndRelay4Str = timeRel4EndTxt.Text;

                    prepareString += "R4" + timeRel4StartTxt.Text.ToString() + ";"
                                            + timeRel4EndTxt.Text.ToString() + ";";
                }
                else
                {
                    prepareString += "R4x;";
                }

                dataForBltForSend = new Java.Lang.String("b" + prepareString + "xx\n");
                WriteData(dataForBltForSend);
            }
        }

        /* #################################################################### */
        void Relay1OnOffBtn_HandleCheckedChange(object sender, CompoundButton.CheckedChangeEventArgs e)
        {
            Relay1_ClickMethod();
        }

        void Relay1_ClickMethod()
        {
            if (relay1OnOffBtn.Text == "RELAY1 ON")
            {
                dataToSend = new Java.Lang.String("b" + "R1On" + "xx" + "\n");
                WriteData(dataToSend);
            }
            else if (relay1OnOffBtn.Text == "RELAY1 OFF")
            {
                dataToSend = new Java.Lang.String("b" + "R1Off" + "xx" + "\n");
                WriteData(dataToSend);
            }
        }
        /* #################################################################### */
        void Relay2OnOffBtn_HandleCheckedChange(object sender, CompoundButton.CheckedChangeEventArgs e)
        {
            Relay2_ClickMethod();
        }

        void Relay2_ClickMethod()
        {
            if (relay2OnOffBtn.Text == "RELAY2 ON")
            {
                dataToSend = new Java.Lang.String("b" + "R2On" + "xx" + "\n");
                WriteData(dataToSend);
            }
            else if (relay2OnOffBtn.Text == "RELAY2 OFF")
            {
                dataToSend = new Java.Lang.String("b" + "R2Off" + "xx" + "\n");
                WriteData(dataToSend);
            }
        }
        /* #################################################################### */
        void Relay3OnOffBtn_HandleCheckedChange(object sender, CompoundButton.CheckedChangeEventArgs e)
        {
            Relay3_ClickMethod();
        }

        void Relay3_ClickMethod()
        {
            if (relay3OnOffBtn.Text == "RELAY3 ON")
            {
                dataToSend = new Java.Lang.String("b" + "R3On" + "xx" + "\n");
                WriteData(dataToSend);
            }
            else if (relay3OnOffBtn.Text == "RELAY3 OFF")
            {
                dataToSend = new Java.Lang.String("b" + "R3Off" + "xx" + "\n");
                WriteData(dataToSend);
            }
        }
        /* #################################################################### */
        void Relay4OnOffBtn_HandleCheckedChange(object sender, CompoundButton.CheckedChangeEventArgs e)
        {
            Relay4_ClickMethod();
        }

        void Relay4_ClickMethod()
        {
            if (relay4OnOffBtn.Text == "RELAY4 ON")
            {
                dataToSend = new Java.Lang.String("b" + "R4On" + "xx" + "\n");
                WriteData(dataToSend);
            }
            else if (relay4OnOffBtn.Text == "RELAY4 OFF")
            {
                dataToSend = new Java.Lang.String("b" + "R4Off" + "xx" + "\n");
                WriteData(dataToSend);
            }
        }
    }
}

