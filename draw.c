#include "draw.h"
#include "stdlib.h"

uint32_t * screen = NULL;
uint32_t fill_color;
int screen_width, screen_height;

int get_pixel_idx(canvas_t * canvas, int x, int y)
{
    return canvas->width * y + x;
}

void set_pixel(canvas_t * canvas, uint32_t val, int x, int y)
{

    canvas->framebuffer[get_pixel_idx(canvas, x, y)] = val;
}

void set_fill_color(uint32_t color)
{
    fill_color = color;
}

void remove_sharp_edges(canvas_t * canvas, int start_x, int start_y, int direction, int num_pixels, uint32_t end_alpha, uint32_t middle_alpha)
{
    uint32_t idx, color, count, alpha;
    int i, j;
    i = start_y; j = start_x, count = num_pixels;
    while(count-- > 0)
    {
        idx = get_pixel_idx(canvas, j, i);
        color = canvas->framebuffer[idx];
        if(count == 0 || count == num_pixels - 1)
            alpha = end_alpha;
        else
            alpha = middle_alpha;
        set_pixel(canvas, SET_ALPHA(color, alpha), j, i);
        i++;
        j += direction;
    }
}

void round_corner_effect(canvas_t * canvas)
{
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3 - i; j++)
        {
            set_pixel(canvas, 0x0, j, i);
            set_pixel(canvas, 0x0, canvas->width - 3 + i + j, i);
        }
    }
    remove_sharp_edges(canvas, 0, canvas->width - 4, 1, 4, 0x88, 0xee);
    remove_sharp_edges(canvas, 0, canvas->width - 5, 1, 5, 0xff, 0xee);
    remove_sharp_edges(canvas, 1, canvas->width - 5, 1, 4, 0xff, 0xff);

    remove_sharp_edges(canvas, 0, 3, -1, 4, 0x88, 0xee);
    remove_sharp_edges(canvas, 0, 4, -1, 5, 0x88, 0xee);
    remove_sharp_edges(canvas, 1, 4, -1, 4, 0x88, 0xee);
}

void canvas_clear(canvas_t* canvas)
{
    uint32_t size = canvas->height * canvas->width;
    register uint32_t* fb = canvas->framebuffer;

    for(int i = 0; i < size; i++)
    {
        *fb++ = fill_color;
    }
}

void rect_offset_to_centre(rect_t* rect)
{
    int cx = rect->x + rect->width / 2;
    int cy = rect->y + rect->height / 2;

    rect->x = cx - rect->width / 2;
    rect->y = cy - rect->height / 2;
}

void draw_rect(canvas_t * canvas, int x, int y, int width, int height)
{
    register int i = 0;
    register int j = 0;
    for(i = y; i < y + height; i++)
    {
        for(j = x; j < x + width; j++)
        {
            set_pixel(canvas, fill_color, j, i);
        }
    }
}

void draw_rect_pixels(canvas_t * canvas, rect_region_t * rect_region)
{
    for(int i = rect_region->r.y;  i < rect_region->r.y + rect_region->r.height; i++)
    {
        for(int j = rect_region->r.x; j < rect_region->r.x + rect_region->r.width; j++)
        {
            int idx = rect_region->r.width * (i - rect_region->r.y) + (j - rect_region->r.x);
            if(rect_region->region[idx] != 0x0)
                set_pixel(canvas, rect_region->region[idx], j, i);
        }
    }
}

uint32_t* argb_to_brga(uint32_t* src, uint32_t width, uint32_t height)
{
    for(register int y = 0; y < height; y++)
    {
        for(register int x = 0; x < width; x++)
        {
            register uint8_t* buf = &(src[width * y + x]);
            uint8_t temp = buf[0];
            buf[0] = buf[2];
            buf[2] = temp;
        }
    }
    return src;
}

void control_brightness(uint8_t* fb, uint32_t sz, double brightness)
{
    for(uint32_t i = 0; i < sz; i++)
    {
        fb[i] = ((uint8_t)(fb[i]*brightness) & 0xFF);
    }
}

uint32_t bioscolor_to_vesa(uint32_t vesa)
{
    uint32_t ret = 0;
    switch(vesa)
    {
    case 0:
        ret = BIOS_COLOR0;
        break;
    case 1:
        ret = BIOS_COLOR1;
        break;
    case 2:
        ret = BIOS_COLOR2;
        break;
    case 3:
        ret = BIOS_COLOR3;
        break;
    case 4:
        ret = BIOS_COLOR4;
        break;
    case 5:
        ret = BIOS_COLOR5;
        break;
    case 6:
        ret = BIOS_COLOR6;
        break;
    case 7:
        ret = BIOS_COLOR7;
        break;
    case 8:
        ret = BIOS_COLOR8;
        break;
    case 9:
        ret = BIOS_COLOR9;
        break;
    case 10:
        ret = BIOS_COLOR10;
        break;
    case 11:
        ret = BIOS_COLOR11;
        break;
    case 12:
        ret = BIOS_COLOR12;
        break;
    case 13:
        ret = BIOS_COLOR13;
        break;
    case 14:
        ret = BIOS_COLOR14;
        break;
    case 15:
        ret = BIOS_COLOR15;
        break;
    default:
        ret = BIOS_COLOR0;
        break;
    };
    return ret;
}

