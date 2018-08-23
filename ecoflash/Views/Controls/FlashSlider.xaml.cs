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

using ecoflash.Views.Converters;
using NLog;

using ecoflash.Models.Controllers;
using GalaSoft.MvvmLight.Ioc;

namespace ecoflash.Views.Controls
{
    public delegate void OnSetPointChanged(double newSetPoint);

    /// <summary>
    /// Interaction logic for FlashSlider.xaml
    /// </summary>
    public partial class FlashSlider : UserControl
    {
        public event OnSetPointChanged SetPointChanged;

        private static Logger logger = LogManager.GetCurrentClassLogger();
        private DoubleToCelciusConverter tempConverter = new DoubleToCelciusConverter();

        private IPIDController tempController;

        public FlashSlider()
        {
            InitializeComponent();

            
        }

        private void onSliderChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if(lblSliderSetPoint != null)
            {
                //lblSliderSetPoint.Content = tempConverter.Convert(e.NewValue, e.GetType(), null, System.Globalization.CultureInfo.CurrentCulture);
                lblSliderSetPoint.Text = (string)tempConverter.Convert(e.NewValue, e.GetType(), null, System.Globalization.CultureInfo.CurrentCulture);
            }

            SetPointChanged?.Invoke(e.NewValue);

            IPIDController tempController = SimpleIoc.Default.GetInstance<IPIDController>();
            if (tempController != null)
                tempController.SetPoint = e.NewValue;
        }

        private void onSetPointChangedManually(object sender, TextChangedEventArgs e)
        {
            double setPoint = 0.0;

            try
            {
                string value = lblSliderSetPoint.Text;

                if (value.Contains(" "))
                {
                    value = value.Split(' ')[0];

                    double.TryParse(value, out setPoint);

                    if (setPoint > 0 && setPoint < 150)
                    {
                        slider.Value = setPoint;
                    }
                }
            } catch (Exception ex)
            {
                logger.Warn(ex.Message);
            }
        }
    }
}
