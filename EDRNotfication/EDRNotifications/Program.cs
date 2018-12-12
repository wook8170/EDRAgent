using System;
using System.Windows.Forms;
using System.Text;
using System.Threading;
using ToastNotifications;

namespace EDRNotificator
{
    class Program
    {
        static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Application.Run(new NotificationLauncher(
                5
                , (int)FormAnimator.AnimationMethod.Slide
                , (int)FormAnimator.AnimationDirection.Left
                , "normal"
            ));

            do
            {
                Thread.Sleep(1000);
            } while (true);
        }

    }
}


