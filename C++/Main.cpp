#include "SDL_net.h"
#include "MyGame.h"

using namespace std;

const char* IP_NAME = "localhost"; // we can change to any ip address
const Uint16 PORT = 55555; // anything above 4000 should be fine, low values are reserved for other ports

bool is_running = true;

// two new variables for height and width
int WindowH = 800;
int WindowW = 600;

MyGame* game;
Uint32 frameStart;
int frameTime;
const int FPS = 60;
const int frameDelay = 1000 / FPS;

// Receive thread
static int on_receive(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;
    // possible way how to enchance performance
    const int message_length = 200; // 100; original was 1024

    char message[message_length];
    int received;

    
    // TODO: while(), rather than do
    do {
        received = SDLNet_TCP_Recv(socket, message, message_length);
        message[received] = '\0';

        char* pch = strtok(message, ",");

        // get the command, which is the first string in the message
        string cmd(pch);

        // then get the arguments to the command
        vector<string> args;

                
        while (pch != NULL) {
            pch = strtok(NULL, ",");

            if (pch != NULL) {
                args.push_back(string(pch));
            }
            
        }

        game->on_receive(cmd, args);

        if (cmd == "exit") {
            break;
        }

    } while (received > 0 && is_running);

    return 0;
}
// Send thread
static int on_send(void* socket_ptr) {
    TCPsocket socket = (TCPsocket)socket_ptr;
    
    while (is_running) {
        if (game->messages.size() > 0) {
            string message = "CLIENT_DATA";

            for (auto m : game->messages) {
                message += "," + m;
            }

            game->messages.clear();

            SDLNet_TCP_Send(socket, message.c_str(), message.length());
        }

        SDL_Delay(1);
    }

    return 0;
}

// LOOP is input, update, render - fundamental thing to know.
void loop(SDL_Renderer* renderer) {
    SDL_Event event;
    
    while (is_running) {
            
        frameStart = SDL_GetTicks();
            
            while (SDL_PollEvent(&event)) {
                if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) && event.key.repeat == 0) {
                    game->input(event);

                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        is_running = false;
                        break;

                    default:
                        break;
                    }
                }

                if (event.type == SDL_QUIT) {
                    is_running = false;
                }

            }
            

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // this is rendering the background (black collor)
        SDL_RenderClear(renderer);
        // update
        game->update();
        // render
        game->render(renderer);
        SDL_RenderPresent(renderer);
        // 60 frames a second roughly.. was (SDL_Delay(17), now with the new version IT IS 60fps
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
           SDL_Delay(frameDelay - frameTime);
            
        }
        
    }
}

const int USE_ENTIRE_TEXTURE = NULL;
int run_game() {
    // creating an application window
    SDL_Window* window = SDL_CreateWindow(
        "Multiplayer Pong Client",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WindowH, WindowW, //800, 600,
        SDL_WINDOW_SHOWN
    );
    // checking if there is window
    if (nullptr == window) {
        std::cout << "Failed to create window" << SDL_GetError() << std::endl;
        return -1;
    }
    // abstraction of our GPU, let us to draw stuff
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // checking if there is an renderer
    if (nullptr == renderer) {
        std::cout << "Failed to create renderer" << SDL_GetError() << std::endl;
        return -1;
    }
    
    loop(renderer);
    return 0;
}

// GAME START here
int main(int argc, char** argv) {

    // Initialize SDL - first thing to do is to initiliase the SDL, if fail then exit
    if (SDL_Init(0) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // initilisation of library
    if (TTF_Init() == -1) {
        printf("TFF_Init: %s\n", SDL_GetError());
        exit(5);
    }

    // Initialize SDL_net
    if (SDLNet_Init() == -1) {
        printf("SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }
    // Creating the IP address structure IP and SOCKET are both different values and they both vary at different client
    // this might be useful for a controller setup
    /*TTF FONT INITIALISATION**********************************/
    TTF_Font* scoreFont = NULL;
    scoreFont = TTF_OpenFont("assets/pong.ttf", 50);
    if (scoreFont != nullptr) {
        std::cout << "Font is loaded\n" << std::endl;
    }
    else {
        std::cout << "Font is NOT loaded\n" << std::endl;
    }
 
    /*********************************************************/
    
    /*Surface**************************************************/
    SDL_Surface* surface = NULL;
    surface = IMG_Load("assets/confetti.png");
    if (surface != nullptr) {
        std::cout << "confetti picture loaded\n" << std::endl;
    }
    else {
        std::cout << "confetti picture is NOT loaded\n" << std::endl;
    }

    SDL_Surface* pBall = NULL;
    pBall = IMG_Load("assets/pongBall.png");
    if (pBall != nullptr) {
        std::cout << "confetti picture loaded\n" << std::endl;
    }
    else {
        std::cout << "confetti picture is NOT loaded\n" << std::endl;
    }

    /***************************************************************/
    
    /*Sound library initialisation***********************************/
    if (SDL_Init(SDL_INIT_AUDIO) == -1) {
        printf("SDL_Init: %s\n", SDL_GetError());
    }

    /* PLAY BACKGROUND MUSIC */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Mix_OpenAudio is null");
    }

    Mix_Music* background = NULL;
    background = Mix_LoadMUS("assets/Arcade_Kid.wav");
    if (background != nullptr) {
        printf("Backgrouhd music loaded");
    }
    else {
        printf("Background music is NOT loaded");
    }
    Mix_PlayMusic(background,-1);
    Mix_VolumeMusic(10); // 10% of volume

    Mix_Chunk* sound = NULL;
    sound = Mix_LoadWAV("assets/hit_wall.wav");
    if (sound != nullptr) {
        printf("Sound effect is loaded");
    }
    else {
        printf("SoundEffect is NOT loaded");
    }

    Mix_Chunk* sound2 = NULL;
    sound2 = Mix_LoadWAV("assets/hit_bat.wav");
    if (sound2 != nullptr) {
        printf("Sound effect is loaded");
    }
    else {
        printf("SoundEffect is NOT loaded");
    }
    
    IPaddress ip;
    printf("IP Address created\n");
    std::cout << &ip << std::endl;
    // Resolve host (ip name + port) into an IPaddress type
    if (SDLNet_ResolveHost(&ip, IP_NAME, PORT) == -1) {
        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        exit(3);
    }

    // Open the connection to the server - we can send and receive trhough the socket
    TCPsocket socket = SDLNet_TCP_Open(&ip);
    printf("Socket\n");
    std::cout << &socket << std::endl;
    if (!socket) {
        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        exit(4);
    }

    game = new MyGame(scoreFont,sound,sound2,surface, pBall);

    // Two threads, one receive and one send - this is a blocking function..
    SDL_CreateThread(on_receive, "ConnectionReceiveThread", (void*)socket);
    SDL_CreateThread(on_send, "ConnectionSendThread", (void*)socket);

    run_game();
   
    // DELETE AND MEMORY UNLOAD
    delete game;
    
    //release the resource to OP
    TTF_CloseFont(scoreFont);
    scoreFont = NULL;
    // close the music
    Mix_FreeMusic(background);
    background = NULL;

    Mix_FreeChunk(sound); 
    sound = NULL;

    Mix_FreeChunk(sound2);
    sound2 = NULL;

    SDL_FreeSurface(surface);
    SDL_FreeSurface(pBall);
    // Close connection to the server
    SDLNet_TCP_Close(socket);

    // Shutdown SDL_net
    SDLNet_Quit();

    // Shutdown SDL
    SDL_Quit();

    return 0;
}