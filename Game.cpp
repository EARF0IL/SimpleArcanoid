#include "Engine.h"
#include <stdlib.h>
#include <memory.h>
#include <utility>
#include <math.h>

//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, 'A', 'B')
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button)
//  clear_buffer() - set all pixels in buffer to 'black'
//  is_window_active() - returns true if window is active
//  schedule_quit_game() - quit game after act()

enum class CollideType {
    VERTICAL,
    HORIZONTAL,
    NOT_COLLIDE
};

struct GameObject {
    std::pair<float, float> pos;
    std::pair<int, int> size;
    std::pair<short, short> velocity;
    float speed;
    uint32_t color;

    CollideType check_collide(const GameObject& other) {
        // Check top collide
        if (pos.first + size.first > other.pos.first and pos.first < other.pos.first + other.size.first) {
            if (pos.second + size.second > other.pos.second and pos.second < other.pos.second + other.size.second)
                return CollideType::HORIZONTAL;
        }
        else if (pos.second + size.second > other.pos.second and pos.second < other.pos.second + other.size.second) {
            if (pos.first + size.first < other.pos.first and pos.first > other.pos.first + other.size.first) {
                return CollideType::VERTICAL;
            }
        }
        return CollideType::NOT_COLLIDE;
    }

    void pos_update() {
        pos.first = std::max(std::min(pos.first + velocity.first * speed, static_cast<float>(SCREEN_WIDTH) - size.first), 0.f);
        pos.second = std::max(std::min(pos.second + velocity.second * speed, static_cast<float>(SCREEN_HEIGHT) - size.second), 0.f);
    }
};


struct Board : GameObject {

    Board() {
        pos = std::make_pair(512, 600);
        size = std::make_pair(120, 10);
        color = 0x0000FF;
        velocity = std::make_pair(0, 0);
        speed = 0.5f;
    }

};

struct Ball : GameObject {

    Ball() {
        pos = std::make_pair(545, 510);
        size = std::make_pair(10, 10);
        color = 0xFFFF00;
        speed = 0.3f;
        velocity = std::make_pair(1, -1);
    }

    void collide_borders() {
        // Vertical boards
        if (pos.first >= SCREEN_WIDTH - size.first) {
            velocity.first = -velocity.first;
            pos.first = SCREEN_WIDTH - size.first;
        }
        else if (pos.first <= 0) {
            velocity.first = -velocity.first;
            pos.first = 0;
        }

        // Horizontal boards
        if (pos.second >= SCREEN_HEIGHT - size.second) {
            velocity.second = -velocity.second;
            pos.second = SCREEN_HEIGHT - size.second;
        }
        else if (pos.second <= 0) {
            velocity.second = -velocity.second;
            pos.second = 0;
        }
    }
};

struct Brick : GameObject{
    bool is_alive;

    Brick() {
        pos = std::make_pair(20, 20);
        size = std::make_pair(62, 20);
        velocity = std::make_pair(0, 0);
        speed = 0.f;
        color = 0x00FF00;
        is_alive = true;
    }
};

struct Lava : GameObject {
    Lava() {
        pos = std::make_pair(0, SCREEN_HEIGHT - 30);
        size = std::make_pair(SCREEN_WIDTH, 30);
        velocity = std::make_pair(0, 0);
        speed = 0.f;
        color = 0xFF0000;
    }
};

Board play_board;
Ball ball;
Brick* bricks;
Lava lava;
int bricks_count;


void draw_game_object(const GameObject &game_object) {
    int begin_pos_x = static_cast<int>(game_object.pos.first);
    int end_pos_x = begin_pos_x + static_cast<int>(game_object.size.first);
    int begin_pos_y = static_cast<int>(game_object.pos.second);
    int end_pos_y = begin_pos_y + static_cast<int>(game_object.size.second);
    for (int y = begin_pos_y; y < end_pos_y; y++) {
        for (int x = begin_pos_x; x < end_pos_x; x++) {
            buffer[y][x] = game_object.color;
        }
    }
}

// initialize game data in this function
void initialize() {
    int brick_space = 2; // px
    int top_space = 50; // px
    int bricks_row_count = SCREEN_WIDTH / (62 + brick_space);
    int brick_rows = 4;
    bricks_count = bricks_row_count * brick_rows;
    bricks = new Brick[bricks_count];
    for (int i = 0; i < brick_rows; i++) {
        for (int j = 0; j < bricks_row_count; j++) {
            Brick *brick = &bricks[i * bricks_row_count + j];
            brick->pos = std::make_pair((brick_space + 62) * j, top_space + (brick_space + 20) * i);
        }
    }
}

void act(float dt)
{
    ball.collide_borders();
    CollideType collide_type = ball.check_collide(play_board);
    if (collide_type != CollideType::NOT_COLLIDE)
        ball.velocity.second = -1;
    for (int i = 0; i < bricks_count; i++) {
        if (!bricks[i].is_alive)
            continue;
        collide_type = ball.check_collide(bricks[i]);
        bricks[i].is_alive = false;
        switch (collide_type) {
            case CollideType::HORIZONTAL:
                ball.velocity.second = -ball.velocity.second;
                break;

            case CollideType::VERTICAL:
                ball.velocity.first = -ball.velocity.first;
                break;

            case CollideType::NOT_COLLIDE:
                bricks[i].is_alive = true;
        }
    }
    ball.pos_update();

    collide_type = ball.check_collide(lava);
    if (collide_type != CollideType::NOT_COLLIDE)
        schedule_quit_game();

    if (is_key_pressed(VK_ESCAPE))
        schedule_quit_game();
    else if (is_key_pressed(VK_LEFT))
        play_board.velocity.first = -1;
    else if (is_key_pressed(VK_RIGHT))
        play_board.velocity.first = 1;
    else
        play_board.velocity.first = 0;
    play_board.pos_update();
}

void draw()
{
    // clear backbuffer
    memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    draw_game_object(play_board);
    draw_game_object(ball);
    draw_game_object(lava);
    for (int i = 0; i < bricks_count; i++) {
        Brick brick = bricks[i];
        if (brick.is_alive)
            draw_game_object(brick);
    }
    
}

void finalize()
{
    delete[] bricks;
}

