#include "MyGame.h"

Particle::Particle(double x, double y, double vel_x, double vel_y, int size, SDL_Color color) {

    this->x = x;
    this->y = y;
    this->vel_x = vel_x;
    this->vel_y = vel_y;
    this->size = size;
    this->color = color;
    this->life = 1.0*10;
}

MyGame::MyGame(TTF_Font* font, Mix_Chunk* soundEffect, 
                Mix_Chunk* soundEffect2, SDL_Surface* surface, 
                SDL_Surface* pBall) {

    this->pFont = font;
    this->pSoundEffect = soundEffect;
    this->pSoundEffect2 = soundEffect2;
    this->pSurface = surface;
    this->pBall = pBall;
    
}

// receive
void MyGame::on_receive(std::string cmd, std::vector<std::string>& args) {
    if (cmd == "GAME_DATA") {
        // arguments sent by game data to server side, need to be in exact order
        if (args.size() == 6) {

            game_data.player1Y = stoi(args.at(0));
            game_data.player2Y = stoi(args.at(1));
            game_data.ballX = stoi(args.at(2));
            game_data.ballY = stoi(args.at(3));
            game_data.player1S = stoi(args.at(4));
            game_data.player2S = stoi(args.at(5));
            
        }

        // Handling events sent from server
    } else {
        std::cout << "Event Received from server: " << cmd << "\n" << std::endl;
        if (cmd == "HIT_WALL_RIGHTGAME_DATA" || cmd == "HIT_WALL_LEFTGAME_DATA" ||
            cmd == "HIT_WALL_DOWN" || cmd == "HIT_WALL_UP") {
            bPlaySound = true;
            playSoundEffect();
            if (cmd == "HIT_WALL_RIGHTGAME_DATA") {
                hitter = 1;
                bRightWallHit = true;
            }
            else if (cmd == "HIT_WALL_LEFTGAME_DATA") {
                hitter = 2;
                bLeftWallHit = true;
            }
            if (cmd == "HIT_WALL_DOWN" || cmd == "HIT_WALL_UP") {
                hitter = 0;
            }
        }
        else if (cmd == "BALL_HIT_BAT1" || cmd == "BALL_HIT_BAT2") {
            hitter = 0;
            bPlaySound2 = true;
            playSoundEffect();
        }
    }
}
//send
void MyGame::send(std::string message) {

    messages.push_back(message);
}
// inputs for both the players
void MyGame::input(SDL_Event& event) {

    switch (event.key.keysym.sym) {
    case SDLK_w:
        send(event.type == SDL_KEYDOWN ? "W_DOWN" : "W_UP");
        break;
    case SDLK_s:
        send(event.type == SDL_KEYDOWN ? "S_DOWN" : "S_UP");
        break;
    case SDLK_i:
        send(event.type == SDL_KEYDOWN ? "I_DOWN" : "I_UP");
        break;
    case SDLK_k:
        send(event.type == SDL_KEYDOWN ? "K_DOWN" : "K_UP");
        break;
    }
}
// update game logic
void MyGame::update() {
   
    if (player1score || player2score <= 10) {

        player1.y = game_data.player1Y;
        player2.y = game_data.player2Y;
        ball.x = game_data.ballX;
        ball.y = game_data.ballY;
        player1score = game_data.player1S;
        player2score = game_data.player2S;
        player1.x = playerXAxis.player1X;
        player2.x = playerXAxis.player2X;

       
        rotate += 10.0;
        if (rotate >= 360) { // rotate value is allowed only between 0 and 360
            rotate = 0.0;
        }

        t += 0.016;

        for (auto p : particles) {
            
            p->x += p->vel_x;
            p->y += p->vel_y;
            p->life -= 0.1;

            if (p->life <= 0) {
                p->color.a = 0;
            }
            else {
                p->color.a = (Uint8)(p->life / 3.0) * 255;
            }
        }
    }
}
// render
void MyGame::render(SDL_Renderer* renderer) {

    if (player1score || player2score <= 10) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

        particleEffect();

        for (Particle* p : particles) {

            double random_value = rand() * 1.0 / RAND_MAX;
            SDL_Rect rect = { (int)p->x, (int)p->y, p->size * 2, p->size * 2 };

            switch (hitter)
            {
            case 1:
                SDL_SetRenderDrawColor(renderer, (Uint8)(255 * random_value), p->color.g, p->color.b, p->color.a);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, (Uint8)(255 * random_value), 255, 255, p->color.a);
                break;
            case 0: 
                SDL_SetRenderDrawColor(renderer, (Uint8)(255 * random_value), 255, 0, p->color.a); 
                break;
            default:
                break;
            }

            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &player1);
    SDL_RenderFillRect(renderer, &player2);
  
    SDL_Color score_color1 = { 255,0,0,0 };
    SDL_Color score_color2 = { 0,70,255,0 };

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, pBall);
    SDL_RenderCopyEx(renderer, texture, NULL, &ball, rotate ,0, SDL_FLIP_NONE);
    
    std::string score_player1 = std::to_string(player1score);
    SDL_Surface* score_surface1 = TTF_RenderText_Blended(pFont, score_player1.c_str(), score_color1);
    if (score_surface1 != nullptr) {
        SDL_Texture* score_text_texture1 = SDL_CreateTextureFromSurface(renderer, score_surface1);
        SDL_FreeSurface(score_surface1);

        if (score_text_texture1 != nullptr) {

            SDL_RenderCopy(renderer, score_text_texture1, NULL, &player1score_rect);
            SDL_DestroyTexture(score_text_texture1);
        }else {
            std::cout << "Score texture for player 1 is nullptr!" << std::endl;
        }
    }else {
        std::cout << "Score surface 1 is nullptr!" << std::endl;
    }
    
    std::string score_player2 = std::to_string(player2score);
    SDL_Surface* score_surface2 = TTF_RenderText_Blended(pFont, score_player2.c_str(), score_color2);
    if (score_surface2 != nullptr) {
        SDL_Texture* score_text_texture2 = SDL_CreateTextureFromSurface(renderer, score_surface2);
        SDL_FreeSurface(score_surface2);

        if (score_text_texture2 != nullptr) {

            SDL_RenderCopy(renderer, score_text_texture2, NULL, &player2score_rect);
            SDL_DestroyTexture(score_text_texture2);
        }else {
            std::cout << "Score texture for player 2 is nullptr!" << std::endl;
        }
    }else {
        std::cout << "Score surface 2 is nullptr!" << std::endl;
    }
    
    //Rendering picture and victory text, using the same surfrace is easy to prevent overriding
    if (player1score >= 10) {

        SDL_Surface* scoreSurface3 = TTF_RenderText_Blended(pFont, "Player 1 victory", score_color1);
        if (scoreSurface3 != nullptr) {
            SDL_Texture* scoreTextTexture3 = SDL_CreateTextureFromSurface(renderer, scoreSurface3);
            SDL_FreeSurface(scoreSurface3);

            if (scoreTextTexture3 != nullptr) {
                
                int w, h;
                SDL_QueryTexture(scoreTextTexture3, NULL, NULL, &w, &h);
                SDL_Rect dst = { 300,200,w,h };

                SDL_RenderCopy(renderer, scoreTextTexture3, NULL, &dst);
                SDL_DestroyTexture(scoreTextTexture3);
            }else {
                std::cout << "Score texture 3 is nullptr!" << std::endl;
            }
        }else {
            std::cout << "Score surface 3 is nullptr!" << std::endl;
        }

        int texture_width = 200;
        int texture_height = 200;

        SDL_Rect dst = { 100,100, texture_height, texture_width };
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, pSurface);
        if (texture != nullptr) {
            SDL_RenderCopy(renderer, texture, NULL, &dst);
            SDL_DestroyTexture(texture);
        }else {
            std::cout << "texture is nullptr!" << std::endl;
        }

    }
    else if (player2score >= 10) {

        SDL_Surface* scoreSurface3 = TTF_RenderText_Blended(pFont, "Player 2 victory", score_color2);
        if (scoreSurface3 != nullptr) {
            SDL_Texture* scoreTextTexture3 = SDL_CreateTextureFromSurface(renderer, scoreSurface3);
            SDL_FreeSurface(scoreSurface3);

            if (scoreTextTexture3 != nullptr) {
                
                int w, h;
                SDL_QueryTexture(scoreTextTexture3, NULL, NULL, &w, &h);
                SDL_Rect dst = { 300,200,w,h };

                SDL_RenderCopy(renderer, scoreTextTexture3, NULL, &dst);
                SDL_DestroyTexture(scoreTextTexture3);
            }else {
                std::cout << "Score texture 3 is nullptr!" << std::endl;
            }
        }else {
            std::cout << "Score surface 3 is nullptr!" << std::endl;
        }
    
        int texture_width = 200;
        int texture_height = 200;

        SDL_Rect dst = { 100,100, texture_height, texture_width };
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, pSurface);
        if (texture != nullptr) {
            SDL_RenderCopy(renderer, texture, NULL, &dst);
            SDL_DestroyTexture(texture);
        }else {
            std::cout << "texture is nullptr!" << std::endl;
        }
    }

}
// soud effect function
void MyGame::playSoundEffect() {
    if (bPlaySound) {
        Mix_PlayChannel(-1, pSoundEffect, 0);
        Mix_VolumeChunk(pSoundEffect, 5);
        bPlaySound = false;
    }if (bPlaySound2) {
        Mix_PlayChannel(-1, pSoundEffect2, 0);
        Mix_VolumeChunk(pSoundEffect2, 5);
        bPlaySound2 = false;
    }
}

void MyGame::particleEffect() {

    int x = ball.x + 10;
    int y = ball.y + 10;
    
    for (int i = 0; i < 5; i++) {

        double vel_x = get_random();
        double vel_y = get_random();

        particles.push_back(new Particle(x, y, vel_x, vel_y, 1, { 255,0,0,0 }));
       
        x++;

        if (x == 30) {
            y++;
        }

    }

}