require 'socket'

port = 8888

socket = TCPSocket.open('localhost', port)
# socket = TCPSocket.open('localhost', 8888)

while line = socket.gets
	puts line
end

socket.close
