using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using NLog;
using MccDaq;

namespace ecoflash.Devices
{
    public class OmegaManager : IAnalogUSBWriter
    {
        private static Logger logger = LogManager.GetCurrentClassLogger();

        private MccBoard board;
        private VOutOptions options = VOutOptions.Default;

        public OmegaManager()
        {
        }

        public void Close()
        {

        }

        public void Initialize()
        {
            MccDaq.ErrorInfo ULStat = MccDaq.MccService.ErrHandling(MccDaq.ErrorReporting.PrintAll, MccDaq.ErrorHandling.StopAll);
            
            board = new MccDaq.MccBoard(0);
        }

        public void Write(double voltage)
        {
            
            ErrorInfo result = board.VOut(0, Range.Bip10Volts, (float)voltage, options);
            ErrorInfo.ErrorCode code = result.Value;

            if (code != ErrorInfo.ErrorCode.NoErrors)
            {
                logger.Error("Omega Error: " + result.Message);
            }
        }
    }
}
