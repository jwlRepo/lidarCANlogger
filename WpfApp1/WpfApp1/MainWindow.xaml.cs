using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Windows.Threading;


namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    
    public partial class MainWindow : Window
    {
        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern IntPtr createLidar(string IP, int port, int timeout);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void lidar_del(IntPtr pClassNameObject);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern int lidar_conn(IntPtr pClassNameObject);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void lidar_startLog(IntPtr pClassNameObject, string filename);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void lidar_stopLog(IntPtr pClassNameObject);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void lidar_setPlane(IntPtr pClassNameObject, int plane);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern IntPtr createAxio(string IP, int port, int timeout);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void axio_del(IntPtr pClassNameObject);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern int axio_conn(IntPtr pClassNameObject);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void axio_startLog(IntPtr pClassNameObject, string filename);

        [DllImport("gpsImuDll.dll", CallingConvention = CallingConvention.Cdecl)]
        static public extern void axio_stopLog(IntPtr pClassNameObject);

        bool logging = false;
        bool connect = false;
        IntPtr lidar_ptr;
        int lidarPort_Def = 2111;
        string lidarIP_Def = "169.254.76.136";
        IntPtr axio_ptr;
        int axioPort_Def = 4000;
        string axioIP_Def = "169.254.0.34";
        UInt64 timeInterval;

        DispatcherTimer timer1 = new DispatcherTimer();

        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(Window_Loaded);
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            timer1.Tick += new EventHandler(timerTick);
            timer1.Interval = new TimeSpan(0, 0, 1);
            timeInterval = 0;
            consoleManager.Show();
        }

        private void timerTick(object sender, EventArgs e)
        {
            timeInterval += Convert.ToUInt64(timer1.Interval.TotalSeconds);
            timerBox.Text = (timeInterval/3600).ToString().PadLeft(2,'0')+":"+(timeInterval/60%60).ToString().PadLeft(2,'0')+ ":" + (timeInterval%60).ToString().PadLeft(2, '0');
        }

        private void cropLoad(object sender, RoutedEventArgs e)
        {
            try
            {
                using (StreamReader cropFile = new StreamReader("crop.txt"))
                {
                    while (!cropFile.EndOfStream)
                    {
                      crop.Items.Add(cropFile.ReadLine());
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("File could not be read:");
                Console.WriteLine(ex.Message);
            }
        }

        private void connectClick(object sender, RoutedEventArgs e)
        {
            if (connect == false)
            {
                int plane;
                connect = true;
                con_but.Content = "Disconnect";
                con_but.Foreground = Brushes.Red;
                log_but.IsEnabled = true;
                if (lidarEnable.IsChecked == true)
                {
                    plane = getPlane();
                    lidar_ptr = createLidar(lidarIP.Text, Convert.ToInt32(lidarPort.Text), 2111);
                    Console.WriteLine(plane);
                    if (lidar_conn(lidar_ptr) == 0)
                    {
                        lidar_del(lidar_ptr);
                        return;
                    }
                        
                }
            }
            else
            {
                connect = false;
                con_but.Content = "Connect";
                con_but.Foreground = Brushes.Green;
                if (logging == true)
                {
                    //stop logging
                    logging = false;
                }
                log_but.IsEnabled = false;
                lidar_del(lidar_ptr);
            }
        }

        private void logClick(object sender, RoutedEventArgs e)
        {
            if (fileName.Text == "")
            {
                noFile.Text = "*Please enter file name";
                return;
            }
            else
            {
                noFile.Text = "";
            }

            if (logging == false)
            {
                logging = true;
                log_but.Content = "Stop Log";
                log_but.Foreground = Brushes.Red;
                startTime.Text = DateTime.Now.ToString("h:mm:ss tt");
                timer1.Start();

                using (StreamWriter w = File.AppendText(fileName.Text + ".txt")) {
                    w.WriteLine("<FieldName>" + fieldName.Text);
                    w.WriteLine("<TillageTool>" + tool.Text);
                    w.WriteLine("<Crop>" + crop.Text);
                    w.WriteLine("<RipperDepth(in)>" + rDepth.Text);
                    w.WriteLine("<DiskDepth(in)>" + dDepth.Text);
                    w.WriteLine("<ClosingDepth(in)>" + cdDepth.Text);
                    w.WriteLine("<Basket(psi)>" + basket.Text);
                    w.WriteLine("<Speed(mph)>" + speed.Text);
                    w.WriteLine("<SoilMoisture(%)>" + moisture.Text);
                    w.WriteLine("<Chopping>" + chopping.IsChecked);
                    w.WriteLine("<Comments>" + comment.Text + "<Comments_end>");
                    w.WriteLine("<LogStartTime(UTC)>" + DateTime.Now.ToFileTimeUtc());
                }
             }
            else
            {
                logging = false;
                log_but.Content = "Start Log";
                log_but.Foreground = Brushes.Green;
                timer1.Stop();
                timeInterval = 0;
            }
        }

        private void lidarEna_checked(object sender, RoutedEventArgs e)
        {
            lidar1.IsEnabled = true; lidar2.IsEnabled = true; lidar3.IsEnabled = true; lidar4.IsEnabled = true;
            lidarIP.IsEnabled = true; lidarPort.IsEnabled = true; lidarIPDef_but.IsEnabled = true;
        }

        private void lidarEna_unchecked(object sender, RoutedEventArgs e)
        {
            lidar1.IsEnabled = false; lidar2.IsEnabled = false; lidar3.IsEnabled = false; lidar4.IsEnabled = false;
            lidarIP.IsEnabled = false; lidarPort.IsEnabled = false; lidarIPDef_but.IsEnabled = false;
        }

        private void lidarDef_click(object sender, RoutedEventArgs e)
        {
            lidarIP.Text = lidarIP_Def;
            lidarPort.Text = lidarPort_Def.ToString();
        }

        private void axioEna_checked(object sender, RoutedEventArgs e)
        {
            axioIP.IsEnabled = true; axioPort.IsEnabled = true; axioIPDef_but.IsEnabled = true;
        }

        private void axioEna_unchecked(object sender, RoutedEventArgs e)
        {
            axioIP.IsEnabled = false; axioPort.IsEnabled = false; axioIPDef_but.IsEnabled = false;
        }

        private void axioDef_click(object sender, RoutedEventArgs e)
        {
            axioIP.Text = axioIP_Def;
            axioPort.Text = axioPort_Def.ToString();
        }



        private int getPlane()
        {
            int plane = 0;
            plane = 8 * ((lidar1.IsChecked ?? false) ? 1 : 0);
            plane += 4 * ((lidar2.IsChecked ?? false) ? 1 : 0);
            plane += 2 * ((lidar3.IsChecked ?? false) ? 1 : 0);
            plane += 1 * ((lidar4.IsChecked ?? false) ? 1 : 0);
            return plane;
        }
    }
}
