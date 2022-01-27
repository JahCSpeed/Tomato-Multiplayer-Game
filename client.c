#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10

typedef struct
{
    int x;
    int y;
} Position;

typedef struct
{
    int active;
    int id;
    Position playerPosition;
} Player;

typedef enum
{
    TILE_GRASS,
    TILE_TOMATO
} TILETYPE;

TILETYPE grid[GRIDSIZE][GRIDSIZE];

Player *playerList;
Player self;
Player player2;
Player player3;
Player player4;
Position playerPosition;

int score;
int level;
int numTomatoes;

bool shouldExit = false;

TTF_Font* font;
char ip[1024];
char buffer[1024];
int port;
int cSocket;
struct sockaddr_in addy;
void readData(char data[1024]);
void updatePlayerPos(int playerId, Position playerPos);
void printPlayerData();
// get a random value in the range [0, 1]


double rand01()
{
    return (double) rand() / (double) RAND_MAX;
}
int checkPlayerId(int playerId){
    if(playerId == player2.id || playerId == player3.id || playerId == player3.id){
        return 0;
    }
    return -1;
}
int checkPlayerPos(int x, int y){
    if(player2.playerPosition.x == x && player2.playerPosition.y == y){
        //printf("On Top Of Player 2\n");
        return 0;
    }
    if(player3.playerPosition.x == x && player3.playerPosition.y == y){
        //printf("On Top Of Player 3\n");
        return 0;
    }
    if(player4.playerPosition.x == x && player4.playerPosition.y == y){
        //printf("On Top Of Player \n");
        return 0;
    }
    return -1;
}
void addPlayer(int playerId){
    if(player2.active == 0 && checkPlayerId(playerId) == -1){
        player2.id = playerId;
        player2.active = 1;
        return;
    }else if(player3.active == 0 && checkPlayerId(playerId) == -1){
        player3.id = playerId;
        player3.active = 1;
        return;
    }else if(player4.active == 0 && checkPlayerId(playerId) == -1){
        player4.id = playerId;
        player4.active = 1;
        return;
    }
}
void removePlayer(int playerId){
    if(player2.id == playerId){
        player2.active = 0;
        return;
    }else if(player3.id == playerId){
        player3.active = 0;
        return;
    }else if(player4.id == playerId){
        player4.active = 0;
        return;
    }
}
void * doRecieving(void * sockID){

	int clientSocket = *((int *) sockID);

	while(1){

		char data[1024];
		int read = recv(clientSocket,data,1024,0);

		data[read] = '\0';
		//printf("%s\n",data);
        readData(data);
        

	}

}
void printPlayerData(){
    if(player2.active == 1){
        //printf("Player: %d \t Pos: (%d,%d)\n",player2.id,player2.playerPosition.x,player2.playerPosition.y);
    }
    if(player3.active == 1){
        //printf("Player: %d \t Pos: (%d,%d)\n",player3.id,player3.playerPosition.x,player3.playerPosition.y);
    }
    if(player4.active == 1){
        //printf("Player: %d \t Pos: (%d,%d)\n\n",player4.id,player4.playerPosition.x,player4.playerPosition.y);
    }
}
void updatePlayerPos(int playerId, Position playerPos){

    if(player2.id == playerId){
        player2.playerPosition = playerPos;
        return;
    }
    if(player3.id == playerId){
        player3.playerPosition = playerPos;
        return;
    }
    if(player4.id == playerId){
        player4.playerPosition = playerPos;
        return;
    }
    
}
void finishGrid(){

    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            if (grid[i][j] != TILE_TOMATO) {
                grid[i][j] = TILE_GRASS;
            }
            
        }
    }
    
}
void resetGrid(){

    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            grid[i][j] = TILE_GRASS;
        }
    }
    
}
void readData(char data[1024]){
    char* token = strtok(data,"|\n");
    resetGrid();
    int playerid = -1;
    Position playerPos;
    while(token != NULL){
        
        if(strstr(token,"TPos") != NULL){
            int x = atoi(&token[5]);
            int y = atoi(&token[7]);
            grid[y][x] = TILE_TOMATO;
                    
        }else if(strstr(token,"Score") != NULL){
            score = atoi(&token[6]);
                    
        }else if(strstr(token,"Level") != NULL){
            level = atoi(&token[6]);
                    
        }else{ 
            playerid = atoi(&token[0]);
            if(playerid == 0){continue;}
            if(strstr(token,"exit") != NULL){
                removePlayer(playerid);
            }else{
                playerPos.x = atoi(&token[2]);
                playerPos.y = atoi(&token[4]);
                addPlayer(playerid);
                updatePlayerPos(playerid,playerPos);
            }
        }
        
        token = strtok(NULL,"|\n");
        
        
    }
    //finishGrid();
    //printPlayerData();

}
void initGrid()
{
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
                grid[i][j] = TILE_GRASS;
        }
    }
}

void initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int rv = IMG_Init(IMG_INIT_PNG);
    if ((rv & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "Error initializing IMG: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "Error initializing TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

void moveTo(int x, int y)
{
    // Prevent falling off the grid
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return;

    // Sanity check: player can only move to 4 adjacent squares
    if (!(abs(self.playerPosition.x - x) == 1 && abs(self.playerPosition.y - y) == 0) &&
        !(abs(self.playerPosition.x - x) == 0 && abs(self.playerPosition.y - y) == 1)) {
        fprintf(stderr, "Invalid move attempted from (%d, %d) to (%d, %d)\n", self.playerPosition.x, self.playerPosition.y, x, y);
        return;
    }
    if(player2.active == 1){
        if(x == player2.playerPosition.x && y == player2.playerPosition.y ){
            return;
        }
       
    }
    if(player3.active == 1){
        if(x == player3.playerPosition.x && y == player3.playerPosition.y ){
            return;
        }
        
    }
    if(player4.active == 1){
        if(x == player4.playerPosition.x && y == player4.playerPosition.y ){
            return;
        }
        
    }

    self.playerPosition.x = x;
    self.playerPosition.y = y;

    if (grid[x][y] == TILE_TOMATO) {
        grid[x][y] = TILE_GRASS;
        //score++;
        //numTomatoes--;

        if (numTomatoes == 0) {
            //level++;
            //initGrid();
        }
    }
}

void handleKeyDown(SDL_KeyboardEvent* event)
{
    // ignore repeat events if key is held down
    if (event->repeat)
        return;

    if (event->keysym.scancode == SDL_SCANCODE_Q || event->keysym.scancode == SDL_SCANCODE_ESCAPE)
        shouldExit = true;

    if (event->keysym.scancode == SDL_SCANCODE_UP || event->keysym.scancode == SDL_SCANCODE_W)
        moveTo(self.playerPosition.x, self.playerPosition.y - 1);

    if (event->keysym.scancode == SDL_SCANCODE_DOWN || event->keysym.scancode == SDL_SCANCODE_S)
        moveTo(self.playerPosition.x, self.playerPosition.y + 1);

    if (event->keysym.scancode == SDL_SCANCODE_LEFT || event->keysym.scancode == SDL_SCANCODE_A)
        moveTo(self.playerPosition.x - 1, self.playerPosition.y);

    if (event->keysym.scancode == SDL_SCANCODE_RIGHT || event->keysym.scancode == SDL_SCANCODE_D)
        moveTo(self.playerPosition.x + 1, self.playerPosition.y);

    
}

void processInputs()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				shouldExit = true;
				break;

            case SDL_KEYDOWN:
                handleKeyDown(&event.key);
				break;

			default:
				break;
		}
	}
}
void placePlayer(){
    for(int i = 0; i < GRIDSIZE;i++){
        for(int j = 0; j < GRIDSIZE; j++){
            if(grid[i][j] == TILE_GRASS && checkPlayerPos(i,j) == -1 ){
                self.playerPosition.x = i;
                self.playerPosition.y = j;    
            }
        }
    }
}
void drawGrid(SDL_Renderer* renderer, SDL_Texture* grassTexture, SDL_Texture* tomatoTexture, SDL_Texture* playerTexture)
{
    SDL_Rect dest;
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            dest.x = 64 * i;
            dest.y = 64 * j + HEADER_HEIGHT;
            SDL_Texture* texture = (grid[i][j] == TILE_GRASS) ? grassTexture : tomatoTexture;
            SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, texture, NULL, &dest);
        }
    }

    dest.x = 64 * self.playerPosition.x;
    dest.y = 64 * self.playerPosition.y + HEADER_HEIGHT;
    SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, playerTexture, NULL, &dest);
    if(player2.active == 1){
        dest.x = 64 * player2.playerPosition.x;
        dest.y = 64 * player2.playerPosition.y + HEADER_HEIGHT;
        SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
        SDL_RenderCopy(renderer, playerTexture, NULL, &dest);
    }
    if(player3.active == 1){
        dest.x = 64 * player3.playerPosition.x;
        dest.y = 64 * player3.playerPosition.y + HEADER_HEIGHT;
        SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
        SDL_RenderCopy(renderer, playerTexture, NULL, &dest);
    }
    if(player4.active == 1){
        dest.x = 64 * player4.playerPosition.x;
        dest.y = 64 * player4.playerPosition.y + HEADER_HEIGHT;
        SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
        SDL_RenderCopy(renderer, playerTexture, NULL, &dest);
    }
}

