import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('127.0.0.1', 3400))
server_socket.listen(1)

client_socket, client_address = server_socket.accept()
print(f"Accepted connection from {client_address}")

while True:
    data = client_socket.recv(1024)
    if not data:
        break
    lat, lon, el = map(float, data.decode().split(','))
    print(f"Received plane position: lat={lat}, lon={lon}, el={el}")

client_socket.close()
server_socket.close()
