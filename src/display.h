#ifndef DD_DISPLAY_H
#define DD_DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include <PNGdec.h>

#include "config.h"
#include "image.h"


// set the type of canvas for external use
#define DD_CANVAS_T_DEFINED


// Make possible user color depth override
extern uint8_t CANVAS_COLOR_DEPTH __attribute__((weak));
extern bool ALLOW_DMA __attribute__((weak));



struct Graphics_params_t{
    uint8_t font_id = 0;
    uint8_t text_size = 1;
    uint32_t text_color = TFT_WHITE;
    bool wrap_x = true, wrap_y = false;
    int16_t cur_x = 0, cur_y = 0;
    int16_t orig_x = 0, orig_y = 0;
};



class DD_canvas_t : public TFT_eSprite{
    using TFT_eSprite::TFT_eSprite;

    uint8_t* fonts[FONTS_MAX_N] = {nullptr};
    uint16_t font_h;
    uint8_t dynamic_mem_font = 0;
    uint8_t font_id = 0;
public:
    explicit DD_canvas_t(TFT_eSPI *tft) : TFT_eSprite(tft){}
    ~DD_canvas_t();


    void pushMaskedImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask, uint8_t sbpp = 16);

    using TFT_eSprite::pushImage;
    void pushImage(int32_t x, int32_t y, Image_raw16_t *image);
    void pushImage(int32_t x, int32_t y, Image_raw16_t &image);
    void pushImage(int32_t x, int32_t y, Image_raw8_t *image);
    void pushImage(int32_t x, int32_t y, Image_raw8_t &image);

    void drawPNGFromFile(File *file, int32_t x, int32_t y, bool alpha_channel = false);

    void loadFont(File *file, uint8_t id = 1);
    void loadFont(const uint8_t array[], uint8_t id = 1);
    void unloadFont(uint8_t id = 1);
    void setFont(uint8_t id = 0);
    uint8_t getFontID();

    void setLineSpacing(float multiplier);

    void setDefaultGraphicsParams();
    void setGraphicsParams(Graphics_params_t params);
    Graphics_params_t graphicsParams();
};


class DD_display{
    int16_t w, h;
    uint8_t ledc_ch;
    uint8_t brightness = 255;

    TFT_eSPI disp = TFT_eSPI();
    DD_canvas_t canvas = DD_canvas_t(&disp);

    bool initialized = false;
    uint8_t *canvas_buffer_ptr = nullptr;
public:
    DD_display() = default;

    bool init(uint16_t width = DISP_WIDTH, uint16_t height = DISP_HEIGHT, uint8_t backlight_channel = DISP_BACKLIGHT_LEDC_CHANNEL);

    TFT_eSPI* get_display_reference();
    DD_canvas_t* get_canvas_reference();

    void set_brightness(uint8_t brightness_);
    uint8_t get_brightness();


    // Base canvas section
    void display_canvas();
    void display_canvas(int16_t x0, int16_t y0, uint16_t window_w, uint16_t window_h);
    void clear_canvas();
    
    // Sprites section
    DD_canvas_t* create_sprite(uint16_t width, uint16_t height, uint8_t color_depth = 8);
    void display_sprite(DD_canvas_t *sprite, int16_t disp_x = 0, int16_t disp_y = 0);
    void display_sprite(
        DD_canvas_t *sprite, 
        int16_t disp_x, int16_t disp_y, 
        int16_t sprite_x0, int16_t sprite_y0, 
        uint16_t window_w, uint16_t window_h
    );
    void clear_sprite(DD_canvas_t *sprite);
    void delete_sprite(DD_canvas_t* sprite);
};

#endif