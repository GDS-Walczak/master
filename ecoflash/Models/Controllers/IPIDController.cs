using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ecoflash.Models.Controllers
{
    public interface IPIDController
    {
        double SetPoint
        {
            get; set;
        }

        void Start();

        void Stop();
    }
}
