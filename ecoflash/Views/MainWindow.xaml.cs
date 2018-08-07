using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Media.Animation;
using NLog;
using Dataq;
using Dataq.Misc;
using LibUsbDotNet;
using LibUsbDotNet.Main;
using System.Text.RegularExpressions;

using ecoflash.Devices;
using ecoflash.Models;
using ecoflash.ViewModel;
using ecoflash.Utilities;
using ecoflash.Utilities.Messages;

namespace ecoflash
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        private static Logger logger = LogManager.GetCurrentClassLogger();

        public MainWindow()
        {
            InitializeComponent();
            Closing += (s, e) => ViewModelLocator.Cleanup();

            logger.Debug("Starting up");

            // TODO this should be relative.
            dashboard.Height = 800;
        }

        
        private void btnClose_Click(object sender, RoutedEventArgs e)
        {
            var viewModel = (MainViewModel) DataContext;
            viewModel.StopExecute();
            
            logger.Debug("Closing");

            Close();
        }


    }
}
