using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ecoflash.Devices
{
    public interface IAnalogUSBWriter
    {
        void Initialize();

        void Close();

        void Write(double voltage);
    }
}
