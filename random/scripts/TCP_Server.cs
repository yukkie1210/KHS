using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;

public class TCP_Server : MonoBehaviour
{
    private TcpListener server;
    private Thread serverThread;
    private bool isRunning = false;

    private List<TcpClient> clients = new List<TcpClient>(); // 接続中のクライアントを管理
    private object clientLock = new object(); // クライアントリストの同期用

    private GetHMDPose getHMDPose;

    void Start()
    {
        // GetHMDPoseコンポーネントを取得
        getHMDPose = GetComponent<GetHMDPose>();
        if (getHMDPose == null)
        {
            Debug.LogError("GetHMDPose component is not attached to this GameObject.");
            return;
        }

        // サーバーを開始
        StartServer();
    }

    void OnDestroy()
    {
        StopServer();
    }

    private void StartServer()
    {
        try
        {
            // server = new TcpListener(IPAddress.Parse("127.0.0.1"), 12345); 
            server = new TcpListener(IPAddress.Any, 46963); // ポート番号12345
            server.Start();
            isRunning = true;

            serverThread = new Thread(ServerLoop);
            serverThread.Start();

            Debug.Log("TCP Server started on port 12345.");
        }
        catch (Exception e)
        {
            Debug.LogError("Failed to start TCP server: " + e.Message);
        }
    }

    private void StopServer()
    {
        isRunning = false;

        if (server != null)
        {
            server.Stop();
            server = null;
        }

        if (serverThread != null && serverThread.IsAlive)
        {
            serverThread.Abort();
            serverThread = null;
        }

        lock (clientLock)
        {
            foreach (var client in clients)
            {
                client.Close();
            }
            clients.Clear();
        }

        Debug.Log("TCP Server stopped.");
    }

    private void ServerLoop()
    {
        while (isRunning)
        {
            try
            {
                // 新しいクライアントの接続を待機
                var client = server.AcceptTcpClient();
                lock (clientLock)
                {
                    clients.Add(client);
                }
                Debug.Log("Client connected.");

                // クライアントごとに新しいスレッドを作成
                Thread clientThread = new Thread(() => HandleClient(client));
                clientThread.Start();
            }
            catch (SocketException e)
            {
                Debug.LogWarning("Socket exception: " + e.Message);
            }
        }
    }

    private void HandleClient(TcpClient client)
    {
        try
        {
            using (var stream = client.GetStream())
            {
                while (client.Connected && isRunning)
                {
                    // GetHMDPoseから位置と回転を取得
                    Vector3 position;
                    Quaternion rotation;

                    if (getHMDPose.GetPose(out position, out rotation))
                    {
                        // データを文字列に変換して送信
                        string data = $"{position.x},{position.y},{position.z},{rotation.x},{rotation.y},{rotation.z},{rotation.w}";
                        byte[] buffer = Encoding.UTF8.GetBytes(data);

                        stream.Write(buffer, 0, buffer.Length);
                        stream.Flush();

                        Debug.Log("Sent data to client: " + data);
                    }

                    // 50msの遅延（20FPS）
                    Thread.Sleep(50);
                }
            }
        }
        catch (Exception e)
        {
            Debug.LogWarning("Client connection error: " + e.Message);
        }
        finally
        {
            lock (clientLock)
            {
                clients.Remove(client);
            }
            client.Close();
            Debug.Log("Client disconnected.");
        }
    }
}

/*using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

namespace Script.TCP_Server
{
    public class TCP_Server : MonoBehaviour {
        private TcpListener _listener;
        private readonly List<TcpClient> _clients = new List<TcpClient>();

        // ソケット接続準備、待機
        protected void Listen(string host, int port){
            Debug.Log("ipAddress:"+host+" port:"+port);
            var ip = IPAddress.Parse(host);
            _listener = new TcpListener(ip, port);
            _listener.Start();
            _listener.BeginAcceptSocket(DoAcceptTcpClientCallback, _listener);
        }

        // クライアントからの接続処理
        private void DoAcceptTcpClientCallback(IAsyncResult ar) {
            var listener = (TcpListener)ar.AsyncState;
            var client = listener.EndAcceptTcpClient(ar);
            _clients.Add(client);
            Debug.Log("Connect: "+client.Client.RemoteEndPoint);

            // 接続が確立したら次の人を受け付ける
            listener.BeginAcceptSocket(DoAcceptTcpClientCallback, listener);

            // 今接続した人とのネットワークストリームを取得
            var stream = client.GetStream();
            var reader = new StreamReader(stream,Encoding.UTF8);

            // 接続が切れるまで送受信を繰り返す
            while (client.Connected) {
                while (!reader.EndOfStream){
                    // 一行分の文字列を受け取る
                    var str = reader.ReadLine ();
                    OnMessage(str);
                }

                // クライアントの接続が切れたら
                if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0)) {
                    Debug.Log("Disconnect: "+client.Client.RemoteEndPoint);
                    client.Close();
                    _clients.Remove(client);
                    break;
                }
            }
        }


        // メッセージ受信
        protected virtual void OnMessage(string msg){
            Debug.Log(msg);
        }

        // クライアントにメッセージ送信
        protected void SendMessageToClient(string msg){
            if (_clients.Count == 0){
                return;
            }
            var body = Encoding.UTF8.GetBytes(msg);

            // 全員に同じメッセージを送る
            foreach(var client in _clients){
                try{
                    var stream = client.GetStream();
                    stream.Write(body, 0, body.Length);
                }catch {
                    _clients.Remove(client);
                }
            }
        }

        // 終了処理
        protected virtual void OnApplicationQuit() {
            if (_listener == null){
                return;
            }

            if (_clients.Count != 0){
                foreach(var client in _clients){
                    client.Close();
                }
            }
            _listener.Stop();
        }
    }
} */