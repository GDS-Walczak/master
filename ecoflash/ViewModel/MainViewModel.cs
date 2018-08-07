using GalaSoft.MvvmLight;
using GalaSoft.MvvmLight.Ioc;
using GalaSoft.MvvmLight.Command;
using System.Windows.Input;
using NLog;

using ecoflash.Models;
using ecoflash.Devices;
using ecoflash.Utilities;
using ecoflash.Utilities.Messages;
using System.Windows;

namespace ecoflash.ViewModel
{
    /// <summary>
    /// This class contains properties that the main View can data bind to.
    /// <para>
    /// Use the <strong>mvvminpc</strong> snippet to add bindable properties to this ViewModel.
    /// </para>
    /// <para>
    /// You can also use Blend to data bind with the tool's support.
    /// </para>
    /// <para>
    /// See http://www.galasoft.ch/mvvm
    /// </para>
    /// </summary>
    public class MainViewModel : ViewModelBase, ISubscriber
    {
        private static Logger logger = LogManager.GetCurrentClassLogger();

        /// <summary>
        /// Initializes a new instance of the MainViewModel class.
        /// </summary>
        public MainViewModel()
        {
            ////if (IsInDesignMode)
            ////{
            ////    // Code runs in Blend --> create design time data.
            ////}
            ////else
            ////{
            ////    // Code runs "for real"
            ////}

            CreateInitCommand();
            CreateStopCommand();
            StatusMessage = "Stopped";
            Messenger.Shared.Subscribe(new UpdateStatusMessage(), this);
        }

        private const string StatusMessagePropertyName = "StatusMessage";
        private string _statusMessage = "";
        public string StatusMessage
        {
            get
            {
                return _statusMessage;
            }
            set
            {
                _statusMessage = value;
                RaisePropertyChanged(StatusMessagePropertyName);
            }
        }

        public void OnMessageReceived(IMessage message)
        {
            var msg = message as UpdateStatusMessage;
            if (msg != null)
            {
                StatusMessage = msg.text;
            }
        }

        public ICommand InitCommand
        {
            get; internal set;
        }

        public string InitIconSource
        {
            get
            {
                return StateManager.Shared.CanInit ? "/Resources/init-icon-enabled.png" : "/Resources/init-icon-disabled.png";
            }
            set
            {
                RaisePropertyChanged("InitIconSource");
            }
        }

        public string InitTest
        {
            get
            {
                return "Init";
            }
        }

        public void InitExecute()
        {
            StateManager.Shared.Initialize();
            
            StatusMessage = "Initializing...";
            InitIconSource = "test";
        }

        private void CreateInitCommand()
        {
            InitCommand = new RelayCommand(InitExecute, StateManager.Shared.CanInit);
        }




        public ICommand StopCommand
        {
            get; internal set;
        }

        private void CreateStopCommand()
        {
            StopCommand = new RelayCommand(StopExecute, StateManager.Shared.CanStop);
        }

        public void StopExecute()
        {
            IAnalogUSBReader dataq = SimpleIoc.Default.GetInstance<IAnalogUSBReader>();
            dataq.Close();

            StatusMessage = "Stopped";
        }
    }
}