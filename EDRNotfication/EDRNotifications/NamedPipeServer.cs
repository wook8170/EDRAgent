﻿using System;
using Microsoft.Win32.SafeHandles;
using System.Text;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;
using ToastNotifications;
using System.Windows.Forms;

namespace EDRNotificator
{
    public class NamedPipeServer
    {

        NotificationLauncher notificationLauncher;

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern SafeFileHandle CreateNamedPipe(
           String pipeName,
           uint dwOpenMode,
           uint dwPipeMode,
           uint nMaxInstances,
           uint nOutBufferSize,
           uint nInBufferSize,
           uint nDefaultTimeOut,
           IntPtr lpSecurityAttributes);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern int ConnectNamedPipe(
           SafeFileHandle hNamedPipe,
           IntPtr lpOverlapped);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern int DisconnectNamedPipe(
           SafeFileHandle hNamedPipe);

        public const uint DUPLEX = (0x00000003);
        public const uint FILE_FLAG_OVERLAPPED = (0x40000000);

        public class Client
        {
            public SafeFileHandle handle;
            public FileStream stream;
        }

        public const int BUFFER_SIZE = 2048;
        public Client clientse =null;

        public string pipeName;
        Thread listenThread;
        SafeFileHandle clientHandle;
        public int ClientType;

        public void StartPipeServer()
        {
            while (true)
            {
                clientHandle = CreateNamedPipe(this.pipeName, DUPLEX | FILE_FLAG_OVERLAPPED, 0, 255, BUFFER_SIZE, BUFFER_SIZE, 0, IntPtr.Zero);

                //could not create named pipe
                if (clientHandle.IsInvalid)
                    return;

                int success = ConnectNamedPipe(clientHandle, IntPtr.Zero);

                //could not connect client
                if (success == 0)
                    return;

                clientse = new Client();
                clientse.handle = clientHandle;
                clientse.stream = new FileStream(clientse.handle, FileAccess.ReadWrite, BUFFER_SIZE, true);

                if (ClientType == 0)
                {
                    if (ClientType == 0)
                    {
                        Read();
                    }
                }
            }
        }

        
        public NamedPipeServer(string PName,int Mode)
        {
            pipeName = PName;
            ClientType = Mode;//0 Reading Pipe, 1 Writing Pipe

        }
       
        public void Start(NotificationLauncher notiLauncher)
        {
            notificationLauncher = notiLauncher;

            Thread readThread = new Thread(new ThreadStart(StartPipeServer));
            readThread.Start();
        }

        private void ListenForClients()
        {
            while (true)
            {

                clientHandle =CreateNamedPipe(this.pipeName,DUPLEX | FILE_FLAG_OVERLAPPED,0,255,BUFFER_SIZE,BUFFER_SIZE,0,IntPtr.Zero);

                //could not create named pipe
                if (clientHandle.IsInvalid)
                    return;

                int success = ConnectNamedPipe(clientHandle, IntPtr.Zero);

                //could not connect client
                if (success == 0)
                    return;

                clientse = new Client();
                clientse.handle = clientHandle;
                clientse.stream = new FileStream(clientse.handle, FileAccess.ReadWrite, BUFFER_SIZE, true);

                if (ClientType == 0)
                {
                    Thread readThread = new Thread(new ThreadStart(Read));
                    readThread.Start();
                }
            }
        }

        public delegate void ShowEDRNotifaction(string title, string message);
        public ShowEDRNotifaction delegationContain;

        private void Read()
        {
            byte[] buffer = null;

            ASCIIEncoding encoder = new ASCIIEncoding();
                        
            while (true)
            {
                
                int bytesRead = 0;

                try
                {
                    buffer = new byte[BUFFER_SIZE];
                    bytesRead = clientse.stream.Read(buffer, 0, BUFFER_SIZE);
                }
                catch
                {
                    //read error has occurred
                    break;
                }

                //client has disconnected
                if (bytesRead == 0)
                    break;

                //fire message received event
                int ReadLength = 0;
                ///*
                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    if (/*buffer[i].ToString("x2") != "cc" && */buffer[i].ToString("x2") != "00")
                    {
                        ReadLength++;
                    }
                    else
                        break;
                }
                //*/
                if (ReadLength > 0)
                {
                    byte[] Rc = new byte[ReadLength];
                    Buffer.BlockCopy(buffer, 0, Rc, 0, ReadLength);

                    buffer.Initialize();

                    //Console.WriteLine("C# App: Received " + ReadLength +" Bytes: "+ encoder.GetString(Rc, 0, ReadLength));
                    //string phrase = System.Text.Encoding.UTF8.GetString(Rc);// encoder.GetString(Rc, 0, ReadLength);
                    //string phrase = System.Text.Encoding.Default.GetString(Rc);// encoder.GetString(Rc, 0, ReadLength);
                    string phrase = System.Text.Encoding.GetEncoding("euc-kr").GetString(Rc);// encoder.GetString(Rc, 0, ReadLength);
                    string[] words = phrase.Split('|');

                    delegationContain = new ShowEDRNotifaction(AddNotification);

                    notificationLauncher.Invoke(delegationContain, words[0], words[1]);

                }
               
            }

            //clean up resources
            clientse.stream.Close();
            clientse.handle.Close();
            
        }

        ///*
        public void AddNotification(string title, string message)
        {
            notificationLauncher.ShowNotification(title, message);
        }
        //*/
        public void SendMessage(string message, Client client)
        {

                ASCIIEncoding encoder = new ASCIIEncoding();
                byte[] messageBuffer = encoder.GetBytes(message);

                if (client.stream.CanWrite)
                {
                    client.stream.Write(messageBuffer, 0, messageBuffer.Length);
                    client.stream.Flush();
                }


        }
        public void StopServer()
        {
            //clean up resources

            DisconnectNamedPipe(this.clientHandle);
            

            this.listenThread.Abort();
        }

    }
}
