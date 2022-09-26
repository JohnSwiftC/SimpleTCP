#include<stdio.h>
#include <io.h>
#include<winsock2.h>
#include <pthread.h>

#define NUM_THREADS 4

typedef struct s_playa{
    SOCKET s;
    int id;
} playa;
void *clientHandlerThread(playa stuff)
{
    int c;
    SOCKET ss;
    ss=stuff.s;
    long tid;
    tid=(long)stuff.id;
    SOCKET client_socket;
    struct sockaddr_in client;
    int recv_size;
    char *message;
    char client_reply[2000];
    int *notice = 1;
    c=sizeof(struct sockaddr_in);
    client_socket = accept(ss, (struct sockaddr *)&client, &c);
        
        if(client_socket==INVALID_SOCKET)
        {
            printf("Accept failed... %d", WSAGetLastError());
        }

    puts("Connection accepted!");

        //Send a response to the client, same as sending to server as a client. In fact, once connection is established, the idea of server and client can be removed,
        // and both can be seen as their respective sockets, allowing for continuous two way communication. (This is unique for TCP)
            
        //send takes a socket, the char pointer to a message (char *message;), the length of the message, and an integer determining a flag. Set this to 0 always.
    message="Chicken butt!\n";
    send(client_socket, message, strlen(message), 0);

    while((recv_size = recv(client_socket , client_reply , 2000 , 0)) > 0)
	{
        //Add a NULL terminating character to make it a proper string before printing
        client_reply[recv_size] = '\0';
        printf("Client %lu: ",pthread_self());
        puts(client_reply);
    }
    return NULL;
}


int main(int argc , char *argv[])
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    pthread_t tid[NUM_THREADS];


    
    

    //init wsa with this function, makeword(2,2) is used to identify which version, &wsa is the pointer to the WSADATA up above.
    if(WSAStartup(MAKEWORD(2, 2),&wsa)!=0)
    {
        printf("Init failed!");
        return 1;
    }

    printf("Init starting!\n");

    //Create a socket. Imagine like a tool not configured or connected, we have just created a TCP socket without anything yet to be doen with it.
    if((s=socket(AF_INET, SOCK_STREAM,0))==INVALID_SOCKET)
    {
        printf("Socket not created... %d",WSAGetLastError());
    }

    //Configuring the server struct seen before. The server address is yet another struct with s_addr being the actual IP. Set to INADDR_ANY
    //because we are not attempting to connect to a socket as a client, we ARE the socket.
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port = htons(8888);

    //bs incoming
    //first param is the socket, second is the pointer of the sockaddr_in struct (looks weird because C), and the size of said struct.
    //if client, use connect(), not bind()
    if(bind(s,(struct sockaddr *)&server, sizeof(server))==SOCKET_ERROR)
    {
        printf("Bind failed... %d",WSAGetLastError());
        return 1;
    }

    //output using puts() after binding, idfk why.
    puts("Bind Success!");

    //start listening for connection on the socket. 2nd param is the number of backlogged connections allowed (in this case I chose 3).
    listen(s,3);

    puts("Awaiting incoming connection...");
    playa stuff;
    stuff.s=s;
    int i;
    for(i=1;i<NUM_THREADS+1;i=i+1){
        stuff.id=i;
        pthread_create(&tid[i], NULL, clientHandlerThread, &stuff);
    }
    //For whatever reason looking at the winsock header, accept is similar to bind and connect, but with subtle differences, such as taking sockaddr_in length as a pointer.
    int check;
    int deathC;
    while(1){
        for(check=1;check<NUM_THREADS+1;check=check+1)
        {
            if((deathC=pthread_kill(tid[check],0))!=0)
            {
                pthread_create(&tid[check], NULL, clientHandlerThread, &stuff);
            }
            usleep(10000);
        }
    }

    getchar();

    closesocket(s);
    WSACleanup();
}
