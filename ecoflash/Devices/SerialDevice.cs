using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.IO.Ports;

namespace ecoflash.Devices
{
    /// <summary>
    /// For mocking the SerialPort
    /// </summary>
    public interface ISerialDevice
    {

        void Open();

        void Close();

        void SetPort(string comPort);

        void WriteLine(string value);

        string ReadLine();
    }


    public class SerialDevice: ISerialDevice
    {

        private SerialPort serialPort = new SerialPort();

        public SerialDevice()
        {
            
        }

        public void Close()
        {
            serialPort.Close();
        }

        public void Open()
        {
            serialPort.Open();
        }

        public string ReadLine()
        {
            return serialPort.ReadLine();
        }

        public void SetPort(string comPort)
        {
            serialPort.PortName = comPort;
        }

        public void WriteLine(string value)
        {
            serialPort.WriteLine(value);
        }
    }

}
