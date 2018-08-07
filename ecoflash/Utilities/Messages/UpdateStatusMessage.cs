using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ecoflash.Utilities.Messages
{
    public class UpdateStatusMessage : IMessage
    {
        public string text = "";

        public UpdateStatusMessage() { }

        public UpdateStatusMessage(string text)
        {
            this.text = text;
        }

        public string GetName()
        {
            return "UpdateStatusMessage";
        }
    }
}
