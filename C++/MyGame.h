#ifndef __MY_GAME_H__
#define __MY_GAME_H__


#include <iostream>
#include <vector>
#include <string>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL.h>

// These are the value of basic game data with initialisation of positions.
static struct GameData {

    short int player1Y = 0;
    short int player2Y = 0;
    short int ballX = 0;
    short int ballY = 0;
    short int player1S = 0;
    short int player2S = 0;
    
} game_data;

// hardcoded positions of X axis, values are same like on server
static struct PlayerXAxis {

    int player2X = 3 * 800 / 4 -20;
    int player1X = 800 / 4;

} playerXAxis;
//Single particle
class Particle {
public:
    double x;
    double y;

    double vel_x;
    double vel_y;

    double life;

    int size;
    SDL_Color color;

    Particle(double x, double y, double vel_x, double vel_y, int size, SDL_Color color);

};

class MyGame {

    private:
        SDL_Rect player1 = { 0, 0, 20, 60 };
        SDL_Rect player2 = { 0, 0, 20, 60 };
        SDL_Rect ball = { 0,0,20,20 };
        TTF_Font *pFont;
        Mix_Chunk* pSoundEffect;
        Mix_Chunk* pSoundEffect2;
        SDL_Surface* pSurface;
        SDL_Surface* pBall;
        std::vector<Particle*> particles;
      
        double get_random() {
            return rand() * 1.0 / RAND_MAX;
        }

    public:
        MyGame(TTF_Font* font, Mix_Chunk* soundEffect, Mix_Chunk* soundEffect2, SDL_Surface* surface, SDL_Surface* pBall);
        int player1score = 0;      
        int player2score = 0;
        SDL_Rect player1score_rect = { 50, 0, 50, 100 }; // score text rectangles
        SDL_Rect player2score_rect = { 700,0,50,100 }; // score text rentangles
        bool bPlaySound = false;
        bool bPlaySound2 = false;
        double rotate = 0.0;
        double t = 0.0;
        bool bRightWallHit = false;
        bool bLeftWallHit = false;
        int hitter = 0;

        std::vector<std::string> messages;
        // this is interesting, in video he has said that "we want pointer no copies" this might be answer for my memory leak
        
        void on_receive(std::string message, std::vector<std::string>& args);
        void send(std::string message);
        void input(SDL_Event& event);
        void update();
        void render(SDL_Renderer* renderer);
        void playSoundEffect();
        void particleEffect();

};

#endif