using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;

public class TCP_Client : MonoBehaviour
{
    public static TCP_Client instance;
    public string serverIP = "10.221.229.66"; // サーバーのIPアドレス
    public int port = 46963; // サーバーのポート番号

    private TcpClient client;
    private NetworkStream stream;
    private bool isConnected = false;

    private CancellationTokenSource tokenSource;

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

        await ConnectToServer(token);

        // サーバーに接続後すぐメッセージ送信
        if (isConnected)
        {
            SendMessageToServer("Test");
        }
    }

    void OnDestroy()
    {
        DisconnectFromServer();
    }

    // 接続の確立
    public async UniTask ConnectToServer(CancellationToken token = default)
    {
        int maxRetries = 100;
        int retryDelay = 500; // 初期遅延（ミリ秒）
        int retryCount = 0;

        while (retryCount < maxRetries)
        {
            try
            {
                client = new TcpClient
                {
                    ReceiveTimeout = 5000, // 5秒
                    SendTimeout = 5000     // 5秒
                };

                await client.ConnectAsync(serverIP, port);
                stream = client.GetStream();
                isConnected = true;

                // 非同期で受信するスレッドを開始
                UniTask.RunOnThreadPool(() => ReceiveData(token), cancellationToken: token).Forget();

                Debug.Log("サーバーに接続しました。");
                return; // 接続成功時にループを終了
            }
            catch (Exception e)
            {
                retryCount++;
                Debug.LogError($"サーバーへの接続に失敗しました (試行 {retryCount}): {e.Message}");
                if (retryCount >= maxRetries)
                {
                    Debug.LogError("サーバーへの接続を断念しました。");
                    return;
                }

                // 再試行時の遅延を指数的に増加させる（最大5秒まで）
                await UniTask.Delay(Math.Min(retryDelay * (1 << retryCount), 5000), cancellationToken: token);
            }
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
    private async UniTaskVoid ReceiveData(CancellationToken token)
    {
        byte[] buffer = new byte[1024];
        try
        {
            while (isConnected && !token.IsCancellationRequested)
            {
                if (stream != null && stream.DataAvailable)
                {
                    int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length, token);
                    if (bytesRead > 0)
                    {
                        string message = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                        Debug.Log("サーバーからのメッセージ: " + message);
                    }
                }
                await UniTask.Yield(); // 無駄なCPU使用を防ぐ
            }
        }
        catch (OperationCanceledException)
        {
            Debug.Log("受信処理がキャンセルされました。");
        }
        catch (Exception e)
        {
            Debug.LogError("メッセージ受信に失敗しました: " + e.Message);
            isConnected = false;
        }
    }

    // 接続の切断
    public void DisconnectFromServer()
    {
        isConnected = false;

        if (stream != null)
        {
            stream.Close();
            stream = null;
        }

        if (client != null)
        {
            client.Close();
            client = null;
        }

        if (tokenSource != null)
        {
            tokenSource.Cancel();
            tokenSource.Dispose();
        }

        Debug.Log("サーバーとの接続を切断しました。");
    }
}