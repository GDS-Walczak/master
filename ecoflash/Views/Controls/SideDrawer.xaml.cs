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

using ecoflash.Devices;

namespace ecoflash.Views.Controls
{
    /// <summary>
    /// Interaction logic for sideDrawer.xaml
    /// </summary>
    public partial class SideDrawer : UserControl
    {
        public SideDrawer()
        {
            InitializeComponent();

            btnSettingsBack.Visibility = Visibility.Hidden;

            //DATAQManager.Shared.DataReceived += OnDataReceived;
        }

        private void OnDataReceived(double reading)
        {
            lblTempPreGLS.Content = reading.ToString("N1") + " °C";
        }

        private void ShowHideMenu(string Storyboard, Button btnHide, Button btnShow, StackPanel pnl)
        {
            Storyboard sb = Resources[Storyboard] as Storyboard;
            sb.Begin(pnl);

            if (Storyboard.Contains("Show"))
            {
                btnHide.Visibility = System.Windows.Visibility.Visible;
                btnShow.Visibility = System.Windows.Visibility.Collapsed;
            }
            else if (Storyboard.Contains("Hide"))
            {
                btnHide.Visibility = System.Windows.Visibility.Collapsed;
                btnShow.Visibility = System.Windows.Visibility.Visible;
            }
        }

        private void btnLeftMenuHide_Click(object sender, RoutedEventArgs e)
        {
            ShowHideMenu("sbHideLeftMenu", btnLeftMenuHide, btnLeftMenuShow, pnlLeftMenu);
        }

        private void btnLeftMenuShow_Click(object sender, RoutedEventArgs e)
        {
            ShowHideMenu("sbShowLeftMenu", btnLeftMenuHide, btnLeftMenuShow, pnlLeftMenu);
        }

        private void btnReports_Click(object sender, RoutedEventArgs e)
        {
            settingsUnderline.Visibility = Visibility.Hidden;
            reportsUnderline.Visibility = Visibility.Visible;
        }

        private void btnSettings_Click(object sender, RoutedEventArgs e)
        {
            settingsUnderline.Visibility = Visibility.Visible;
            reportsUnderline.Visibility = Visibility.Hidden;
        }

        private void btnSettingsNext_Click(object sender, RoutedEventArgs e)
        {
            if (settingsControls1.Visibility == Visibility.Visible)
            {
                settingsControls1.Visibility = Visibility.Collapsed;
                settingsControls2.Visibility = Visibility.Visible;
                btnSettingsBack.Visibility = Visibility.Visible;
                btnSettingsNext.Visibility = Visibility.Hidden;

                // set page indicators
                pageIndicatorCircle1.Fill = new SolidColorBrush(System.Windows.Media.Colors.Transparent);
                pageIndicatorCircle2.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#aa000000"));
            }
        }

        private void btnSettingsBack_Click(object sender, RoutedEventArgs e)
        {
            if (settingsControls2.Visibility == Visibility.Visible)
            {
                settingsControls2.Visibility = Visibility.Collapsed;
                settingsControls1.Visibility = Visibility.Visible;
                btnSettingsBack.Visibility = Visibility.Hidden;
                btnSettingsNext.Visibility = Visibility.Visible;

                pageIndicatorCircle2.Fill = new SolidColorBrush(System.Windows.Media.Colors.Transparent);
                pageIndicatorCircle1.Fill = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#aa000000"));
            }
        }
    }
}
