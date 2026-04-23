// IMPORTANT: Winsock Library ("ws2_32") should be linked

#include <stdio.h>
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

int main(int argc, char *argv[]) {
	//PASO 1
	//Datos
	WSADATA wsaData;
	//La conexión entre sockets
	SOCKET conn_socket; //Solo para establecer la conexion
	SOCKET comm_socket;
	//Puerto, direccion,...
	struct sockaddr_in server;
	struct sockaddr_in client;
	//2 bandas: para enviar y recibir
	char sendBuff[512], recvBuff[512];

	printf("\nInitialising Winsock...\n");
	//Iniciar con la version 2.2 y le paso los datos
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		//En caso de fallo
		printf("Failed. Error Code : %d", WSAGetLastError());
		return -1;
	}
	//Acabamos con la inicialización de la libreria
	printf("Initialised.\n");

	//PASO 2
	//SOCKET creation
	//Por un lado af_inet la version y despues si queremos udp o ccp
	if ((conn_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//Acabamos con la creacion del socket
	printf("Socket created.\n");

	//Asignarle (Ipv 4 = AF_INET): que puertos, familia y direcciones vamos a usar
	server.sin_addr.s_addr = inet_addr(SERVER_IP); //inet_addr : transformar ipv4 a una standar (numeros y puntos
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	//BIND (the IP/port with socket)
	if (bind(conn_socket, (struct sockaddr*) &server,
			sizeof(server)) == SOCKET_ERROR) {
		//Si falla print el error
		printf("Bind failed with error code: %d", WSAGetLastError());
		//Lo destruimos el socket
		closesocket(conn_socket);
		//Limpiamos la libreria
		WSACleanup();
		return -1;
	}

	printf("Bind done.\n");

	//LISTEN to incoming connections (socket server moves to listening mode)
	//Como servidor (listening mode) == > con la funcion listen()
	//El 1 indica el numero de usuarios que se esta esperando
	if (listen(conn_socket, 1) == SOCKET_ERROR) {
		printf("Listen failed with error code: %d", WSAGetLastError());
		//Destruimos el socket
		closesocket(conn_socket);
		//Limpiamos la libreria
		WSACleanup();
		return -1;
	}

	//ACCEPT incoming connections (server keeps waiting for them)
	printf("Waiting for incoming connections...\n");
	int stsize = sizeof(struct sockaddr);
	//Nos quedamos a la escucha y cuando haya una comunicacion  le damos paso por el socket de comunicacion
	//y que el otro se quede a la escucha
	comm_socket = accept(conn_socket, (struct sockaddr*) &client, &stsize);
	// Using comm_socket is able to send/receive data to/from connected client
	if (comm_socket == INVALID_SOCKET) {
		printf("accept failed with error code : %d", WSAGetLastError());
		//Destruimos el socket
		closesocket(conn_socket);
		//Limpiamos la libreria
		WSACleanup();
		return -1;
	}
	printf("Incomming connection from: %s (%d)\n", inet_ntoa(client.sin_addr),
			ntohs(client.sin_port));

	// Closing the listening sockets (is not going to be used anymore)
	//Una vez que tenemos el puente de envio de comunicacion cerramos el socket de conexion porque en este caso
	//ya no esperamos más comunicaciones
	closesocket(conn_socket);

	//SEND and RECEIVE data
	printf("Waiting for incoming messages from client... \n");
	do {
		//Record value, coge la informacion y la pone en el buffer
		int bytes = recv(comm_socket, recvBuff, sizeof(recvBuff), 0);
		if (bytes > 0) {
			//Logica del servidor
			printf("Receiving message... \n");
			printf("Data received: %s \n", recvBuff);

			printf("Sending reply... \n");
			strcpy(sendBuff, "ACK -> ");
			strcat(sendBuff, recvBuff);
			//Acaba la logica del servidor
			//Manda atraves de este socket los bytes que estan en el buffer
			//Este solo copia y pega. Send mensaje de vuelta atraves del buffer
			send(comm_socket, sendBuff, sizeof(sendBuff), 0);
			printf("Data sent: %s \n", sendBuff);

			if (strcmp(recvBuff, "Bye") == 0)
				break;
		}
	} while (1);

	// CLOSING the sockets and cleaning Winsock...
	//Siempre que acabemos con algo cerramos el socket y limpiamos la libreria
	closesocket(comm_socket);
	WSACleanup();

	return 0;
}

