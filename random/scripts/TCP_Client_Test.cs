using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;

public class NetworkManager1 : MonoBehaviour
{
    public static NetworkManager1 instance;
    public string serverIP = "192.168.0.80"; // サーバーのIPアドレス
    public int port = 49152; // サーバーのポート番号

    private TcpClient client;
    private NetworkStream stream;
    private bool isConnected = false;

    private CancellationTokenSource tokenSource;

    public Transform hmdTransform; // HMDのTransformを指定する

    // シングルトン
    void Awake()
    {
        if (instance == null)
        {
            instance = this;
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            Destroy(gameObject);
        }
    }

    private async void Start()
    {
        // CancellationTokenSourceの生成  
        tokenSource = new CancellationTokenSource();  
        var token = tokenSource.Token;

        ConnectToServer(token);
        await UniTask.WaitForSeconds(1); // 1秒待機して接続が確立するのを待つ

        // HMDの姿勢を定期的に送信
        SendHMDPosePeriodically(token).Forget();
    }

    void OnDestroy()
    {
        DisconnectFromServer();
    }

    // 接続の確立
    public async void ConnectToServer(CancellationToken token = default)
    {
        try
        {
            client = new TcpClient();
            await client.ConnectAsync(serverIP, port);
            stream = client.GetStream();
            isConnected = true;

            // 非同期で受信するスレッドを開始
            await UniTask.RunOnThreadPool(ReceiveData, cancellationToken: token);

            Debug.Log("サーバーに接続しました。");
        }
        catch (Exception e)
        {
            Debug.LogError("サーバーへの接続に失敗しました: " + e.Message);
        }
    }

    // サーバーサイドへメッセージの送信
    public void SendMessageToServer(string message)
    {
        if (isConnected && stream != null)
        {
            try
            {
                byte[] data = Encoding.UTF8.GetBytes(message);
                stream.Write(data, 0, data.Length);
                Debug.Log("メッセージを送信しました: " + message);
            }
            catch (Exception e)
            {
                Debug.LogError("メッセージ送信に失敗しました: " + e.Message);
            }
        }
    }

    // サーバーサイドからのメッセージの受信（非同期）
    private void ReceiveData()
    {
        while (isConnected)
        {
            try
            {
                if (stream != null && stream.DataAvailable)
                {
                    byte[] buffer = new byte[1024];
                    int bytesRead = stream.Read(buffer, 0, buffer.Length);
                    if (bytesRead > 0)
                    {
                        string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                        Debug.Log("サーバーからのメッセージ: " + message);
                    }
                }
            }
            catch (Exception e)
            {
                Debug.LogError("メッセージ受信に失敗しました: " + e.Message);
                isConnected = false;
            }
        }
    }

    // 接続の切断
    public void DisconnectFromServer()
    {
        isConnected = false;

        if (stream != null)
        {
            tokenSource.Cancel();
            stream.Close();
            stream = null;
        }

        if (client != null)
        {
            tokenSource.Cancel();
            client.Close();
            client = null;
        }

        Debug.Log("サーバーとの接続を切断しました。");
    }

    // HMDの姿勢を定期的に送信する
    private async UniTaskVoid SendHMDPosePeriodically(CancellationToken token)
    {
        while (isConnected && !token.IsCancellationRequested)
        {
            if (hmdTransform != null)
            {
                // HMDの位置と回転を取得
                Vector3 position = hmdTransform.position;
                Quaternion rotation = hmdTransform.rotation;

                // 位置と回転を文字列に変換（例: "Position:x,y,z;Rotation:x,y,z,w"）
                string message = $"Position:{position.x:F2},{position.y:F2},{position.z:F2};" +
                                 $"Rotation:{rotation.x:F2},{rotation.y:F2},{rotation.z:F2},{rotation.w:F2}";

                // サーバーに送信
                SendMessageToServer(message);
            }

            await UniTask.Delay(100); // 100msごとに送信
        }
    }
}
