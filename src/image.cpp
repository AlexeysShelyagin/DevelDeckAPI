#include "image.h"

Image_raw16_t::~Image_raw16_t(){
    clear();
}

void Image_raw16_t::clear(){
    delete [] img_buffer;
    delete [] alpha_buffer;
    img_buffer = nullptr;
    alpha_buffer = nullptr;
}

Image_raw16_t::Image_raw16_t(Image_raw16_t&& other) noexcept{
    img_buffer = other.img_buffer;
    alpha_buffer = other.alpha_buffer;
    other.img_buffer = nullptr;
    other.alpha_buffer = nullptr;
}

Image_raw16_t& Image_raw16_t::operator=(Image_raw16_t&& other) noexcept{
    if (this != &other) {
        w = other.w;
        h = other.h;
        alpha = other.alpha;
        delete [] img_buffer;
        delete [] alpha_buffer;
        img_buffer = other.img_buffer;
        alpha_buffer = other.alpha_buffer;
        other.img_buffer = nullptr;
        other.alpha_buffer = nullptr;
    }
    return *this;
}

bool Image_raw16_t::create_(uint16_t w_, uint16_t h_, bool alpha_, int buff_size){
    alpha_buff_size = ((w_ + 7) >> 3) * h_;

    if(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < w_ * h_ * sizeof(uint16_t))
        return 0;
    if (alpha_ && heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < alpha_buff_size)
        return 0;
    
    clear();

    w = w_;
    h = h_;
    alpha = alpha_;

    img_buffer = new uint16_t[buff_size];
    if(alpha)
        alpha_buffer = new uint8_t[alpha_buff_size];

    return 1;
}

bool Image_raw16_t::create(uint16_t w_, uint16_t h_, bool alpha_){
    return create_(w_, h_, alpha_, w_ * h_);
}

bool Image_raw8_t::create(uint16_t w_, uint16_t h_, bool alpha_){
    return create_(w_, h_, alpha_, w_ * h_ / 2);
}

Image_raw8_t::Image_raw8_t(Image_raw16_t img){
    if(!create(img.w, img.h, img.alpha))
        return;

    int size = w * h / 2;
    int i = 0;
    while(i < size){
        uint16_t c1 = img.img_buffer[i * 2] << 8 | img.img_buffer[i * 2] >> 8;
        uint16_t c2 = img.img_buffer[i * 2 + 1] << 8 | img.img_buffer[i * 2 + 1] >> 8;

        img_buffer[i++] = (c1 & 0xE000) >> 8 | (c1 & 0x0700) >> 6 | (c1 & 0x0018) >> 3 |
                          (c2 & 0xE000)      | (c2 & 0x0700) << 2 | (c2 & 0x0018) << 5;
    }

    if(alpha)
        memcpy(alpha_buffer, img.alpha_buffer, alpha_buff_size);
}