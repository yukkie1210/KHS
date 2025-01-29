import socket

def start_client(server_ip, server_port):
    try:
        # サーバーに接続
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((server_ip, server_port))
        print(f"Connected to server {server_ip}:{server_port}")

        while True:
            # データを受信
            data = client.recv(1024)
            if not data:
                print("Server closed the connection.")
                break

            # デコードして表示
            received_data = data.decode("utf-8")
            print(f"Received: {received_data}")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        client.close()
        print("Connection closed.")

if __name__ == "__main__":
    # サーバーのIPアドレスとポートを指定
    SERVER_IP = "192.168.180.207"
    SERVER_PORT = 46963

    start_client(SERVER_IP, SERVER_PORT)