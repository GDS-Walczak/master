using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Configuration;
using System.Threading.Tasks;

using NLog;
using LibUsbDotNet.Main;

using Dataq.Devices;
//using Dataq.Devices.DI245;
//using Dataq.Devices.DI1110;
using Dataq.Devices.DI2008;

using ecoflash.Utilities;
using ecoflash.Utilities.Messages;


namespace ecoflash.Devices
{
    public delegate void OnDataReceived(double reading);

    public interface IAnalogUSBReader
    {
        void Initialize();

        void Close();

        event OnDataReceived DataReceived;
    }


    public class DATAQManager: IAnalogUSBReader
    {
        private static Logger logger = LogManager.GetCurrentClassLogger();

        private Device device = null;
        private string deviceModel = "DI-2008";

        private CancellationTokenSource cancelRead = null;
        private Task taskRead = null;

        
        public event OnDataReceived DataReceived;

        public DATAQManager()
        {

        }

        public async void Initialize()
        {
            ErrorCode ec = ErrorCode.None;

            try
            {
                var deviceList = await Dataq.Misc.Discovery.AllDevicesAsync();
                if (deviceList.Count == 1 && deviceList[0].Model == deviceModel)
                {
                    logger.Debug("found device" + deviceModel);
                    device = deviceList[0] as Device;

                    await device.ConnectAsync();
                    await device.AcquisitionStopAsync();
                    await device.QueryDeviceAsync();

                    if (device.IsConnected)
                    {
                        logger.Debug("Connected to " + device.Model);
                        logger.Debug("Serial number: " + device.Serial);

                        device.Channels.Clear();

                        //var analogChannel = device.ChannelFactory(typeof(AnalogVoltageIn), 1) as AnalogVoltageIn;
                        var analogChannel = device.ChannelFactory(typeof(ThermocoupleIn), 1) as ThermocoupleIn;
                        analogChannel.ThermocoupleType = new ThermocoupleTypeT();
                        analogChannel.AcquisitionMode = new AverageSample();
                        analogChannel.IsExceptionsInEnabled = true;
                        device.SetSampleRateOnChannels(1000);

                        // Initialize the device with channel configuration and sample rate
                        await device.InitializeAsync();
                        cancelRead = new CancellationTokenSource();
                        await device.AcquisitionStartAsync();

                        taskRead = StartReading();
                        //taskRead.Start();
                        //await taskRead;
                    }
                    else
                    {
                        Messenger.Shared.Publish(new UpdateStatusMessage("Not connected to DATAQ"));
                    }
                } else
                {
                    Messenger.Shared.Publish(new UpdateStatusMessage("Not connected to DATAQ"));
                }
            }
            catch (Exception ex)
            {
                var errorText = (ec != ErrorCode.None ? ec + ":" : String.Empty) + ex.Message;
                logger.Error(errorText);
                Messenger.Shared.Publish(new UpdateStatusMessage(errorText));
            }
        }

        private async Task StartReading()
        {
            var channel = device.Channels[0] as IChannelIn;
            double sum = 0;
            double average = 0;
            
            while(device.IsAcquiring)
            {

                try
                {
                    await device.ReadDataAsync(cancelRead.Token);

                    if (channel.DataIn.Count == 0)
                    {
                        continue;
                    }
                    sum = 0;
                    average = 0;

                    for(int i = 0; i < channel.DataIn.Count; i++)
                    {
                        double value = channel.DataIn[i];
                        sum += value;
                    }

                    average = sum / channel.DataIn.Count;

                    DataReceived?.Invoke(average);

                }
                catch(OperationCanceledException ex)
                {
                    logger.Error("DATAQ stopped acquiring data: " + ex.StackTrace);
                    Messenger.Shared.Publish(new UpdateStatusMessage("DATAQ stopped acquiring data"));
                    break;
                }
            }

            logger.Debug("Done acquiring");
        }

        public void Close()
        {
            if (taskRead != null && taskRead.Status == TaskStatus.Running)
            {
                cancelRead.Cancel();
            }

            if (device != null)
            {
                device.Dispose();
            }
        }
    }
}
