using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Ioc;
using GalaSoft.MvvmLight.Command;
using System.Windows.Input;
using NLog;

using ecoflash.Models;
using ecoflash.Devices;
using ecoflash.Utilities;
using ecoflash.Utilities.Messages;
using System.Windows;

namespace ecoflash.ViewModel
{
    public class SideDrawerViewModel : ViewModelBase, ISubscriber
    {
        private static Logger logger = LogManager.GetCurrentClassLogger();

        public SideDrawerViewModel()
        {
            IAnalogUSBReader dataq = SimpleIoc.Default.GetInstance<IAnalogUSBReader>();
            dataq.DataReceived += onAnalogDataReceived;

            PreGLSTemp = 0;
        }

        private double _preGLSTemperature = 0;
        public double PreGLSTemp
        {
            get { return _preGLSTemperature; }
            set { _preGLSTemperature = value; RaisePropertyChanged("PreGLSTemp"); }
        }

        private void onAnalogDataReceived(double reading)
        {
            // TODO this should use a converter
            //PreGLSTemp = reading.ToString("N1") + " °C";
            PreGLSTemp = reading;
        }

        public void OnMessageReceived(IMessage message)
        {

        }
    }
}
