using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ecoflash.DataEntities
{
    public class Procedure
    {
        Double preGLSHeaterSetPoint = 50.0;         // pre-GLS heater set point in C
        Double afConHeaterSetPoint = 50.0;          // af-con heater set point in C
        Double heaterWarmupTimeout = 300;           // amount of time to wait for heater to warm up.
        Double stage1CO2Pressure = 5;               // Stage one CO2 pressure in Bar.
        Double stage1CO2FlowRate = 5;               // Stage one liquid CO2 flow rate in mLPM.
        Double stage1Duration = 10;                 // Stage one duration in seconds.
        Double stage2CO2Pressure = 10;              // Stage two CO2 pressure in Bar.
        Double stage2CO2FlowRate = 10;              // Stage two CO2 flow rate in mLPM.
        Double stage2Duration = 15;                 // Stage two duration.
        Double stableFlowTimeout = 600;             // Timeout that occurs if flow/pressure has not stabilized.
        Double depressurizationTimeout = 480;       // Timeout that occurs if depressurization fails.
        Double depressurizationTarget = 0.3;        // Depressurization target in Bar.

        public Procedure()
        {

        }
    }
}
