#include <bits/stdc++.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#define GRAVITY_SPEED 1
#define MAX_FAIL_SPEED 4
#define PLAYER_SPEED 3
const int TILE_SIZE = 50;
const int MAX_MAP_X = 24;
const int MAX_MAP_Y = 12;
static SDL_Event gEvent;
using namespace std;
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
const string WINDOW_TITLE = "Trap Adventure";
typedef struct Input{
    int left_;
    int right_;
    int up_;
    int down_;
    int jump_;
};

void logSDLError(ostream& os, const string &msg, bool fatal = false);
void logSDLError(ostream& os, const string &msg, bool fatal)
{
    os << msg << " Error: " << SDL_GetError() << endl;
    if (fatal) {
        SDL_Quit();
        exit(1);
    }
}
void initSDL(SDL_Window* &window, SDL_Renderer* &renderer){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        logSDLError(cout, "SDL_Init", true);

    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) logSDLError(cout, "CreateWindow", true);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) logSDLError(cout, "CreateRenderer", true);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}
void quitSDL(SDL_Window* window, SDL_Renderer* renderer){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
Uint32 callback( Uint32 interval, void* param )
{
    bool *check = (bool*) param;
    *check = 1;
    return 0;
}
//-----------------------------------------------------------------------------------------

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *renderer)
{
    SDL_Texture *texture = nullptr;
	//nap anh tu ten file (voi duong dan)
	SDL_Surface *loadedImage = IMG_Load(file.c_str());
	//Neu khong co loi, chuyen doi ve dang texture and va tra ve
	if (loadedImage != nullptr){
		texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
		//dam bao viec chuyen doi khong co loi
		if (texture == nullptr){
			logSDLError(std::cout, "CreateTextureFromSurface");
		}
	}
	else {
		logSDLError(std::cout, "LoadBMP");
	}
	return texture;
}
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h)
{
	//Thiet lap hinh chu nhat dich ma chung ta muon ve anh vao trong
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
    dst.w = w;
    dst.h = h;
    //Dua toan bo anh trong texture vao hinh chu nhat dich
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);

	SDL_RenderCopy(ren, tex, NULL, &dst);
}
//-----------------------------------------------------------------------------------------

