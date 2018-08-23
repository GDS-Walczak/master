using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ecoflash.Devices;

namespace ecoflash.Models
{
    public class ConfigurationManager
    {
        private static ConfigurationManager instance;

        private Dictionary<DeviceType, string> PortMap = new Dictionary<DeviceType, string>();

        public static ConfigurationManager Shared
        {
            get
            {
                if (instance == null)
                {
                    instance = new ConfigurationManager();
                }
                return instance;
            }
        }

        private ConfigurationManager()
        {
            
        }

        public void SetPort(DeviceType deviceType, string port)
        {
            PortMap[deviceType] = port;
        }

        /// <summary>
        /// Gets the port for a given device
        /// </summary>
        /// <param name="deviceType">The device to lookup</param>
        /// <returns></returns>
        public string GetPort(DeviceType deviceType)
        {
            if (PortMap.ContainsKey(deviceType))
                return PortMap[deviceType];
            else
                return string.Empty;
        }
    }
}
