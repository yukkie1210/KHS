import socket

SERVER_IP = "127.0.0.1"
SERVER_PORT = 9000
CLIENT_PORT =9001

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client_socket.bind(("0.0.0.0", CLIENT_PORT))

try:
    message = "Hello from Python UDP Client!"
    print(f"Sending message to {SERVER_IP}:{SERVER_PORT} -> {message}")
    client_socket.sendto(message.encode(), (SERVER_IP, SERVER_PORT))

    print("Waiting for response...")
    data, server = client_socket.recvfrom(1024)
    print(f"Received response from {server}: {data.decode()}")

except Exception as e:
    print(f"An error occurred: {e}")

finally:
    client_socket.close()
    print("Socket closed.")