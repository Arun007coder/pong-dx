#include "pong.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define BUILD_DEBUG 0

void* zalloc(uint32_t size)
{
    void* r = malloc(size);
    memset(r, 0, size);

    return r;
}

#define ZALLOC_TYPES(type) zalloc(sizeof(type))

static char title[512];

typedef struct key_pair_struct
{
    char key[256];
    char value[256];
}key_pair_t;

int parse_key_value_pair(const char *line, key_pair_t *kv)
{
    char *delimiter = "=";
    char *token = strtok(line, delimiter);
    
    if (token == NULL)
    {
        return 0;
    }
    
    char *keyStart = token;
    char *keyEnd = token + strlen(token) - 1;
    while (*keyStart == ' ')
    {
        keyStart++;
    }
    while (*keyEnd == ' ' || *keyEnd == '\n' || *keyEnd == '\r')
    {
        *keyEnd = '\0';
        keyEnd--;
    }

    strncpy(kv->key, keyStart, sizeof(kv->key));
    kv->key[sizeof(kv->key) - 1] = '\0';
    
    token = strtok(NULL, delimiter);
    if (token == NULL)
    {
        return 0;
    }
    
    char *valueStart = token;
    char *valueEnd = token + strlen(token) - 1;
    while (*valueStart == ' ')
    {
        valueStart++;
    }
    while (*valueEnd == ' ' || *valueEnd == '\n' || *valueEnd == '\r')
    {
        *valueEnd = '\0';
        valueEnd--;
    }
    strncpy(kv->value, valueStart, sizeof(kv->value));
    kv->value[sizeof(kv->value) - 1] = '\0';
    
    return 1;
}

int pong_load_config(pong_inst_t* pi, char* file)
{
    FILE* cf = fopen(file, "r");
    key_pair_t kp;

    if(cf == NULL)
        return 1;

    char line[256];
    while (fgets(line, sizeof(line), cf) != NULL)
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
        {
            continue;
        }
        
        if (parse_key_value_pair(line, &kp))
        {
            if(strcmp(kp.key, "player1") == 0)
            {
                if(strcmp(kp.value, "bot") == 0)
                {
                    pi->flags &= ~(PONG_FLAGS_PLAYER1_STATUS);
                }
                else
                {
                    pi->flags |= PONG_FLAGS_PLAYER1_STATUS;
                }
            }
            else if(strcmp(kp.key, "player2") == 0)
            {
                if(strcmp(kp.value, "bot") == 0)
                {
                    pi->flags &= ~(PONG_FLAGS_PLAYER2_STATUS);
                }
                else
                {
                    pi->flags |= PONG_FLAGS_PLAYER2_STATUS;
                }
            }
            else if(strcmp(kp.key, "random_speed") == 0)
            {
                if(strcmp(kp.value, "true") == 0)
                {
                    pi->flags |= PONG_FLAGS_RANDOM_SPEED;
                }
                else
                {
                    pi->flags &= ~(PONG_FLAGS_RANDOM_SPEED);
                }
            }
            else if(strcmp(kp.key, "use_mouse") == 0)
            {
                if(strcmp(kp.value, "true") == 0)
                {
                    pi->flags |= PONG_FLAGS_USE_MOUSE;
                }
                else
                {
                    pi->flags &= ~(PONG_FLAGS_USE_MOUSE);
                }
            }
#if BUILD_DEBUG
            printf("Key: %s, Value: %s\n", kp.key, kp.value);
#endif
        }
    }
    
    fclose(cf);

    return 0;
}

