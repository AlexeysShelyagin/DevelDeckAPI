#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <PNGdec.h>

#include "config.h"
#include "image.h"


// set the type of canvas for external use
#define GAMEPAD_CANVAS_T_DEFINED

class Gamepad_canvas_t : public TFT_eSprite{
    using TFT_eSprite::TFT_eSprite;

    uint8_t *fonts[FONTS_MAX_N];
    uint16_t font_h;
    uint8_t dynamic_mem_font = 0;
    uint8_t font_id = 0;
public:
    struct graphics_params_t{
        uint8_t font_id;
        uint8_t text_size;
        uint32_t text_color;
        bool wrap_x, wrap_y;
        int16_t cur_x, cur_y;
        int16_t orig_x, orig_y;
    };
    void pushMaskedImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask, uint8_t sbpp = 16);

    using TFT_eSprite::pushImage;
    void pushImage(int32_t x, int32_t y, Image_raw16_t &image);
    void pushImage(int32_t x, int32_t y, Image_raw8_t &image);

    void drawPNGFromFile(File *file, int32_t x, int32_t y, bool alpha_channel = false);

    void loadFont(File *file, uint8_t id = 1);
    void loadFont(const uint8_t array[], uint8_t id = 1);
    void unloadFont(uint8_t id = 1);
    void setFont(uint8_t id = 0);
    uint8_t getFontID();

    void setLineSpacing(float multiplier);

    void setDefaultGraphicsParams();
    void setGraphicsParams(graphics_params_t params);
    graphics_params_t graphicsParams();
};


class Gamepad_display{
    int16_t w, h;
    uint8_t channel;
    uint8_t brightness = 255;

    TFT_eSPI disp = TFT_eSPI();
    Gamepad_canvas_t canvas = Gamepad_canvas_t(&disp);

    bool initialized = false;
    uint8_t *sprite_buffer_ptr = nullptr;
public:
    Gamepad_display() = default;

    bool init(uint16_t width = DISP_WIDTH, uint16_t height = DISP_HEIGHT, uint8_t backlight_channel = DISP_BACKLIGHT_LEDC_CHANNEL);

    TFT_eSPI* get_display_reference();
    Gamepad_canvas_t* get_canvas_reference();

    void update();
    void flip();
    void clear();

    void set_brightness(uint8_t brightness_);
    uint8_t get_brightness();
    
    Gamepad_canvas_t* create_sprite(uint16_t width, uint16_t height, uint8_t color_depth = 8);
    void display_sprite(Gamepad_canvas_t *sprite, uint16_t x = 0, uint16_t y = 0);
    void clear_sprite(Gamepad_canvas_t *sprite);
    void delete_sprite(Gamepad_canvas_t* sprite);
};

#endif