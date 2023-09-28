#if !defined(__PONG_H__)
#define __PONG_H__

#include "stdint.h"
#include "draw.h"

typedef struct pong_instance_struct
{
    canvas_t play_field;
    canvas_t og_play_field;
    rect_region_t status;
    canvas_t status_canvas;

    rect_t p1_paddle;
    rect_t p2_paddle;
    rect_t ball;

    uint32_t p1_score;
    uint32_t p2_score;

    int p1_dy;
    int p2_dy;
    int paddle_speed;

    int p1_modifier;
    int p2_modifier;

    uint32_t flags;

    int ball_dx;
    int ball_dy;
    int ball_speed;
}pong_inst_t;

#define WIN_WIDTH 500
#define WIN_HEIGHT 500

#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 60

#define BALL_WIDTH  10
#define BALL_HEIGHT 10

#define PADDLE_COLOR BIOS_COLOR10
#define BALL_COLOR BIOS_COLOR5

#define PONG_KB_UP 0x01
#define PONG_KB_DOWN 0x02
#define PONG_KB_ALT_UP 0x03
#define PONG_KB_ALT_DOWN 0x04
#define PONG_KB_PAUSE 0x05
#define PONG_KB_EXIT 0x06
#define PONG_KB_RESET 0x07
#define PONG_KB_MODIFIER 0x08
#define PONG_KB_ALT_MODIFIER 0x09

#define PONG_FLAGS_PLAYER1_STATUS (1 << 0)
#define PONG_FLAGS_PLAYER2_STATUS (1 << 1)
#define PONG_FLAGS_RANDOM_SPEED (1 << 2)
#define PONG_FLAGS_PAUSED (1 << 3)
#define PONG_FLAGS_USE_MOUSE (1 << 4)

int init_pong(pong_inst_t* pi);

void* zalloc(uint32_t size);

// Should be defined in client
void draw_play_field(pong_inst_t* pi);
int pong_get_keys(int* pressed, uint8_t* pong_scancode);
void pong_change_title(char* title);
void pong_lock_mouse(int state);

#endif // __PONG_H__
