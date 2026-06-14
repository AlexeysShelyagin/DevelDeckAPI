#include "image.h"

#ifdef GLOBAL_PNG_DECODER
static PNG png_dec;
PNG *png_decoder = &png_dec;
#endif



Image_raw16_t::~Image_raw16_t(){
    clear();
}

void Image_raw16_t::clear(){
    if(!esp_ptr_in_drom(img_buff))
        delete [] img_buff;
    if(!esp_ptr_in_drom(alpha_buff))
        delete [] alpha_buff;
    img_buff = nullptr;
    alpha_buff = nullptr;
    w = h = 0;
    alpha = false;
    alpha_buff_size = 0;
}

Image_raw16_t::Image_raw16_t(Image_raw16_t&& other) noexcept{
    img_buff = other.img_buff;
    alpha_buff = other.alpha_buff;
    other.img_buff = nullptr;
    other.alpha_buff = nullptr;
}

Image_raw16_t& Image_raw16_t::operator=(Image_raw16_t&& other) noexcept{
    if (this != &other) {
        clear();

        w = other.w;
        h = other.h;
        alpha = other.alpha;
        alpha_buff_size = other.alpha_buff_size;
        img_buff = other.img_buff;
        alpha_buff = other.alpha_buff;

        other.w = other.h = 0;
        other.alpha = false;
        alpha_buff_size = 0;
        other.img_buff = nullptr;
        other.alpha_buff = nullptr;
    }
    return *this;
}

bool Image_raw16_t::create_(void *ibuff, void *abuff, uint16_t w_, uint16_t h_, bool alpha_, int buff_size){
    alpha_buff_size = ((w_ + 7) >> 3) * h_;

    clear();
    if(!esp_ptr_in_drom(ibuff)){
        if(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < w_ * h_ * sizeof(uint16_t))
            return 0;
    }
    if(!esp_ptr_in_drom(abuff)){
        if (alpha_ && heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < alpha_buff_size)
            return 0;
    }

    w = w_;
    h = h_;
    alpha = alpha_;

    img_buff = (ibuff == nullptr) ? new uint16_t[buff_size] : (uint16_t*) ibuff;
    if(alpha)
        alpha_buff = (abuff == nullptr) ? new uint8_t[alpha_buff_size] : (uint8_t*) abuff;

    return 1;
}

bool Image_raw16_t::create(uint16_t w_, uint16_t h_, bool alpha_){
    return create_(nullptr, nullptr, w_, h_, alpha_, w_ * h_);
}

bool Image_raw16_t::create(const void *image_data, const void *alpha_data, uint16_t w_, uint16_t h_){
    return create_((uint16_t*) image_data, (uint8_t*) alpha_data, w_, h_, true, w_ * h_);
}

bool Image_raw16_t::create(const void *image_data, uint16_t w_, uint16_t h_){
    return create_((uint16_t*) image_data, nullptr, w_, h_, false, w_ * h_);
}



bool Image_raw8_t::create(uint16_t w_, uint16_t h_, bool alpha_){
    return create_(nullptr, nullptr, w_, h_, alpha_, w_ * h_ / 2);
}

bool Image_raw8_t::create(const void *image_data, const void *alpha_data, uint16_t w_, uint16_t h_){
    return create_((uint16_t*) image_data, (uint8_t*) alpha_data, w_, h_, true, w_ * h_ / 2);
}

bool Image_raw8_t::create(const void *image_data, uint16_t w_, uint16_t h_){
    return create_((uint16_t*) image_data, nullptr, w_, h_, false, w_ * h_ / 2);
}

Image_raw8_t::Image_raw8_t(Image_raw16_t &img){
    if(!create(img.w, img.h, img.alpha))
        return;

    int size = w * h / 2;
    int i = 0;
    while(i < size){
        uint16_t c1 = img.img_buff[i * 2] << 8 | img.img_buff[i * 2] >> 8;
        uint16_t c2 = img.img_buff[i * 2 + 1] << 8 | img.img_buff[i * 2 + 1] >> 8;

        img_buff[i++] = (c1 & 0xE000) >> 8 | (c1 & 0x0700) >> 6 | (c1 & 0x0018) >> 3 |
                          (c2 & 0xE000)      | (c2 & 0x0700) << 2 | (c2 & 0x0018) << 5;
    }

    if(alpha)
        memcpy(alpha_buff, img.alpha_buff, alpha_buff_size);
}