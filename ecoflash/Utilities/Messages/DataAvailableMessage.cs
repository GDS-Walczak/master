using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ecoflash.Utilities.Messages
{
    public class DataAvailableMessage : IMessage
    {

        public DataAvailableMessage()
        {

        }

        public string GetName()
        {
            return "DataAvailableMessage";
        }
    }
}