void drawUI(SDL_Renderer* renderer)
{
    // largest score/level supported is 2147483647
    char scoreStr[18];
    char levelStr[18];
    sprintf(scoreStr, "Score: %d", score);
    sprintf(levelStr, "Level: %d", level);

    SDL_Color white = {255, 255, 255};
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreStr, white);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

    SDL_Surface* levelSurface = TTF_RenderText_Solid(font, levelStr, white);
    SDL_Texture* levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);

    SDL_Rect scoreDest;
    TTF_SizeText(font, scoreStr, &scoreDest.w, &scoreDest.h);
    scoreDest.x = 0;
    scoreDest.y = 0;

    SDL_Rect levelDest;
    TTF_SizeText(font, levelStr, &levelDest.w, &levelDest.h);
    levelDest.x = GRID_DRAW_WIDTH - levelDest.w;
    levelDest.y = 0;

    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDest);
    SDL_RenderCopy(renderer, levelTexture, NULL, &levelDest);

    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    SDL_FreeSurface(levelSurface);
    SDL_DestroyTexture(levelTexture);
}

int main(int argc, char* argv[])
{
    
    //Start OF SERVER SHIT
    srand(time(NULL));
    strcpy(ip,argv[1]);
    port = atoi(argv[2]);
    int cSocket;
    struct sockaddr_in addy;

    cSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(cSocket < 0){
        perror("[-]Socket error.\n");
        exit(1);
    }
    memset(&addy, '\0', sizeof(addy));
    addy.sin_family = AF_INET;
    addy.sin_port = port;
    addy.sin_addr.s_addr = inet_addr(ip);

    connect(cSocket, (struct sockaddr*)&addy, sizeof(addy));
    printf("Connected to the server!\n");
    pthread_t thread;
	pthread_create(&thread, NULL, doRecieving, (void *) &cSocket );
    //END OF SERVER SHIT
    level = 1;

    initSDL();

    font = TTF_OpenFont("resources/Burbank-Big-Condensed-Bold-Font.otf", HEADER_HEIGHT);
    if (font == NULL) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    placePlayer();
    //initGrid();

    SDL_Window* window = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (window == NULL) {
        fprintf(stderr, "Error creating app window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	if (renderer == NULL)
	{
		fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
	}

    SDL_Texture *grassTexture = IMG_LoadTexture(renderer, "resources/grass.png");
    SDL_Texture *tomatoTexture = IMG_LoadTexture(renderer, "resources/tomato.png");
    SDL_Texture *playerTexture = IMG_LoadTexture(renderer, "resources/player.png");
    // main game loop
    while (!shouldExit) {
        SDL_SetRenderDrawColor(renderer, 0, 105, 6, 255);
        SDL_RenderClear(renderer);

        
        sprintf(buffer, "%d,%d",self.playerPosition.x,self.playerPosition.y);
        send(cSocket, buffer, strlen(buffer), 0);
        bzero(buffer, 1024);
        processInputs();     
        drawGrid(renderer, grassTexture, tomatoTexture, playerTexture);   
        drawUI(renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // 16 ms delay to limit display to 60 fps
        
        

    }
    
    sprintf(buffer, ":exit:");
    send(cSocket, buffer, strlen(buffer), 0);
    close(cSocket);
// clean up everything
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(tomatoTexture);
    SDL_DestroyTexture(playerTexture);

    TTF_CloseFont(font);
    TTF_Quit();

    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
