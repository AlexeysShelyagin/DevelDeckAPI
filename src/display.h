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


// Make possible user color depth override
extern uint8_t CANVAS_COLOR_DEPTH __attribute__((weak));
extern bool ALLOW_DMA __attribute__((weak));


/**
 * @brief Graphics styles and parameters container
 * 
 */
struct Graphics_params_t{
        uint8_t font_id;
        uint8_t text_size;
        uint32_t text_color;
        bool wrap_x, wrap_y;
        int16_t cur_x, cur_y;
        int16_t orig_x, orig_y;
    };

class Gamepad_canvas_t : public TFT_eSprite{
    using TFT_eSprite::TFT_eSprite;

    uint8_t *fonts[FONTS_MAX_N];
    uint16_t font_h;
    uint8_t dynamic_mem_font = 0;
    uint8_t font_id = 0;
public:
    /**
     * @brief Wrapper around `TFT_eSPI::pushMaskedImage()` for writing image to canvas instead display
     * 
     * @param x 
     * @param y 
     * @param w 
     * @param h 
     * @param img 
     * @param mask 
     * @param sbpp 
     */
    void pushMaskedImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask, uint8_t sbpp = 16);


    using TFT_eSprite::pushImage;

    /**
     * @brief Draw image to canvas from `Image_raw16_t` variable.
     * 
     * @param x 
     * @param y 
     * @param image 
     */
    void pushImage(int32_t x, int32_t y, Image_raw16_t &image);

    /**
     * @brief Draw image to canvas from `Image_raw8_t` variable.
     * 
     * @param x 
     * @param y 
     * @param image 
     */
    void pushImage(int32_t x, int32_t y, Image_raw8_t &image);


    /**
     * @brief Draw contentes of PNG file directly to display
     * 
     * @note Usage of `Gamepad::game_files.get_file_reference()` is recommended for `*file` param
     * 
     * @param file pointer to PNG file
     * @param x 
     * @param y 
     * @param alpha_channel PNG alpha would be ingnored on false
     */
    void drawPNGFromFile(File *file, int32_t x, int32_t y, bool alpha_channel = false);



    /**
     * @brief Load font from the `.vlw` file and assign `id` to it.
     * 
     * @note Usage of `Gamepad::game_files.get_file_reference()` is recommended for `*file` param
     * 
     * @note id = 0 is default system font. It is impossible to overwrite it
     * 
     * @param file pointer to VLW file
     * @param id unique id of the font
     * 
     * @note If specific id was already used, font with this id would be overwritten 
     */
    void loadFont(File *file, uint8_t id = 1);

    /**
     * @brief Load font from the `.vlw` data array and assign `id` to it.
     * 
     * @note id = 0 is default system font. It is impossible to overwrite it
     * 
     * @param file pointer to VLW file
     * @param id unique id of the font
     * 
     * @note If specific id was already used, font with this id would be overwritten 
     */
    void loadFont(const uint8_t array[], uint8_t id = 1);

    /**
     * @brief Remove all data of the font by its id. Its id now can be used for loading another font
     * 
     * @param id font id
     */
    void unloadFont(uint8_t id = 1);

    /**
     * @brief Select current canvas font by id
     * 
     * @param id font id
     */
    void setFont(uint8_t id = 0);

    /**
     * @brief Get the id of currently used font
     * 
     * @return uint8_t: id
     */
    uint8_t getFontID();


    /**
     * @brief Set spacing between multiline text lines
     * 
     * @param multiplier line spacing in respect to font height
     */
    void setLineSpacing(float multiplier);



    /**
     * @brief Set canvas formating to defaults
     * 
     * @note This function affects:
     *  - set default font (id = 0)
     *  - text size to 1
     *  - text color to White
     *  - default text wrap mode
     *  - canvas origin to (0, 0)
     */
    void setDefaultGraphicsParams();

    /**
     * @brief Set graphics parameters from variable
     * 
     * @param params `Gamepad_canvas_t::graphics_params_t` variable with saved parameters
     */
    void setGraphicsParams(Graphics_params_t params);

    /**
     * @brief Get current graphics parameters
     * 
     * @return Gamepad_canvas_t::graphics_params_t parameters container variable
     */
    Graphics_params_t graphicsParams();
};


class Gamepad_display{
    int16_t w, h;
    uint8_t channel;
    uint8_t brightness = 255;

    TFT_eSPI disp = TFT_eSPI();
    Gamepad_canvas_t canvas = Gamepad_canvas_t(&disp);

    bool initialized = false;
    uint8_t *canvas_buffer_ptr = nullptr;
public:
    Gamepad_display() = default;

    bool init(uint16_t width = DISP_WIDTH, uint16_t height = DISP_HEIGHT, uint8_t backlight_channel = DISP_BACKLIGHT_LEDC_CHANNEL);

    TFT_eSPI* get_display_reference();
    Gamepad_canvas_t* get_canvas_reference();

    void set_brightness(uint8_t brightness_);
    uint8_t get_brightness();


    // Base canvas section
    void display_canvas();
    void display_canvas(int16_t x0, int16_t y0, uint16_t window_w, uint16_t window_h);
    void clear_canvas();
    
    // Sprites section
    Gamepad_canvas_t* create_sprite(uint16_t width, uint16_t height, uint8_t color_depth = 8);
    void display_sprite(Gamepad_canvas_t *sprite, int16_t disp_x = 0, int16_t disp_y = 0);
    void display_sprite(
        Gamepad_canvas_t *sprite, 
        int16_t disp_x, int16_t disp_y, 
        int16_t sprite_x0, int16_t sprite_y0, 
        uint16_t window_w, uint16_t window_h
    );
    void clear_sprite(Gamepad_canvas_t *sprite);
    void delete_sprite(Gamepad_canvas_t* sprite);
};

#endif