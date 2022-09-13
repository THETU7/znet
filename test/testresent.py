import socket, sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 9981))
str = 'a' * int(sys.argv[1])
sock.send(str.encode())
while 1:
  data = sock.recv(1024)
  print(data.decode())
