//
// Created by fazhehy on 2023/8/25.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "oled.h"

//TODO:利用断言限制xy范围

static void oled_iic_start()
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

static void oled_iic_stop()
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

static void oled_send_byte(uint8_t byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA(byte & (0x80 >> i));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
    OLED_W_SCL(0);
}

static void oled_write_command(uint8_t command)
{
    oled_iic_start();
    oled_send_byte(0x78);
    oled_send_byte(0x00);
    oled_send_byte(command);
    oled_iic_stop();
}

static void oled_write_data(uint8_t data)
{
    oled_iic_start();
    oled_send_byte(0x78);
    oled_send_byte(0x40);
    oled_send_byte(data);
    oled_iic_stop();
}

/*---------------------------------------------------------------------------------*/
static void oled_fill(uint8_t *buffer)
{
    uint8_t * p = buffer;

//    oled_write_command(0xb0);
//    oled_write_command(0x00);
//    oled_write_command(0x10);

    oled_write_command(0x20);
    oled_write_command(0x00);
    oled_write_command(0x21);
    oled_write_command(0x00);
    oled_write_command(0x7F);
    oled_write_command(0x22);
    oled_write_command(0x00);
    oled_write_command(0x07);

    oled_iic_start();
    oled_send_byte(0x78);
    oled_send_byte(0x40);
    for (uint16_t i = 0; i < 128*8; ++i)
        oled_send_byte(*p++);
    oled_iic_stop();
}

void oled_clear()
{
    oled_write_command(0x20);
    oled_write_command(0x00);
    oled_write_command(0x21);
    oled_write_command(0x00);
    oled_write_command(0x7F);
    oled_write_command(0x22);
    oled_write_command(0x00);
    oled_write_command(0x07);

    oled_iic_start();
    oled_send_byte(0x78);
    oled_send_byte(0x40);
    for (uint16_t i = 0; i < 128*8; ++i)
        oled_send_byte(0x00);
    oled_iic_stop();
}

void oled_init()
{
    uint32_t i, j;

    for (i = 0; i < 1000; i++)			//上电延时
    {
        for (j = 0; j < 1000; j++);
    }

//    OLED_I2C_Init();			//端口初始化

    oled_write_command(0xAE);	//关闭显示

    oled_write_command(0xD5);	//设置显示时钟分频比/振荡器频率
    oled_write_command(0x80);

    oled_write_command(0xA8);	//设置多路复用率
    oled_write_command(0x3F);

    oled_write_command(0xD3);	//设置显示偏移
    oled_write_command(0x00);

    oled_write_command(0x40);	//设置显示开始行

    oled_write_command(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置

    oled_write_command(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

    oled_write_command(0xDA);	//设置COM引脚硬件配置
    oled_write_command(0x12);

    oled_write_command(0x81);	//设置对比度控制
    oled_write_command(0xCF);

    oled_write_command(0xD9);	//设置预充电周期
    oled_write_command(0xF1);

    oled_write_command(0xDB);	//设置VCOMH取消选择级别
    oled_write_command(0x30);

    oled_write_command(0xA4);	//设置整个显示打开/关闭

    oled_write_command(0xA6);	//设置正常/倒转显示

    oled_write_command(0x8D);	//设置充电泵
    oled_write_command(0x14);

    oled_write_command(0xAF);	//开启显示

    oled_clear();				//OLED清屏
}
/*--------------------------------------------------------------------------*/
uint8_t SCREEN_BUFFER[OLED_HEIGHT / 8][OLED_WIDTH] = {0};
void oled_update_screen()
{
    oled_fill(SCREEN_BUFFER[0]);
}

void oled_clear_buffer()
{
    memset(SCREEN_BUFFER, 0, OLED_WIDTH*OLED_HEIGHT/8);
}
/*--------------------------------------------------------------------------*/

void oled_show_bmp(uint8_t x, uint8_t y, const uint8_t *bmp, uint8_t w, uint8_t h)
{
    for (uint8_t y_ = y; y_ < y+h; y_+=8){
        for (uint8_t x_ = x; x_ < x+w; ++x_){
            SCREEN_BUFFER[y_/8][x_] |= bmp[(y_-y)/8*w+(x_-x)] << (y_%8);
            SCREEN_BUFFER[y_/8+1][x_] |= (bmp[(y_-y)/8*w+(x_-x)] << (y_%8))>>8;
        }
    }
}

void oled_show_char(uint8_t x, uint8_t y, char ch)
{
    for (uint8_t i = 0 ; i < 6; i++){
        SCREEN_BUFFER[y/8][x+i] = oled_font_6x8[ch-' '][i] << (y%8);
        SCREEN_BUFFER[y/8+1][x+i] = (oled_font_6x8[ch-' '][i] << (y%8))>>8;
    }
}

void oled_show_string(uint8_t x, uint8_t y, const char * str)
{
    while (*str){
        oled_show_char(x, y, *str);
        x += 6;
        str ++;
    }
}

uint32_t oled_pow(uint32_t x, uint32_t y)
{
    uint32_t result = 1;
    while (y--)
        result *= x;

    return result;
}

void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
    for (uint8_t i = 0; i < len; ++i) {
        oled_show_char(x+i*6, y, '0'+num/oled_pow(10, len-i-1)%10);
    }
}

void oled_draw_point(uint8_t x, uint8_t y)
{
//    if (x >= OLED_WIDTH)
//        x = OLED_WIDTH-1;
//    if (y >= OLED_HEIGHT)
//        y = OLED_HEIGHT-1;

    SCREEN_BUFFER[y/8][x] |= 1 << (y%8);
}

void oled_draw_point_xor(uint8_t x, uint8_t y)
{
//    if (x >= OLED_WIDTH)
//        x = OLED_WIDTH-1;
//    if (y >= OLED_HEIGHT)
//        y = OLED_HEIGHT-1;

    SCREEN_BUFFER[y/8][x] ^= 1 << (y%8);
}

void oled_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for (uint8_t i = x; i < x + w; ++i) {
        SCREEN_BUFFER[y/8][i] |= 0xff<<(y%8);
        for (uint8_t j = y/8+1; j < (y+h-1)/8; ++j) {
            SCREEN_BUFFER[j][i] = 0xff;
        }
        SCREEN_BUFFER[(y+h-1)/8][i] |= 0xff>>(8-(y+h-1)%8);
    }
}

