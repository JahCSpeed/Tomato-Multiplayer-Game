#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
int totalConnections = 0;
int clientCount = 0;

int serverSocket;
struct client{
	int index;
	int socketId;
	struct sockaddr_in clientAddy;
	int len;
	char position[1024];

};

int tomatoPosition[10][10];
struct client clients[1024];
pthread_t thread[1024];
int numTomatoes = 0;
int score = 0;
int level = 0;
double rand01()
{
    return (double) rand() / (double) RAND_MAX;
}
void initGrid()
{
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            double r = rand01();
            if (r < 0.1) {
                tomatoPosition[i][j] = 1;
                numTomatoes++;
            }
            else
                tomatoPosition[i][j] = 0;
        }
    }

    // ensure grid isn't empty
    while (numTomatoes == 0)
        initGrid();
}



void printGrid(){
	//printf("Number of Tom: %d\n",numTomatoes);
	//printf("Score: %d\n",score);
	//printf("Level: %d\n",level);
	for(int i = 0; i < 10; i++){
		for(int j = 0; j < 10; j++){
			//printf("%d  ",tomatoPosition[j][i]);	
		}
		//printf("\n");
	}
	//printf("\n\n");
}
void updateBoard(int x, int y){
	if(tomatoPosition[x][y] == 1){
		score++;
		tomatoPosition[x][y] = 0;
		numTomatoes--;	
	}
	if (numTomatoes == 0) {
		level++;
        initGrid();
    }

}
void readData(char data[1024],struct client* clientDetail){
    char* token = strtok(data,"|\n");
    while(token != NULL){
		strcpy(clientDetail->position,data);
		int x = atoi(&token[0]);
        int y = atoi(&token[2]);
		updateBoard(x,y);
		token = strtok(NULL,"|\n");
	}
    //printGrid();
	
}
void* doNetworking(void* ClientDetail){
	struct client* clientDetail = (struct client*)ClientDetail;
	int index = clientDetail ->index;
	int clientSocket = clientDetail ->socketId;
	//printf("Client %d connected.\n",index +1);
	while(1){
		char data[1024];
		int read = recv(clientSocket,data,1024,0);
		data[read] = '\0';
		//printf("Client %d: %s\n",index + 1, data);
		char buffer[1024];
		readData(data,clientDetail);
		
		if(strcmp(data, ":exit:") == 0){
			int l = 0;
			for(int i = 0 ; i < clientCount ; i ++){
				//if(i != index)
					l += snprintf(buffer + l,1024,"%d:exit|\n",index +1);

			}
			send(clientSocket,buffer,1024,0);
			//printf("Client %d disconnected.\n",index +1);
			bzero(buffer, 1024);
			totalConnections--;
			break;
		}else{
			int l = 0;
			for(int i = 0 ; i < clientCount ; i ++){
				l += snprintf(buffer + l,1024,"Score:%d|\n",score);
				l += snprintf(buffer + l,1024,"Level:%d|\n",level);
				
				for(int i = 0; i < 10; i++){
					for(int j = 0; j < 10; j++){
						if(tomatoPosition[i][j] == 1)
							l += snprintf(buffer + l,1024,"TPos:%d,%d|\n",j,i);	
					}
				}
					
				if(i != index)
					l += snprintf(buffer + l,1024,"%d:%s|\n",i + 1,clients[i].position);

			}
 
			send(clientSocket,buffer,1024,0);
			bzero(buffer, 1024);
			continue;


		}	
	}
	return NULL;
}

int main(int argc, char **argv){
 char hostbuffer[256];
    struct hostent *host_entry;
    int hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);

    char *ip = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    int port = atoi(argv[1]);
    printf("Ip Address: %s\n", ip);
	printf("Port: %d.\n",port);
    struct sockaddr_in serverAddy;
    socklen_t addySize;
    
    int n;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0){
        perror("[-]Socket error.\n");
        exit(1);
    }
    //printf("[+]TCP server socket created.\n");

    memset(&serverAddy, '\0', sizeof(serverAddy));
    serverAddy.sin_family = AF_INET;
    serverAddy.sin_port = port;
    serverAddy.sin_addr.s_addr = inet_addr(ip);

    n = bind(serverSocket, (struct sockaddr*)&serverAddy, sizeof(serverAddy));
    if(n < 0){
        perror("[-]Bind error.\n");
        exit(1);
    }
    //printf("[+]Bind to port number: %d.\n", port);

    listen(serverSocket, 5);
    //printf("Listening...\n");
	initGrid();
	char buffer[1024];
	while(1){
		clients[clientCount].socketId = accept(serverSocket, (struct sockaddr*)&clients[clientCount].clientAddy, &clients[clientCount].len);
		if(totalConnections < 4){
			clients[clientCount].index = clientCount;
			pthread_create(&thread[clientCount],NULL,doNetworking,(void*)&clients[clientCount]);
			totalConnections++;
			clientCount++;
		}else{
			snprintf(buffer,1024,"Max Players of 4 Reached!\n");
			send(clients[clientCount].socketId,buffer,1024,0);
			bzero(buffer, 1024);
			close(clients[clientCount].socketId);
		}	
	}
	for(int i = 0; i < clientCount;i++){
		pthread_join(thread[i],NULL);
	}

	return EXIT_SUCCESS;
}