void draw_rect_clip_pixels(canvas_t * canvas, rect_region_t * rect_region, int rect_true_width)
{
    for(int i = rect_region->r.y;  i < rect_region->r.y + rect_region->r.height; i++)
    {
        for(int j = rect_region->r.x; j < rect_region->r.x + rect_region->r.width; j++)
        {

            int idx = rect_true_width * (i - rect_region->r.y) + (j - rect_region->r.x);
            if(rect_region->region[idx] != 0x0)
                set_pixel(canvas, rect_region->region[idx], j, i);
        }
    }
}

void draw_rect_clip_pixels2(canvas_t * canvas, rect_region_t * rect_region, int rect_true_width, int rect_true_x, int rect_true_y)
{
    for(int i = rect_region->r.y;  i < rect_region->r.y + rect_region->r.height; i++)
    {
        for(int j = rect_region->r.x; j < rect_region->r.x + rect_region->r.width; j++)
        {
            int idx = rect_true_width * (i - rect_true_y) + (j - rect_true_x);
            if(rect_region->region[idx] != 0x0)
                set_pixel(canvas, rect_region->region[idx], j, i);
        }
    }
}


void draw_line(canvas_t * canvas, int x1, int y1, int x2, int y2)
{
    int dx = x2-x1;
    int dy = y2-y1;
    int dxabs = abs(dx);
    int dyabs = abs(dy);
    int sdx = sign(dx);
    int sdy = sign(dy);
    int x = 0;
    int y = 0;
    int px = x1;
    int py = y1;

    set_pixel(canvas, fill_color, px, py);
    if (dxabs>=dyabs)
    {
        for(int i=0;i<dxabs;i++)
        {
            y+=dyabs;
            if (y>=dxabs)
            {
                y-=dxabs;
                py+=sdy;
            }
            px+=sdx;
            set_pixel(canvas, fill_color, px, py);
        }
    }
    else
    {
        for(int i=0;i<dyabs;i++)
        {
            x+=dxabs;
            if (x>=dyabs)
            {
                x-=dyabs;
                px+=sdx;
            }
            py+=sdy;
            set_pixel(canvas, fill_color, px, py);
        }
    }
}

int is_line_overlap(int line1_x1, int line1_x2, int line2_x1, int line2_x2)
{
    int ret1 = line1_x1 < line2_x1 && (line1_x1 >= line2_x1 && line1_x2 <= line2_x2);
    int ret2 = (line1_x1 >= line2_x1 && line1_x1 <= line2_x2) && (line1_x2 >= line2_x1 && line1_x2 <= line2_x2);
    int ret3 = (line1_x1 >= line2_x1 && line1_x1 <= line2_x2) && (line1_x2 > line2_x2);
    int ret4 = (line1_x1 < line2_x1) && (line1_x1 > line2_x2);
    return ret1 || ret2 || ret3 || ret4;
}

int is_rect_overlap(rect_t rect1, rect_t rect2)
{
    return (rect1.x < rect2.x + rect2.width &&
            rect1.x + rect1.width > rect2.x &&
            rect1.y < rect2.y + rect2.height &&
            rect1.y + rect1.height > rect2.y);
}

rect_t find_rect_overlap(rect_t rect1, rect_t rect2)
{
    rect_t ret;
    if(rect1.x + rect1.width > rect2.x + rect2.width)
    {
        ret.width = rect2.x + rect2.width - max(rect1.x, rect2.x);
    }
    else
    {
        ret.width = rect1.x + rect1.width - max(rect1.x, rect2.x);
    }
    ret.x = max(rect1.x, rect2.x);
    if(rect1.y + rect1.height > rect2.y + rect2.height)
    {
        ret.height = rect2.y + rect2.height - max(rect1.y, rect2.y);
    }
    else
    {
        ret.height = rect1.y + rect1.height - max(rect1.y, rect2.y);
    }
    ret.y = max(rect1.y, rect2.y);
    return ret;
}


int is_point_in_rect(int point_x, int point_y, rect_t * r)
{
    return (point_x >= r->x && point_x < r->x + r->width) && (point_y >= r->y && point_y < r->y + r->height);
}

rect_t rect_create(int x, int y, int width, int height)
{
    rect_t r;
    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;
    return r;
}

void draw_rect2(canvas_t* c, rect_t* r)
{
    draw_rect(c, r->x, r->y, r->width, r->height);
}

canvas_t canvas_create(int width, int height, uint32_t * framebuffer)
{
    canvas_t canvas;
    canvas.width = width;
    canvas.height = height;
    canvas.framebuffer = framebuffer;
    return canvas;
}