void draw_map(SDL_Renderer* renderer, int **mat, SDL_Texture *img[], SDL_Texture *bg1, SDL_Texture *bg2, bool *ok){
    SDL_RenderClear(renderer);
    renderTexture(bg1, renderer, 0, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT);
    renderTexture(bg2, renderer, SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT);
    for (int i = 0; i < MAX_MAP_Y; i++){
        for (int j = 0; j < MAX_MAP_X; j++){
            int pos_y = i*TILE_SIZE;
            int pos_x = j*TILE_SIZE;
            switch (mat[i][j]) {
                case 1: {
                    renderTexture(img[1], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 2: {
                    if(!ok[2])
                        renderTexture(img[1], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 3: {
                    renderTexture(img[6], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 4: {
                    if(ok[4])
                        renderTexture(img[2], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 5: {
                    if(ok[5])
                        renderTexture(img[3], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 6: {
                    if(ok[6])
                        renderTexture(img[4], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 7: {
                    if(ok[7])
                        renderTexture(img[5], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 8: {
                   // renderTexture(img[7], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE*2);
                    if(ok[8])
                        renderTexture(img[8], renderer, pos_x+TILE_SIZE/4, pos_y-TILE_SIZE+15, TILE_SIZE/2, TILE_SIZE-15);
                    break;
                }
                case 9: {
                    if(!ok[9])
                        renderTexture(img[1], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
                case 10: {
                    renderTexture(img[4], renderer, pos_x, pos_y, TILE_SIZE, TILE_SIZE);
                    break;
                }
            }
        }

    }
}

//-----------------------------------------------------------------------------------------
void show_dango(SDL_Renderer* renderer, SDL_Rect *rect_, int *x_pos_, int *y_pos_, SDL_Texture *dango) {
    rect_ -> x = *x_pos_ ;
    rect_ -> y = *y_pos_ ;
    SDL_Rect renderQuad = {rect_ -> x, rect_ -> y, rect_ -> w, rect_ -> h};
    SDL_RenderCopy(renderer, dango, NULL, &renderQuad);
}
void checkmap(int **mat, bool *ok, int *x_pos_, int *y_pos_, int *x_val_, int *y_val_, SDL_Rect *rect_, bool *on_ground) {
    int x1 = 0;
    int x2 = 0;
    int y1 = 0;
    int y2 = 0;
    x1 = (*x_pos_ + *x_val_) / TILE_SIZE;
    x2 = (*x_pos_ + *x_val_ + rect_ -> w - 1) / TILE_SIZE;
    y1 = (*y_pos_) / TILE_SIZE;
    y2 = (*y_pos_ + rect_ -> h - 1) / TILE_SIZE;
    if (x1 >= 0 && x2 < MAX_MAP_X && y1 >= 0 && y2 < MAX_MAP_Y) {
        if (*x_val_ > 0) {
            if (mat[y1][x2] == 1 || mat[y2][x2] == 1 || ((mat[y1][x2] == 2 || mat[y2][x2] == 2) && ok[2] == 0) || ((mat[y1][x2] == 9 || mat[y2][x2] == 9)&& ok[9] == 0) ) {
                *x_pos_ = x2 * TILE_SIZE;
                *x_pos_ -= rect_ -> w;
                *x_val_ = 0;
            }
        }
        else if (*x_val_ < 0) {
            if (mat[y1][x1] == 1 || mat[y2][x1] == 1 || ((mat[y1][x1] == 2 || mat[y2][x1] == 2) && ok[2] == 0) || ((mat[y1][x1] == 9 || mat[y2][x1] == 9) && ok[9] == 0)) {
                *x_pos_ = (x1 + 1) * TILE_SIZE;
                *x_val_ = 0;
            }
        }
    }
    x1 = *x_pos_ / TILE_SIZE;
    x2 = (*x_pos_ + rect_ -> w - 1) / TILE_SIZE;
    y1 = (*y_pos_ + *y_val_) / TILE_SIZE;
    y2 = (*y_pos_ + *y_val_ + rect_ -> h - 1) / TILE_SIZE;

    if (x1 >= 0 && x2 < MAX_MAP_X && y1 >= 0 && y2 < MAX_MAP_Y) {
        if (*y_val_ > 0) {
            if (mat[y2][x1] == 1 || mat[y2][x2] == 1 || ((mat[y2][x1] == 2 || mat[y2][x2] == 2) && ok[2] == 0) || ((mat[y1][x1] == 9 || mat[y2][x1] == 9) && ok[9] == 0)) {
                *y_pos_ = y2 * TILE_SIZE;
                *y_pos_ -= rect_ -> h;
                *y_val_ = 0;
                *on_ground = true;

            }
        }
        else if (*y_val_ < 0) {
            if (mat[y1][x1] == 1 || mat[y1][x2] == 1 || ((mat[y1][x1] == 2 || mat[y1][x2] == 2) && ok[2] == 0) || ((mat[y1][x1] == 9 || mat[y2][x1] == 9) && ok[9] == 0)) {
                *y_pos_ = (y1 + 1) * TILE_SIZE;
                *y_val_ = 0;
            }
        }
    }
    *x_pos_ += *x_val_;
    *y_pos_ += *y_val_;
    if (*x_pos_ < 0)
        *x_pos_ = 0;
    else if (*x_pos_ + rect_ -> w >= SCREEN_WIDTH) {
        *x_pos_ = SCREEN_WIDTH - rect_ -> w - 1;
    }
    if (*y_pos_ < 0)
        *y_pos_ = 0;
    else if (*y_pos_ + rect_ -> h >= SCREEN_HEIGHT) {
        *y_pos_ = SCREEN_HEIGHT - rect_ -> h - 1;
    }
}
void trap(int **mat, bool *ok, int x_pos_, int y_pos_) {
    bool *p = &ok[mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE]];
    SDL_TimerID timerID = SDL_AddTimer( 700, callback, p );
}
void trap1(int **mat, bool *ok, int x_pos_, int y_pos_) {
    bool *p = &ok[mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE]];
    SDL_TimerID timerID = SDL_AddTimer( 1000, callback, p );
}
void do_player(int **mat, int *x_pos_, int *y_pos_, int *x_val_, int *y_val_, bool *on_ground, Input *input_type_, bool *ok, SDL_Rect *rect_) {
    *x_val_ = 0;
    *y_val_ += 1;
    if (*y_val_ >= MAX_FAIL_SPEED)
        *y_val_ = MAX_FAIL_SPEED;
    if (input_type_ -> left_ == 1) {
        *x_val_ -= PLAYER_SPEED;
    }

    else if (input_type_ -> right_ == 1) {
        *x_val_ += PLAYER_SPEED;
    }
    if (input_type_ -> up_ == 1) {
        if (*y_pos_ <= 500)
            *y_val_ = -PLAYER_SPEED;
    }
    checkmap(mat, ok, x_pos_, y_pos_, x_val_, y_val_, rect_, on_ground);
}

void input_action(SDL_Event events, Input *input_type_ ) {
    if (events.type == SDL_KEYDOWN) {
        switch (events.key.keysym.sym) {
            case SDLK_RIGHT: {

                input_type_ -> right_ = 1;
                input_type_ -> left_ = 0;
                break;
            }
            case SDLK_LEFT: {
                input_type_ -> left_ = 1;
                input_type_ -> right_ = 0;
                break;
            }
            case SDLK_UP: {
                input_type_ -> up_ = 1;
                break;
            }
        }
    }
    else if (events.type == SDL_KEYUP) {
        switch (events.key.keysym.sym) {
            case SDLK_RIGHT: {
                input_type_ -> right_ = 0;
                break;
            }
            case SDLK_LEFT: {
                input_type_ -> left_ = 0;
                break;
            }
            case SDLK_UP: {
                input_type_ -> up_ = 0;
                break;
            }
        }
    }
}

void play(SDL_Renderer* renderer, int **mat, SDL_Texture *img[], SDL_Texture *bg1, SDL_Texture *bg2, SDL_Texture *dango,bool *ok, SDL_Rect *rect_) {
    int x_val_ = 0;
    int y_val_ = 0;
    int x_pos_ = 0;
    int y_pos_ = 450;
    bool on_ground = false;
    Input input_type_;
    input_type_.left_ = 0;
    input_type_.right_ = 0;
    input_type_.down_ = 0;
    input_type_.up_ = 0;
    bool is_quit = false;
    while (!is_quit) {
        while (SDL_PollEvent(&gEvent) != 0) {
            if (gEvent.type == SDL_QUIT){
                is_quit = true;
                return;
            }

            input_action(gEvent, &input_type_);
        }
        SDL_RenderClear(renderer);
        do_player(mat, &x_pos_, &y_pos_, &x_val_, &y_val_, &on_ground, &input_type_, ok, rect_);
        if(y_pos_ > 550) {
            SDL_Delay(50);
            SDL_RenderClear(renderer);
            SDL_Texture *die = loadTexture("img/die.png", renderer);
            renderTexture(die, renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_RenderPresent(renderer);
            SDL_Delay(1000);
            return;
        }
        if((y_pos_)/TILE_SIZE == 9 && (x_pos_)/TILE_SIZE == 0 )
            trap1(mat, ok, x_pos_, y_pos_);
        if(mat[(y_pos_+35)/TILE_SIZE][x_pos_/TILE_SIZE] == 2 || mat[(y_pos_+35)/TILE_SIZE][x_pos_/TILE_SIZE] == 9)
            trap(mat, ok, x_pos_, y_pos_+35);
        if(mat[y_pos_/TILE_SIZE][(x_pos_+35)/TILE_SIZE] == 2 || mat[y_pos_/TILE_SIZE][(x_pos_+35)/TILE_SIZE] == 9)
            trap(mat, ok, x_pos_+35, y_pos_);
        if(mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE] > 1 && mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE] != 3)
            trap(mat, ok, x_pos_, y_pos_);

        draw_map(renderer, mat, img, bg1, bg2, ok);
        show_dango(renderer, rect_, &x_pos_, &y_pos_, dango);
        SDL_RenderPresent(renderer);
        if (ok[mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE]] == 1 && mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE] != 2 && mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE] != 9){


            draw_map(renderer, mat, img, bg1, bg2, ok);
            show_dango(renderer, rect_, &x_pos_, &y_pos_, dango);
            SDL_RenderPresent(renderer);
            SDL_Delay(300);
            SDL_RenderClear(renderer);
            SDL_Texture *die = loadTexture("img/die.png", renderer);
            renderTexture(die, renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_RenderPresent(renderer);
            SDL_Delay(1000);
            return;
        }
        if(mat[y_pos_/TILE_SIZE][x_pos_/TILE_SIZE] == 3){
            SDL_Delay(50);
            SDL_RenderClear(renderer);
            SDL_Texture *win = loadTexture("img/win.png", renderer);
            renderTexture(win, renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_RenderPresent(renderer);
            SDL_Delay(1000);
            return;
        }

    }
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{

    SDL_Window* window;
    SDL_Renderer* renderer;
    initSDL(window, renderer);
//

    SDL_Rect rect_;
    rect_.x = 0;
    rect_.y = 450;
    rect_.h = 30;
    rect_.w = 30;

    SDL_Texture *img[9];
    img[1] = loadTexture("img/gach.png", renderer);
    img[2] = loadTexture("img/gai.png", renderer);
    img[3] = loadTexture("img/gai1.png", renderer);
    img[4] = loadTexture("img/gai2.png", renderer);
    img[5] = loadTexture("img/gai3.png", renderer);
    img[6] = loadTexture("img/co.png", renderer);
    img[7] = loadTexture("img/ong.png", renderer);
    img[8] = loadTexture("img/hoa.png", renderer);
    bool ok[11];
    memset(ok, 0, sizeof(ok));
    ok[10] = 1;
    SDL_RenderClear(renderer);

    freopen("bg/map.dat", "r", stdin);
    int **mat;
    mat = new int *[25];
    for (int i = 0; i < MAX_MAP_Y; i++){
        mat[i] = new int[25];
        for (int j = 0; j < MAX_MAP_X; j++){
            cin >> mat[i][j];
        }
    }
    SDL_Texture *welcome = loadTexture("welcome.png", renderer);
    renderTexture(welcome, renderer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_RenderPresent(renderer);
    SDL_Event e;
    while (true) {
        if ( SDL_WaitEvent(&e) != 0 ){
            if(e.type == SDL_KEYDOWN) {
                SDL_RenderClear(renderer);
                SDL_Texture *bg1 = loadTexture("bg1_1.png", renderer);
                SDL_Texture *bg2 = loadTexture("bg1_2.png", renderer);
                SDL_Texture *dango = loadTexture("img/dango.png", renderer);
                draw_map(renderer, mat, img, bg1, bg2, ok);
                renderTexture(dango, renderer, 0, 400, 30, 30);
                SDL_RenderPresent(renderer);
                play(renderer, mat, img, bg1, bg2, dango, ok, &rect_);
                SDL_DestroyTexture(bg1);
                SDL_DestroyTexture(bg2);
                SDL_DestroyTexture(dango);
            }
            memset(ok, 0, sizeof(ok));
            ok[10] = 1;
            if(e.type == SDL_QUIT ){
                quitSDL(window, renderer);
                break;
            }

        }

        SDL_Delay(100);

    }

    quitSDL(window, renderer);
    return 0;
}
