using System;
using System.Collections.Generic;
using System.Net;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.XR;


namespace UDP
{
    public class UdpTest : MonoBehaviour
    {
        [SerializeField]
        private string _sendData = "test";
        [SerializeField]
        private List<string> address;
        [SerializeField]
        private int receivePort = 9000;
        [SerializeField]
        private int sendPort = 9001;
        
        private UDPServer _udp;
        private List<IPEndPoint> _ipEndPoints = new List<IPEndPoint>();

        private void Start()
        {
            _udp = new UDPServer(receivePort);
            
            var token = this.GetCancellationTokenOnDestroy();
            WaitReceive(token).Forget();

            for (int i = 0; i < address.Count; i++)
            {
                _ipEndPoints.Add(new IPEndPoint(IPAddress.Parse(address[i]), sendPort));
            }
        }

        private void Update()
        {
            if (Keyboard.current.digit0Key.wasPressedThisFrame)
            {
                _udp.Send(_sendData,_ipEndPoints[0]).Forget();
            }
            if (Keyboard.current.digit1Key.wasPressedThisFrame)
            {
                _udp.Send(_sendData,_ipEndPoints[1]).Forget();
            }
            if (Keyboard.current.digit2Key.wasPressedThisFrame)
            {
                _udp.Send(_sendData,_ipEndPoints[2]).Forget();
            }
            if (OVRInput.GetDown(OVRInput.Button.One))
            {
                _udp.Send(_sendData,_ipEndPoints[2]).Forget();
            }
        }
        
        async UniTaskVoid WaitReceive(CancellationToken token)
        {
            while (!token.IsCancellationRequested)
            {
                string data = await _udp.Receive(token);
                Debug.Log(data);
                //await UniTask.Delay(TimeSpan.FromSeconds(3), cancellationToken: token);
            }
        }
    }
}