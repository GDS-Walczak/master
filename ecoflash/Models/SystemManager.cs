using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using GalaSoft.MvvmLight.Ioc;

using ecoflash.Devices;

namespace ecoflash.Models
{
    public class SystemManager
    {
        private static SystemManager instance;
        private bool didStart = false;

        private SystemManager()
        {
            
        }

        public static SystemManager Shared
        {
            get
            {
                if (instance == null)
                {
                    instance = new SystemManager();
                }
                return instance;
            }
        }

        /// <summary>
        /// Registers Dependencies. Should only be called once!
        /// </summary>
        public void Start()
        {
            if (!didStart)
            {
                didStart = true;

                // Register dependencies
                SimpleIoc.Default.Register<IAnalogUSBReader, DATAQManager>();
            }
        }
    }
}
