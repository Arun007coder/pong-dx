#include "stdio.h"
#include "pong.h"
#include "SDL2/SDL.h"


#include "time.h"
#include "unistd.h"

#define PONG_SHOW_FRAMERATE 0

typedef struct inst_info_struct
{
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;
}inst_info_t;

static inst_info_t iinfo;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

uint8_t convert_keycode(uint32_t key)
{
    uint8_t ret = 0;

    switch(key)
    {
    case SDLK_w:
    {
        ret = PONG_KB_UP;
    }break;
    case SDLK_s:
    {
        ret = PONG_KB_DOWN;
    }break;
    case SDLK_i:
    {
        ret = PONG_KB_ALT_UP;
    }break;
    case SDLK_k:
    {
        ret = PONG_KB_ALT_DOWN;
    }break;
    case SDLK_r:
    {
        ret = PONG_KB_RESET;
    }break;
    case SDLK_ESCAPE:
    case SDLK_e:
    {
        ret = PONG_KB_EXIT;
    }break;
    case SDLK_SPACE:
    case SDLK_p:
    {
        ret = PONG_KB_PAUSE;
    }break;
    case SDLK_LSHIFT:
    {
        ret = PONG_KB_MODIFIER;
    }break;
    case SDLK_RSHIFT:
    {
        ret = PONG_KB_ALT_MODIFIER;
    }break;
    default:
    {
        ret = 0;
    }break;
    };

    return ret;
}


static void addKeyToQueue(int pressed, unsigned int keyCode)
{
    unsigned char key = convert_keycode(keyCode);

    if(key == 0)
        return;

    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

int pong_get_keys(int* pressed, uint8_t* keycode)
{
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
    {
        return 0;
    }
    else
    {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

        *pressed = keyData >> 8;
        *keycode = keyData & 0xFF;

        return 1;
    }
}

void pong_change_title(char* title)
{
    SDL_SetWindowTitle(iinfo.window, title);
}

int main()
{
    SDL_CreateWindowAndRenderer(WIN_WIDTH, WIN_HEIGHT, 0, &iinfo.window, &iinfo.renderer);
    SDL_CreateTexture(iinfo.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIN_WIDTH, WIN_HEIGHT);

    SDL_SetRenderDrawColor(iinfo.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

    pong_inst_t* pi = zalloc(sizeof(pong_inst_t));
    init_pong(pi);

    SDL_Event ev;

#if PONG_SHOW_FRAMERATE
    clock_t start_time, end_time;
    int frame_count = 0;
    double elapsed_time, frame_rate;

    start_time = clock();
#endif

    while(1)
    {
        while(SDL_PollEvent(&ev))
        {
            if(ev.type == SDL_QUIT)
            {
                SDL_Quit();
                exit(0);
            }
            else if(ev.type == SDL_KEYDOWN)
            {
                addKeyToQueue(1, ev.key.keysym.sym);
            }
            else if(ev.type == SDL_KEYUP)
            {
                addKeyToQueue(0, ev.key.keysym.sym);
            }

            if(ev.type == SDL_MOUSEMOTION)
            {
                pi->p1_dy = ev.motion.yrel*2;
            }
        }

        pong_tick(pi);

#if PONG_SHOW_FRAMERATE
        frame_count++;

        end_time = clock();
        elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        frame_rate = frame_count / elapsed_time;

        if (elapsed_time >= 1.0)
        {
            printf("Frame Rate: %.2f FPS\n", frame_rate);
            frame_count = 0;
            start_time = end_time;
        }
#endif

    }
}

void draw_play_field(pong_inst_t* pi)
{
    SDL_RenderClear(iinfo.renderer);

    for(int y = 0; y < pi->og_play_field.height; y++)
        for (int x = 0; x < pi->og_play_field.width; x++)
        {
            uint32_t c = pi->og_play_field.framebuffer[x + (y*WIN_WIDTH)];
            uint8_t r = GET_RED(c);
            uint8_t g = GET_GREEN(c);
            uint8_t b = GET_BLUE(c);

            SDL_SetRenderDrawColor(iinfo.renderer, r, g, b, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(iinfo.renderer, x, y);
        }

    SDL_RenderPresent(iinfo.renderer);
        
}

void pong_lock_mouse(int state)
{
    SDL_SetRelativeMouseMode(state);
}