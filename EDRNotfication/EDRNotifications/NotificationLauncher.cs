using System;
using System.IO;
using System.Windows.Forms;
using System.Threading;
using EDRNotificator;
namespace ToastNotifications
{
    public partial class NotificationLauncher : Form
    {
        private bool _initialLoad = true;

        int m_duration;
        FormAnimator.AnimationMethod m_animationMethod;
        FormAnimator.AnimationDirection m_animationDirection;
        string m_strsound;

        public NotificationLauncher(
            int duration
            , int animation
            , int direction
            , string sound
            )
        {
            this.Opacity = 0.0f;
            this.ShowInTaskbar = false;

            m_duration = duration;
            m_animationMethod = (FormAnimator.AnimationMethod)animation;
            m_animationDirection = (FormAnimator.AnimationDirection)direction;
            m_strsound = sound;

            NamedPipeServer PServer1 = new NamedPipeServer(@"\\.\pipe\EDRNotificator1", 0);

            PServer1.Start(this);
        }

        /*
        private void PopulateComboBoxes()
        {
            foreach (FormAnimator.AnimationMethod method in Enum.GetValues(typeof(FormAnimator.AnimationMethod)))
            {
                comboBoxAnimation.Items.Add(method.ToString());
            }
            comboBoxAnimation.SelectedIndex = 2;

            foreach (FormAnimator.AnimationDirection direction in Enum.GetValues(typeof(FormAnimator.AnimationDirection)))
            {
                comboBoxAnimationDirection.Items.Add(direction.ToString());
            }
            comboBoxAnimationDirection.SelectedIndex = 3;

            var soundsFolder = new DirectoryInfo(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Sounds"));
            foreach (var file in soundsFolder.GetFiles())
            {
                comboBoxSound.Items.Add(Path.GetFileNameWithoutExtension(file.FullName));
            }
            comboBoxSound.SelectedIndex = 5;
            _initialLoad = false;

            comboBoxDuration.SelectedIndex = 0;
        }
        //*/
       
        public void ShowNotification(string title, string message)
        {
            var toastNotification = new Notification(
                title
                , message
                , m_duration
                , m_animationMethod
                , m_animationDirection
            );
            PlayNotificationSound(m_strsound);
            toastNotification.Show();
        }
        
        /*
        private void buttonShowNotification_Click(object sender, EventArgs e)
        {
            //ShowNotification();
        }
        //*/

        private static void PlayNotificationSound(string sound)
        {
            var soundsFolder = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Sounds");
            var soundFile = Path.Combine(soundsFolder, sound + ".wav");

            using (var player = new System.Media.SoundPlayer(soundFile))
            {
                player.Play();
            }
        }

        /*
        private void comboBoxSound_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!_initialLoad)
            {
                PlayNotificationSound(comboBoxSound.Text);
            }
        }
        //*/
    }
}
