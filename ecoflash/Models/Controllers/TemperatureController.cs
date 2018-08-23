using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GalaSoft.MvvmLight.Ioc;
using NLog;
using ecoflash.Devices;

namespace ecoflash.Models.Controllers
{
    public class TemperatureController : IPIDController
    {
        
        private Task taskRun = null;
        private static Logger logger = LogManager.GetCurrentClassLogger();

        private List<double> readings = new List<double>();
        private bool cancel = false;
        private IAnalogUSBWriter analogUSBWriter = null;

        private double kp = 0.3;

        private double _setPoint = 0.0;
        public double SetPoint
        {
            get { return _setPoint; }
            set { _setPoint = value; }
        }

        public TemperatureController()
        {
            //taskRun = Run();
            analogUSBWriter = SimpleIoc.Default.GetInstance<IAnalogUSBWriter>();
        }

        public void Start()
        {
            logger.Info("Starting Temperature control with setpoint " + SetPoint);

            IAnalogUSBReader dataq = SimpleIoc.Default.GetInstance<IAnalogUSBReader>();
            dataq.DataReceived += onDataReceived;

            Run();
        }

        public void Stop()
        {
            cancel = true;
        }

        private void onDataReceived(double reading)
        {
            if (readings.Count < 1000)
                readings.Add(reading);
            else
            {
                readings.RemoveAt(0);
                readings.Add(reading);
            }
        }

        private async Task Run()
        {
            int iter = 0;
            
            while (true)
            {
                iter += 1;
                if (cancel || iter == 3000)
                    break;

                try
                {
                    if (readings.Count > 0)
                    {
                        double lastReading = readings.Last();

                        double error = SetPoint - lastReading;
                        double output = kp * error;

                        // cap the output.
                        if (output > 9.9)
                            output = 9.9;

                        // stop the heater
                        if (output < 0)
                            output = 1.0;


                        analogUSBWriter.Write(output);

                        if (iter % 10 == 0)
                            logger.Info("PID: setpoint: " + SetPoint.ToString("N1") + ", temp: " + lastReading.ToString("N1"));
                    }
                    
                }
                catch (Exception ex)
                {
                    logger.Warn("Warning: " + ex.Message);    
                }

                await Task.Delay(1000);
            }

            
        }

        
    }
}