int init_pong(pong_inst_t* pi)
{
    uint32_t* fb = zalloc(WIN_WIDTH*WIN_HEIGHT*sizeof(uint32_t));

    pi->play_field = canvas_create(WIN_WIDTH, WIN_HEIGHT-100, fb);
    pi->og_play_field = canvas_create(WIN_WIDTH, WIN_HEIGHT, fb);

    pi->status.r = rect_create(0, WIN_HEIGHT-100, WIN_WIDTH, 100);
    pi->status.region = malloc(pi->status.r.height*pi->status.r.width*sizeof(uint32_t));
    pi->status_canvas = canvas_create(pi->status.r.width, pi->status.r.height, pi->status.region);

    pi->p1_paddle = rect_create(0, 0, PADDLE_WIDTH, PADDLE_HEIGHT);
    pi->p2_paddle = rect_create(WIN_WIDTH-PADDLE_WIDTH, 0, PADDLE_WIDTH, PADDLE_HEIGHT);

    pi->ball = rect_create(WIN_WIDTH/2, WIN_HEIGHT/2, BALL_HEIGHT, BALL_WIDTH);

    pi->paddle_speed = 8;
    pi->ball_speed = 4;
    pi->ball_dx = pi->ball_speed;
    pi->ball_dy = pi->ball_speed;

    pi->flags = 0;

    if(pong_load_config(pi, "./pong.cfg") == 1)
    {
        if(pong_load_config(pi, "~/pong.cfg") == 1)
        {
            // Uncomment this if you want to see the bot play two players
            // pi->flags &= ~(PONG_FLAGS_PLAYER1_STATUS | PONG_FLAGS_PLAYER2_STATUS);

            // Uncomment this if player 1 is controlled by human
            pi->flags |= PONG_FLAGS_PLAYER1_STATUS;

            // Uncomment this if player 2 is controlled by human
            // pi->flags |= PONG_FLAGS_PLAYER2_STATUS;

            pi->flags &= ~(PONG_FLAGS_PAUSED);
            pi->flags |= PONG_FLAGS_RANDOM_SPEED;
            pi->flags |= PONG_FLAGS_USE_MOUSE;

            printf("Config file not found. Using default configuration.\n");
        }
    }

    if((pi->flags & PONG_FLAGS_USE_MOUSE) == PONG_FLAGS_USE_MOUSE)
    {
        pong_lock_mouse(1);
    }

    set_fill_color(PADDLE_COLOR);
    draw_rect2(&pi->play_field, &pi->p1_paddle);

    set_fill_color(PADDLE_COLOR);
    draw_rect2(&pi->play_field, &pi->p2_paddle);

    set_fill_color(BALL_COLOR);
    draw_rect2(&pi->play_field, &pi->ball);

    pong_change_title("pong");

    draw_play_field(pi);
}

int pong_control_bot(pong_inst_t* pong)
{
    if((pong->flags & PONG_FLAGS_PLAYER1_STATUS) != PONG_FLAGS_PLAYER1_STATUS)
    {
        int ball_center_x = pong->ball.y + pong->ball.height / 2;
        int paddle_center_y = pong->p1_paddle.y + pong->p1_paddle.height / 2;

        if (ball_center_x < paddle_center_y)
        {
            pong->p1_dy = -pong->paddle_speed;
        }
        else if (ball_center_x > paddle_center_y)
        {
            pong->p1_dy = pong->paddle_speed;
        }
        else
        {
            pong->p1_dy = 0;
        }
    }

    if((pong->flags & PONG_FLAGS_PLAYER2_STATUS) != PONG_FLAGS_PLAYER2_STATUS)
    {
        int ball_center_x = pong->ball.y + pong->ball.height / 2;
        int paddle_center_y = pong->p2_paddle.y + pong->p2_paddle.height / 2;

        if (ball_center_x < paddle_center_y)
        {
            pong->p2_dy = -pong->paddle_speed;
        }
        else if (ball_center_x > paddle_center_y)
        {
            pong->p2_dy = pong->paddle_speed;
        }
        else
        {
            pong->p2_dy = 0;
        }
    }

    return 0;
}

int pong_update_playfield(pong_inst_t* pi)
{
    set_fill_color(rgb(0,0,0));
    canvas_clear(&pi->play_field);

    set_fill_color(rgb(255,255,102));
    canvas_clear(&pi->status_canvas);

    int st_x = 20;
    int st_y = 30;

    for(int i = 0; i < 32; i++)
    {
        int f = (pi->flags & (1 << i));

        if(f)
        {
            set_fill_color(VESA_COLOR_GREEN);
        }
        else
        {
            set_fill_color(VESA_COLOR_RED);
        }

        if(st_x >= pi->status_canvas.width)
        {
            st_y += 40;
            st_x = 20;
        }

        draw_rect(&pi->status_canvas, st_x, st_y, 20, 20);
        st_x += 30;
    }

    draw_rect_pixels(&pi->og_play_field, &pi->status);

    for(int y = 0; y < (pi->play_field.height / 16); y++)
    {
        set_fill_color(VESA_COLOR_WHITE);
        draw_rect(&pi->play_field, pi->play_field.width/2, y*16, 10, 10);
    }

    set_fill_color(PADDLE_COLOR);
    draw_rect2(&pi->play_field, &pi->p1_paddle);

    set_fill_color(PADDLE_COLOR);
    draw_rect2(&pi->play_field, &pi->p2_paddle);

    set_fill_color(BALL_COLOR);
    draw_rect2(&pi->play_field, &pi->ball);

    draw_play_field(pi);
}

int rand_range(int min, int max)
{
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }

    return min + rand() % (max - min + 1);
}

int pong_get_flag(pong_inst_t* pi, int flag)
{
    return ((pi->flags & flag) == flag);
}

int press = 0;
uint8_t scancode = 0;

