using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ecoflash.Devices;
using GalaSoft.MvvmLight.Ioc;

namespace ecoflash.Models
{
    public enum States
    {
        Idle,
        Initializing,
        Initialized,
        Running,
        Stopped,
        Flushing,
        Error,
    };


    public class StateManager
    {
        private static StateManager instance;

        public States State
        {
            get; private set;
        }

        private StateManager()
        {
            State = States.Idle;
        }

        public static StateManager Shared
        {
            get
            {
                if (instance == null)
                {
                    instance = new StateManager();
                }
                return instance;
            }
        }


        public void Initialize()
        {
            if (State == States.Idle)
            {
                // Intialize Comms with DATAQ
                IAnalogUSBReader dataq = SimpleIoc.Default.GetInstance<IAnalogUSBReader>();
                dataq.Initialize();

                State = States.Initializing;
            }

        }

        public void Start()
        {

        }

        public void Stop()
        {

        }

        public bool CanStop
        {
            get
            {

                switch (State)
                {
                    case States.Stopped:
                    case States.Idle:
                    case States.Error:
                        return false;

                    default:
                        return true;
                }
            }
        }

        public bool CanStart
        {
            get
            {
                switch (State)
                {
                    case States.Initialized:
                        return true;

                    default:
                        return false;
                }
            }
        }

        public bool CanInit
        {
            get
            {

                switch (State)
                {
                    case States.Stopped:
                    case States.Idle:
                    case States.Error:
                        return true;

                    default:
                        return false;
                }
            }
        }
    }
}
