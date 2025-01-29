import socket
class UDPServer:
    def __init__(self, port):
        self.server_port = port
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server_socket.bind(("0.0.0.0", self.server_port))
        print(f"UDP Server started on port {self.server_port}")

    def start(self):
        try:
            while True:
                print("Waiting for data...")
                data, client_address = self.server_socket.recvfrom(1024)
                message = data.decode('utf-8')
                print(f"Received message: '{message}' from {client_address}")

                response = f"Hello, {client_address}. You sent: {message}"
                self.server_socket.sendto(response.encode('utf-8'), client_address)
                print(f"Sent response: '{response} to {client_address}")
        except KeyboardInterrupt:
            print("Server is shutting down...")
        finally: 
            self.server_socket.close()
            print("Socket closed.")

if __name__ == "__main__":
    PORT = 9000
    udp_server = UDPServer(PORT)
    udp_server.start()