int pong_tick(pong_inst_t* pi)
{
    if((pi->flags & PONG_FLAGS_PAUSED) != PONG_FLAGS_PAUSED)
    {
        if(pong_get_keys(&press, &scancode))
        {
            if(press)
            {
                switch(scancode)
                {
                case PONG_KB_UP:
                {
                    if(((pi->flags & PONG_FLAGS_USE_MOUSE) != PONG_FLAGS_USE_MOUSE))
                        pi->p1_dy = -1 * (pi->paddle_speed * (pi->p1_modifier ? 2 : 1));
                }break;
                case PONG_KB_DOWN:
                {
                    if(((pi->flags & PONG_FLAGS_USE_MOUSE) != PONG_FLAGS_USE_MOUSE))
                        pi->p1_dy = 1 * (pi->paddle_speed * (pi->p1_modifier ? 2 : 1));
                }break;
                case PONG_KB_ALT_UP:
                {
                    pi->p2_dy = -1 * (pi->paddle_speed * (pi->p2_modifier ? 2 : 1));
                }break;
                case PONG_KB_ALT_DOWN:
                {
                    pi->p2_dy = 1 * (pi->paddle_speed * (pi->p2_modifier ? 2 : 1));
                }break;
                case PONG_KB_PAUSE:
                {
                    if((pi->flags & PONG_FLAGS_PAUSED) == PONG_FLAGS_PAUSED)
                    {
                        pi->flags &= ~(PONG_FLAGS_PAUSED);
                    }
                    else
                    {
                        pi->flags |= PONG_FLAGS_PAUSED;
                    }
                }break;
                case PONG_KB_RESET:
                {
                    if( (((pi->flags & PONG_FLAGS_PLAYER2_STATUS) != PONG_FLAGS_PLAYER2_STATUS) && ((pi->flags & PONG_FLAGS_PLAYER1_STATUS) != PONG_FLAGS_PLAYER1_STATUS)) || pong_get_flag(pi, PONG_FLAGS_RANDOM_SPEED) )
                    {
                        pi->ball.x = pi->play_field.width / 2;
                        pi->ball.y = pi->play_field.height / 2;

                        pi->ball_dx = rand_range(1, 12);
                        pi->ball_dy = rand_range(1, 12);

                        pi->paddle_speed = rand_range(1, 12);

#if BUILD_DEBUG
                        printf("Paddle speed: %d\n", pi->paddle_speed);
                        printf("Ball dx: %d\n", pi->ball_dx);
                        printf("Ball dy: %d\n", pi->ball_dy);
#endif
                    }
                    else
                    {
                        pi->ball.x = pi->play_field.width / 2;
                        pi->ball.y = pi->play_field.height / 2;


                        pi->ball_dx = pi->ball_speed;
                        pi->ball_dy = pi->ball_speed;
                    }
                }break;
                case PONG_KB_MODIFIER:
                {
                    if(((pi->flags & PONG_FLAGS_USE_MOUSE) != PONG_FLAGS_USE_MOUSE))
                        pi->p1_modifier = 1;
                }break;
                case PONG_KB_ALT_MODIFIER:
                {
                    pi->p2_modifier = 1;
                }break;
                case PONG_KB_EXIT:
                {
                    exit(0);
                }break;
                };
            }
            else
            {
                if(scancode == PONG_KB_MODIFIER)
                    if(((pi->flags & PONG_FLAGS_USE_MOUSE) != PONG_FLAGS_USE_MOUSE))
                        pi->p1_modifier = 0;
                else if(scancode == PONG_KB_ALT_MODIFIER)
                    pi->p2_modifier = 0;
            }
        }

        pong_control_bot(pi);

        if((sign(pi->p1_dy) == -1) && (pi->p1_paddle.y > pi->paddle_speed))
            pi->p1_paddle.y += pi->p1_dy;
        if((sign(pi->p1_dy) == 1) && ((pi->p1_paddle.y + pi->p1_paddle.height) < pi->play_field.height))
            pi->p1_paddle.y += pi->p1_dy;
        if((sign(pi->p2_dy) == -1) && (pi->p2_paddle.y > pi->paddle_speed))
            pi->p2_paddle.y += pi->p2_dy;
        if((sign(pi->p2_dy) == 1) && ((pi->p2_paddle.y + pi->p2_paddle.height) < pi->play_field.height))
            pi->p2_paddle.y += pi->p2_dy;
            
        pi->p1_dy = pi->p2_dy = 0;

        pi->ball.x += pi->ball_dx;
        pi->ball.y += pi->ball_dy;

        if (pi->ball.y <= 0)
        {
            pi->ball.y = 0;
            pi->ball_dy = -pi->ball_dy;
        }
        else if (pi->ball.y + pi->ball.height >= pi->play_field.height)
        {
            pi->ball.y = pi->play_field.height - pi->ball.height;
            pi->ball_dy = -pi->ball_dy;
        }

        if(is_rect_overlap(pi->ball, pi->p1_paddle) || is_rect_overlap(pi->ball, pi->p2_paddle))
        {
            pi->ball_dx = -pi->ball_dx;
        }

        if (pi->ball.x < 0)
        {
            pi->p2_score++;
            pi->ball.x = pi->play_field.width / 2;
            pi->ball.y = pi->play_field.height / 2;

            if( (((pi->flags & PONG_FLAGS_PLAYER2_STATUS) != PONG_FLAGS_PLAYER2_STATUS) && ((pi->flags & PONG_FLAGS_PLAYER1_STATUS) != PONG_FLAGS_PLAYER1_STATUS)) || pong_get_flag(pi, PONG_FLAGS_RANDOM_SPEED))
            {
                pi->ball_dx = rand_range(1, 12);
                pi->ball_dy = rand_range(1, 12);

                pi->paddle_speed = rand_range(1, 12);

#if BUILD_DEBUG
                printf("Paddle speed: %d\n", pi->paddle_speed);
                printf("Ball dx: %d\n", pi->ball_dx);
                printf("Ball dy: %d\n", pi->ball_dy);
#endif
            }
            else
            {

                pi->ball_dx = pi->ball_speed;
                pi->ball_dy = pi->ball_speed;
            }

            sprintf(title, "Pong P1: %d, P2: %d", pi->p1_score, pi->p2_score);
            pong_change_title(title);
        }
        else if (pi->ball.x + pi->ball.width > pi->play_field.width)
        {
            pi->p1_score++;
            pi->ball.x = pi->play_field.width / 2;
            pi->ball.y = pi->play_field.height / 2;
            
            if( ((((pi->flags & PONG_FLAGS_PLAYER2_STATUS) != PONG_FLAGS_PLAYER2_STATUS) && ((pi->flags & PONG_FLAGS_PLAYER1_STATUS) != PONG_FLAGS_PLAYER1_STATUS))) || pong_get_flag(pi, PONG_FLAGS_RANDOM_SPEED) )
            {
                pi->ball_dx = rand_range(1, 12);
                pi->ball_dy = rand_range(1, 12);

                pi->paddle_speed = rand_range(1, 12);

#if BUILD_DEBUG
                printf("Paddle speed: %d\n", pi->paddle_speed);
                printf("Ball dx: %d\n", pi->ball_dx);
                printf("Ball dy: %d\n", pi->ball_dy);
#endif
            }
            else
            {    
                pi->ball_dx = pi->ball_speed;
                pi->ball_dy = pi->ball_speed;
            }

            sprintf(title, "Pong P1: %d, P2: %d", pi->p1_score, pi->p2_score);
            pong_change_title(title);
        }

        pong_update_playfield(pi);
    }
    else
    {
        if(pong_get_keys(&press, &scancode))
        {
            if(press)
            {
                switch(scancode)
                {
                case PONG_KB_PAUSE:
                {
                    if((pi->flags & PONG_FLAGS_PAUSED) == PONG_FLAGS_PAUSED)
                    {
                        pi->flags &= ~(PONG_FLAGS_PAUSED);
                    }
                    else
                    {
                        pi->flags |= PONG_FLAGS_PAUSED;
                    }
                }break;
                case PONG_KB_RESET:
                {
                    if( (((pi->flags & PONG_FLAGS_PLAYER2_STATUS) != PONG_FLAGS_PLAYER2_STATUS) && ((pi->flags & PONG_FLAGS_PLAYER1_STATUS) != PONG_FLAGS_PLAYER1_STATUS)) && pong_get_flag(pi, PONG_FLAGS_RANDOM_SPEED) )
                    {
                        pi->ball.x = pi->play_field.width / 2;
                        pi->ball.y = pi->play_field.height / 2;

                        pi->ball_dx = rand_range(1, 12);
                        pi->ball_dy = rand_range(1, 12);

                        pi->paddle_speed = rand_range(1, 12);

                        printf("Paddle speed: %d\n", pi->paddle_speed);
                        printf("Ball dx: %d\n", pi->ball_dx);
                        printf("Ball dy: %d\n", pi->ball_dy);
                    }
                    else
                    {
                        pi->ball.x = pi->play_field.width / 2;
                        pi->ball.y = pi->play_field.height / 2;

                        pi->ball_dx = -pi->ball_speed;
                        pi->ball_dy = pi->ball_speed;
                    }
                }break;
                case PONG_KB_EXIT:
                {
                    exit(0);
                }break;
                };
            }
        }
    }
}