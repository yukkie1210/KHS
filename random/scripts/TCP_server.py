import socket

# サーバーのホストとポートを設定
HOST = "127.0.0.1" # ローカルホスト
PORT = 46963        # 任意のポート番号

def start_server():
    # TCPソケットを作成
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # ソケットをホストとポートにバインド
    server_socket.bind((HOST, PORT))
    
    # クライアントからの接続待ち状態にする
    server_socket.listen(1)
    print(f"サーバーが起動しました。接続を待っています... ({HOST}:{PORT})")
    
    # クライアントからの接続を待機
    client_socket, client_address = server_socket.accept()
    print(f"クライアントが接続しました: {client_address}")
    
    # クライアントに最初のメッセージを送信
    welcome_message = "Hello, client! You've connected to the server."
    client_socket.sendall(welcome_message.encode('utf-8'))
    print("クライアントに最初のメッセージを送信しました。")

    # クライアントからのメッセージを受信する
    try:
        while True:
            # 1024バイトまで受信
            data = client_socket.recv(1024)
            if not data:
                # データが空の場合は接続が終了したとみなす
                print("クライアントが接続を終了しました。")
                break
            
            # クライアントからのメッセージをデコードして表示
            received_message = data.decode('utf-8')
            print(f"クライアントからのメッセージ: {received_message}")

            # 応答メッセージをクライアントに送信
            response_message = "Message received: " + received_message
            client_socket.sendall(response_message.encode('utf-8'))
            print("応答メッセージをクライアントに送信しました。")

    except Exception as e:
        print(f"エラーが発生しました: {e}")
    
    # 接続を閉じる
    client_socket.close()
    server_socket.close()
    print("サーバーを終了しました。")

if __name__ == "__main__":
    start_server()