void oled_fill_rect_xor(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for (uint8_t i = x; i < x + w; ++i) {
        SCREEN_BUFFER[y/8][i] ^= 0xff<<(y%8);
        for (uint8_t j = y/8+1; j < (y+h-1)/8; ++j) {
            SCREEN_BUFFER[j][i] ^= 0xff;
        }
        SCREEN_BUFFER[(y+h-1)/8][i] ^= 0xff>>(8-(y+h-1)%8);
    }
}

void oled_draw_vLine(uint8_t x, uint8_t y, uint8_t len)
{
    if (len > OLED_HEIGHT)
        len = OLED_HEIGHT;

    for (uint8_t i = 0; i < len; ++i) {
        oled_draw_point(x, y+i);
    }
}

void oled_draw_hLine(uint8_t x, uint8_t y, uint8_t len)
{
    if (len > OLED_WIDTH)
        len = OLED_WIDTH;

    for (uint8_t i = 0; i < len; ++i) {
        oled_draw_point(x+i, y);
    }
}

void oled_draw_vLine_xor(uint8_t x, uint8_t y, uint8_t len)
{
    if (len > OLED_HEIGHT)
        len = OLED_HEIGHT;

    for (uint8_t i = 0; i < len; ++i) {
        oled_draw_point_xor(x, y+i);
    }
}

void oled_draw_hLine_xor(uint8_t x, uint8_t y, uint8_t len)
{
    if (len > OLED_WIDTH)
        len = OLED_WIDTH;

    for (uint8_t i = 0; i < len; ++i) {
        oled_draw_point_xor(x+i, y);
    }
}

void oled_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int16_t dx = (int16_t)x2-x1;
    int16_t dy = (int16_t)y2-y1;

    uint8_t x = x1, y = y1;
    int16_t inc_x, inc_y;
    int16_t error_x = 0, error_y = 0;
    int16_t distance;

    if (dx > 0)
        inc_x = 1;
    else if (dx == 0)
        inc_x = 0;
    else{
        inc_x = -1;
        dx = -dx;
    }

    if (dy > 0)
        inc_y = 1;
    else if (dy == 0)
        inc_y = 0;
    else{
        inc_y = -1;
        dy = -dy;
    }

    distance = dx;
    if (dx < dy)
        distance = dy;
    
    for (uint8_t i = 0; i < distance; ++i) {
        oled_draw_point(x, y);
        error_x += dx;
        error_y += dy;

        if (error_x > distance){
            error_x -= distance;
            x += inc_x;
        }

        if (error_y > distance){
            error_y -= distance;
            y += inc_y;
        }
    }
}

void oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    oled_draw_hLine(x, y, w);
    oled_draw_hLine(x, y+h-1, w);
    oled_draw_vLine(x, y, h);
    oled_draw_vLine(x+w-1, y, h);
}

void oled_draw_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    oled_fill_rect(x, y, w, h);
}

void oled_draw_circle(uint8_t x, uint8_t y, uint8_t r)
{
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x_ = 0;
    int y_ = r;
    while (x_<y_)
    {
        if (f >= 0)
        {
            y_--;
            ddF_y += 2;
            f += ddF_y;
        }
        x_++;
        ddF_x += 2;
        f += ddF_x;
        oled_draw_point(x + x_, y + y_);
        oled_draw_point(x + y_, y + x_);
        oled_draw_point(x + x_, y - y_);
        oled_draw_point(x + y_, y - x_);
        oled_draw_point(x - y_, y + x_);
        oled_draw_point(x - x_, y + y_);
        oled_draw_point(x - y_, y - x_);
        oled_draw_point(x - x_, y - y_);
    }
    oled_draw_point(x, y + r);
    oled_draw_point(x, y - r);
    oled_draw_point(x + r, y);
    oled_draw_point(x - r, y);
}

