# Client-server Protocol
The protocol is based on a client-server model where the server listens for incoming connections from clients. Once a client connects, it can send commands to the server to perform certain operations. The server processes these commands and sends back responses to the client.

## Server Setup
The server is set up to listen on a specific port for incoming connections. When a client connects, the server accepts the connection and creates a new socket for communication with that client. This allows the server to handle multiple clients simultaneously, each with its own dedicated socket.

## Client Setup
The client connects to the server by specifying the server’s IP address and port number. Once connected, the client sends its username to the server. The server creates a new directory with the username if it doesn’t exist already.

## Communication
The client and server communicate by sending and receiving data over their sockets. The data is sent in chunks of a specified size (1024 bytes in our case). The client can send commands to the server, and the server processes these commands and sends back responses.

## Classes
The server and client have next classes:

FileHandler: it process all the information that user sends and server receives

Server: class that contains all the stuff about program and connection setup

## Commands
The client can send the following commands to the server:

send: The client sends a file to the server. The file is read in chunks and sent over the socket. The server receives the file and writes it to the user’s directory.

list: The server sends a list of files in the client’s directory. The server reads the directory contents and sends the file names to the client.

delete: The client requests to delete a file in its directory. The server deletes the file and sends a confirmation to the client.

info: The client requests information about a file in its directory. The server retrieves the file information and sends it to the client.

## Functions
Here are the main functions used in the protocol:

handleClient: This function runs in a separate thread for each client. It receives commands from the client and calls the appropriate function to process the command.

sendFile: This function is called when the client wants to send a file to the server. It reads the file in chunks and sends each chunk to the server.

receiveFile: This function is called when the server needs to receive a file from the client. It receives the file in chunks and writes each chunk to a file in the user’s directory.

listFilesInDirectory: This function is called when the client requests a list of files in its directory. The server reads the directory contents and sends the file names to the client.

deleteFile: This function is called when the client requests to delete a file in its directory. The server deletes the file and sends a confirmation to the client.

getFileInfo: This function is called when the client requests information about a file in its directory. The server retrieves the file information and sends it to the client.
