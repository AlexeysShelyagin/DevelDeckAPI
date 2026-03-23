#ifndef GAMEPAD_IMAGE_H
#define GAMEPAD_IMAGE_H

#include <Arduino.h>
#include <PNGdec.h>

#include "config.h"

#ifdef GLOBAL_PNG_DECODER
extern PNG *png_decoder;
#endif

class Image_raw16_t{
protected:
    bool create_(uint16_t w_, uint16_t h_, bool alpha_, int buff_size);

public:
    uint16_t w, h;
    bool alpha;
    int alpha_buff_size;
    uint16_t *img_buffer = nullptr;
    uint8_t *alpha_buffer = nullptr;

    Image_raw16_t() = default;
    ~Image_raw16_t();

    Image_raw16_t(const Image_raw16_t&) = delete;
    Image_raw16_t(Image_raw16_t&& other) noexcept;

    Image_raw16_t& operator=(const Image_raw16_t&) = delete;
    Image_raw16_t& operator=(Image_raw16_t&& other) noexcept;



    /**
     * @brief Reserve memory for the image
     * 
     * @param w_ 
     * @param h_
     * @param alpha_ set true to store alpha layer
     * @return true: created successfully
     * @return false: unable to create, not enough space in heap
     */
    bool create(uint16_t w_, uint16_t h_, bool alpha_ = false);

    /**
     * @brief Delete all image data and free the memory
     * 
     */
    void clear();
};

class Image_raw8_t : public Image_raw16_t{
    using Image_raw16_t::Image_raw16_t;
public:
    Image_raw8_t(Image_raw16_t img);

    /**
     * @brief Reserve memory for the image
     * 
     * @param w_ 
     * @param h_
     * @param alpha_ set true to store alpha layer
     * @return true: created successfully
     * @return false: unable to create, not enough space in heap
     */
    bool create(uint16_t w_, uint16_t h_, bool alpha_ = false);
};

#endif
