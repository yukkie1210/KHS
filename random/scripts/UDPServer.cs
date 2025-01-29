using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;

namespace UDP
{
    public class UDPServer
    {
        private UdpClient udpClient;

        public UDPServer(int port)
        {
            udpClient = new UdpClient(port);
        }
        
        public async UniTaskVoid Send(string data, IPEndPoint endPoint, CancellationToken token = default)
        {
            byte[] message = Encoding.UTF8.GetBytes(data);
            await udpClient.SendAsync(message, message.Length, endPoint);
            Debug.Log("Send:\"" + data + "\" To:" + endPoint.Address + "," + endPoint.Port);
        }
        
        public async UniTask<string> Receive(CancellationToken token = default)
        {
            var result = await udpClient.ReceiveAsync();
            IPEndPoint endPoint = result.RemoteEndPoint;
            Debug.Log("Receive:" + endPoint.Address + "," + endPoint.Port);
            byte[] getByte = result.Buffer;
            string data = Encoding.UTF8.GetString(getByte);
            return data;
        }
    }
}