#include <iostream>
#include <string>
#include <ctime>
#include <cmath>

#include "SDL.h"
#include "Game.h"

using namespace std;

Game::Game()
{
    for (int i = 0; i < GRID_WIDTH; ++i)
        for (int j = 0; j < GRID_HEIGHT; ++j)
        {
            grid[i][j] = Block::empty;
        }

    srand(static_cast<unsigned int>(time(0)));
}

void Game::Run()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    alive = true;
    running = true;
    ReplaceFood();
    GameLoop();
}

void Game::ReplaceFood()
{
    int x, y;
    while (true)
    {
        x = rand() % (GRID_WIDTH - 1);
        y = rand() % (GRID_HEIGHT - 1);

        if (grid[x][y] == Block::empty)
        {
            grid[x][y] = Block::food;
            food.x = x;
            food.y = y;
            break;
        }
    }
}

void Game::GameLoop()
{
    Uint32 before, second = SDL_GetTicks(), after;
    int frame_time, frames = 0;

    while (running)
    {
        before = SDL_GetTicks();

        PollEvents();
        Update();
        Render();

        frames++;
        after = SDL_GetTicks();
        frame_time = after - before;

        if (after - second >= 1000)
        {
            fps = frames;
            frames = 0;
            second = after;
            UpdateWindowTitle();
        }

        if (FRAME_RATE > frame_time)
        {
            SDL_Delay(FRAME_RATE - frame_time);
        }
    }

}

void Game::PollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            running = false;
        }
        else if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_UP:
                    if (last_dir != Move::down || size == 1)
                        dir = Move::up;
                    break;

                case SDLK_DOWN:
                    if (last_dir != Move::up || size == 1)
                        dir = Move::down;
                    break;

                case SDLK_LEFT:
                    if (last_dir != Move::right || size == 1)
                        dir = Move::left;
                    break;

                case SDLK_RIGHT:
                    if (last_dir != Move::left || size == 1)
                        dir = Move::right;
                    break;
            }
        }
    }
}

int Game::GetSize()
{
    return size;
}

void Game::GrowBody(int quantity)
{
    growing += quantity;
}

void Game::Update()
{
    if (!alive)
        return;

    switch (dir)
    {
        case Move::up:
            pos.y -= speed;
            pos.x = floorf (pos.x);
            break;

        case Move::down:
            pos.y += speed;
            pos.x = floorf (pos.x);
            break;

        case Move::left:
            pos.x -= speed;
            pos.y = floorf (pos.y);
            break;

        case Move::right:
            pos.x += speed;
            pos.y = floorf (pos.y);
            break;
    }

    if (pos.x < 0) pos.x = GRID_WIDTH - 1;
    else if (pos.x > GRID_WIDTH - 1) pos.x = 0;

    if (pos.y < 0) pos.y = GRID_HEIGHT - 1;
    else if (pos.y > GRID_HEIGHT - 1) pos.y = 0;

    int new_x = static_cast<int>(pos.x);
    int new_y = static_cast<int>(pos.y);

    if (new_x != head.x || new_y != head.y)
    {
        last_dir = dir;

        if (growing > 0)
        {
            size++;
            body.push_back(head);
            growing--;
            grid[head.x][head.y] = Block::body;
        }
        else
        {
            SDL_Point free = head;
            vector<SDL_Point>::reverse_iterator rit = body.rbegin();
            for ( ; rit != body.rend(); ++rit)
            {
                grid[free.x][free.y] = Block::body;
                swap(*rit, free);
            }

            grid[free.x][free.y] = Block::empty;
        }

    }

    head.x = new_x;
    head.y = new_y;

    Block & next = grid[head.x][head.y];
    if (next == Block::food)
    {
        score++;
        ReplaceFood();
        GrowBody(1);
    }
    else if (next == Block::body)
    {
        alive = false;
    }

    next = Block::head;
}

int Game::GetScore()
{
    return score;
}

void Game::UpdateWindowTitle()
{
    string title = "Snakle++ Score: " + to_string(score) + " FPS: " + to_string(fps);
    SDL_SetWindowTitle(window, title.c_str());
}

void Game::Render()
{
    int x, y, z;
    srand(time(0));
    x = rand() % (250 - 3 + 1) + 0;
    y = rand() % (250 - 3 + 1) + 0;
    z = rand() % (250 - 3 + 1) + 0;
    SDL_Rect block;
    block.w = SCREEN_WIDTH / GRID_WIDTH;
    block.h = SCREEN_WIDTH / GRID_HEIGHT;

    SDL_SetRenderDrawColor(renderer, 139, 105, 20, 0);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 0);
    block.x = food.x * block.w;
    block.y = food.y * block.h;
    SDL_RenderFillRect(renderer, &block);

    SDL_SetRenderDrawColor(renderer, 124, 252, 0, 1);
    for (SDL_Point & point : body)
    {
        block.x = point.x * block.w;
        block.y = point.y * block.h;
        SDL_RenderFillRect(renderer, &block);
    }

    block.x = head.x * block.w;
    block.y = head.y * block.h;
    if (alive) SDL_SetRenderDrawColor(renderer, 255, 105, 180, 0);
    else       SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    SDL_RenderFillRect(renderer, &block);

    SDL_RenderPresent(renderer);
}

void Game::Close()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}