void oled_draw_circle_helper(uint8_t x, uint8_t y, uint8_t r, uint8_t cornername)
{
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x_ = 0;
    int y_ = r;
    while (x_<y_)
    {
        if (f >= 0)
        {
            y_--;
            ddF_y += 2;
            f += ddF_y;
        }
        x_++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x04)
        {
            oled_draw_point(x + x_, y + y_);
            oled_draw_point(x + y_, y + x_);
        }
        if (cornername & 0x02)
        {
            oled_draw_point(x + x_, y - y_);
            oled_draw_point(x + y_, y - x_);
        }
        if (cornername & 0x08)
        {
            oled_draw_point(x - y_, y + x_);
            oled_draw_point(x - x_, y + y_);
        }
        if (cornername & 0x01)
        {
            oled_draw_point(x - y_, y - x_);
            oled_draw_point(x - x_, y - y_);
        }
    }
}

void oled_draw_fill_circle(uint8_t x, uint8_t y, uint8_t r)
{
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x_ = 0;
    int y_ = r;
    while (x_ < y_) {
        if (f >= 0) {
            y_--;
            ddF_y += 2;
            f += ddF_y;
        }

        x_++;
        ddF_x += 2;
        f += ddF_x;

        oled_draw_vLine(x + x_, y - y_, 2 * y_ + 1);
        oled_draw_vLine(x + y_, y - x_, 2 * x_ + 1);
        oled_draw_vLine(x - x_, y - y_, 2 * y_ + 1);
        oled_draw_vLine(x - y_, y - x_, 2 * x_ + 1);
    }
    oled_draw_vLine(x, y-r, 2*r+1);
}

void oled_draw_fill_circle_helper(uint8_t x, uint8_t y, uint8_t r, uint8_t cornername, uint8_t delta)
{
    // used to do circles and roundrects!
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x_ = 0;
    int y_ = r;
    while (x_ < y_)
    {
        if (f >= 0)
        {
            y_--;
            ddF_y += 2;
            f += ddF_y;
        }

        x_++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1)
        {
            oled_draw_vLine(x+x_, y-y_, 2*y_+1+delta);
             oled_draw_vLine(x+y_, y-x_, 2*x_+1+delta);
        }

        if (cornername & 0x2)
        {
            oled_draw_vLine(x-x_, y-y_, 2*y_+1+delta);
            oled_draw_vLine(x-y_, y-x_, 2*x_+1+delta);
        }
    }
}

void oled_draw_fill_circle_helper_xor(uint8_t x, uint8_t y, uint8_t r, uint8_t cornername, uint8_t delta)
{
    // used to do circles and roundrects!
    int f = 1 - r;
    int ddF_x = 1;
    int ddF_y = -2 * r;
    int x_ = 0;
    int y_ = r;
    while (x_ < y_)
    {
        if (f >= 0)
        {
            y_--;
            ddF_y += 2;
            f += ddF_y;
        }

        x_++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1)
        {
            oled_draw_vLine_xor(x+x_, y-y_, 2*y_+1+delta);
            oled_draw_vLine_xor(x+y_, y-x_, 2*x_+1+delta);
        }

        if (cornername & 0x2)
        {
            oled_draw_vLine_xor(x-x_, y-y_, 2*y_+1+delta);
            oled_draw_vLine_xor(x-y_, y-x_, 2*x_+1+delta);
        }
    }
}

void oled_draw_round_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r)
{
    // smarter version
    oled_draw_hLine(x+r, y, w-2*r); // Top
    oled_draw_hLine(x+r, y+h-1, w-2*r); // Bottom
    oled_draw_vLine(x, y+r, h-2*r); // Left
    oled_draw_vLine(x+w-1, y+r, h-2*r); // Right
    // draw four corners
    oled_draw_circle_helper(x+r, y+r, r, 1);
    oled_draw_circle_helper(x+w-r-1, y+r, r, 2);
    oled_draw_circle_helper(x+w-r-1, y+h-r-1, r, 4);
    oled_draw_circle_helper(x+r, y+h-r-1, r, 8);
}

void oled_draw_fill_round_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r)
{
    // smarter version
    oled_fill_rect(x+r, y, w-2*r, h+1);
    oled_update_screen();

    // draw four corners
    oled_draw_fill_circle_helper(x+w-r-1, y+r, r, 1, h-2*r-1);
    oled_draw_fill_circle_helper(x+r, y+r, r, 2, h-2*r-1);
}

void oled_draw_fill_round_rect_xor(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r)
{
    oled_fill_rect_xor(x+r, y, w-2*r, h+1);

    oled_draw_fill_circle_helper(x+w-r-1, y+r, r, 1, h-2*r-1);
    oled_draw_fill_circle_helper(x+r, y+r, r, 2, h-2*r-1);
}

uint8_t oled_printf(uint8_t x, uint8_t y, const char *format, ...)
{
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if(len < 0) {
        va_end(arg);
        return 0;
    }
    va_end(arg);
    oled_show_string(x, y, temp);
    return len;
}
