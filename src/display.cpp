#include "display.h"


/***************************************************************************************
** ---------------Modified function from TFT_eSPI------------------
**					- avoid display update
**					- image bit depth
** Function name:           pushMaskedImage
** Description:             Render a 16-bit colour image to TFT with a 1bpp mask
***************************************************************************************/
// Can be used with a 16bpp sprite and a 1bpp sprite for the mask
void Gamepad_canvas_t::pushMaskedImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask, uint8_t sbpp)
{
  if (_vpOoB || w < 1 || h < 1) return;

  // To simplify mask handling the window clipping is done by the pushImage function
  // Each mask image line assumed to be padded to an integer number of bytes & padding bits are 0

  uint8_t  *mptr = mask;
  uint8_t  *eptr = mask + ((w + 7) >> 3);
  uint16_t *iptr = img;
  uint32_t setCount = 0;

  int32_t ptr_offset = w;
  if(sbpp == 8)
	ptr_offset = w / 2;

  // For each line in the image
  while (h--) {
    uint32_t xp = 0;
    uint32_t clearCount = 0;
    uint8_t  mbyte= *mptr++;
    uint32_t bits  = 8;
    // Scan through each byte of the bitmap and determine run lengths
    do {
      setCount = 0;

      //Get run length for clear bits to determine x offset
      while ((mbyte & 0x80) == 0x00) {
        // Check if remaining bits in byte are clear (reduce shifts)
        if (mbyte == 0) {
          clearCount += bits;      // bits not always 8 here
          if (mptr >= eptr) break; // end of line
          mbyte = *mptr++;
          bits  = 8;
          continue;
        }
        mbyte = mbyte << 1; // 0's shifted in
        clearCount ++;
        if (--bits) continue;;
        if (mptr >= eptr) break;
        mbyte = *mptr++;
        bits  = 8;
      }

      //Get run length for set bits to determine render width
      while ((mbyte & 0x80) == 0x80) {
        // Check if all bits are set (reduces shifts)
        if (mbyte == 0xFF) {
          setCount += bits;
          if (mptr >= eptr) break;
          mbyte = *mptr++;
          //bits  = 8; // NR, bits always 8 here unless 1's shifted in
          continue;
        }
        mbyte = mbyte << 1; //or mbyte += mbyte + 1 to shift in 1's
        setCount ++;
        if (--bits) continue;
        if (mptr >= eptr) break;
        mbyte = *mptr++;
        bits  = 8;
      }

      // A mask boundary or mask end has been found, so render the pixel line
      if (setCount) {
        xp += clearCount;
        clearCount = 0;
		if(sbpp == 16)
        	pushImage(x + xp, y, setCount, 1, iptr + xp);      // pushImage handles clipping
		if(sbpp == 8){
			uint16_t* ptr_8bit = iptr;
			if(xp % 2 != 0)
				ptr_8bit = (uint16_t *) ((uint8_t *) iptr + 1);				// offset the pointer by 1 byte, to get proper coordinate
			pushImage(x + xp, y, setCount, 1, ptr_8bit + xp / 2, 8);
		}
        if (mptr >= eptr) break;
        xp += setCount;
      }
    } while (setCount || mptr < eptr);

    y++;
    iptr += ptr_offset;
    eptr += ((w + 7) >> 3);
  }
}

void Gamepad_canvas_t::pushImage(int32_t x, int32_t y, Image_raw16_t &image){
	if(image.alpha)
		pushMaskedImage(x, y, image.w, image.h, image.img_buffer, image.alpha_buffer);
	else
		TFT_eSprite::pushImage(x, y, image.w, image.h, image.img_buffer);
}

void Gamepad_canvas_t::pushImage(int32_t x, int32_t y, Image_raw8_t &image){
	if(image.alpha)
		pushMaskedImage(x, y, image.w, image.h, image.img_buffer, image.alpha_buffer, 8);
	else
		TFT_eSprite::pushImage(x, y, image.w, image.h, image.img_buffer, 8);
}


File *PNG_disp_file_ptr;
PNG *png_disp;

struct PNG_user_param_t{
	Gamepad_canvas_t *canvas;
	int32_t x;
	int32_t y;
	bool alpha;
};

void *PNG_disp_init(const char *filename, int32_t *size){
	*size = PNG_disp_file_ptr -> size();
	return PNG_disp_file_ptr;
}

int32_t PNG_disp_read(PNGFILE *handle, uint8_t *buffer, int32_t length){
	if(!(*PNG_disp_file_ptr))
		return 0;
	return PNG_disp_file_ptr -> read(buffer, length);
}

int32_t PNG_disp_seek(PNGFILE *handle, int32_t position){
	if(!(*PNG_disp_file_ptr))
		return 0;
	return PNG_disp_file_ptr -> seek(position);
}

int PNG_disp_draw(PNGDRAW *pDraw){
	PNG_user_param_t *params = (PNG_user_param_t *) pDraw -> pUser;

	uint16_t line_buffer[pDraw -> iWidth];

	png_disp -> getLineAsRGB565(pDraw, line_buffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

	if(params -> alpha){
		uint8_t mask_buffer[(pDraw -> iWidth + 7) >> 3];
		if(png_disp -> getAlphaMask(pDraw, mask_buffer, 255))
			params -> canvas -> pushMaskedImage(params -> x, params -> y + pDraw -> y, pDraw -> iWidth, 1, line_buffer, mask_buffer);
	}
	else{
		params -> canvas -> pushImage(params -> x, params -> y + pDraw -> y, pDraw -> iWidth, 1, line_buffer);
	}

	return 1;
}

void Gamepad_canvas_t::drawPNGFromFile(File *file, int32_t x, int32_t y, bool alpha_channel){
	if(file == nullptr)
		return;

	if (heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < sizeof(PNG))
		return;
	png_disp = new PNG;
	
	PNG_disp_file_ptr = file;
	String name = PNG_disp_file_ptr -> name();
	String ext = name.substring(name.length() - 4, name.length());
	if(ext != ".png" && ext != ".PNG")
		return;
	
	int status = png_disp -> open(name.c_str(), PNG_disp_init, NULL, PNG_disp_read, PNG_disp_seek, PNG_disp_draw);
	if(status == PNG_SUCCESS){
		PNG_user_param_t params = {this, x, y, alpha_channel};
		status = png_disp -> decode(&params, 0);
	}

	delete png_disp;
}

/***************************************************************************************
** ---------------Modified function from TFT_eSPI------------------
** Function name:           loadFont
** Description:             loads parameters from a font vlw file
***************************************************************************************/
void Gamepad_canvas_t::loadFont(File *file, uint8_t id){
	if(id == 0)
		return;

	String path = file -> path(), ext = path.substring(path.length() - 4, path.length());
	if(ext != ".vlw" && ext != ".VLW")
		return;
	path = path.substring(1, path.length() - 4);

	unloadFont(id);

	fonts[id - 1] = new uint8_t[file -> size()];
	file -> seek(0);
	file -> read(fonts[id - 1], file -> size());

	dynamic_mem_font |= 1 << (id - 1);
}

void Gamepad_canvas_t::loadFont(const uint8_t array[], uint8_t id){
	if(id == 0)
		return;
	
	unloadFont(id);

	fonts[id - 1] = (uint8_t *) array;

	dynamic_mem_font &= ~(1 << (id - 1));
}

void Gamepad_canvas_t::unloadFont(uint8_t id){
	if(id == 0 || id > FONTS_MAX_N)
		return;
	
	if(fonts[id - 1] == nullptr)
		return;

	if(dynamic_mem_font << (id - 1) & 1)
		delete[] fonts[id - 1];
	fonts[id - 1] = nullptr;

	if(font_id == id){
		TFT_eSprite::unloadFont();
		font_id = 0;
	}
}

void Gamepad_canvas_t::setFont(uint8_t id){
	if(id == font_id || id > FONTS_MAX_N || fonts[id - 1] == nullptr)
		return;
	
	if(id == 0)
		TFT_eSprite::unloadFont();
	else
		TFT_eSprite::loadFont(fonts[id - 1]);
	font_id = id;
	
	gFont.yAdvance = gFont.ascent;
	font_h = gFont.yAdvance;
}

uint8_t Gamepad_canvas_t::getFontID(){
	return font_id;
}

void Gamepad_canvas_t::setLineSpacing(float multiplier){
	gFont.yAdvance = multiplier * font_h;
}

void Gamepad_canvas_t::setDefaultGraphicsParams(){
	setFont(0);
    setTextSize(1);
    setTextColor(TFT_WHITE);
	setCursor(0, 0);
	setTextWrap(true, false);
    setOrigin(0, 0);
}

void Gamepad_canvas_t::setGraphicsParams(graphics_params_t params){
	setFont(params.font_id);
    setTextSize(params.text_size);
    setTextColor(params.text_color);
	setCursor(params.cur_x, params.cur_y);
	setTextWrap(params.wrap_x, params.wrap_y);
    setOrigin(params.orig_x, params.orig_y);
}

Gamepad_canvas_t::graphics_params_t Gamepad_canvas_t::graphicsParams(){
	graphics_params_t res;

	res.font_id = font_id;
	res.text_size = textsize;
	res.text_color = textcolor;
	res.cur_x = cursor_x; res.cur_y = cursor_y;
	res.wrap_x = textwrapX; res.wrap_y = textwrapY;
	res.orig_x = _xDatum; res.orig_y = _yDatum;

	return res;
}



bool Gamepad_display::init(uint16_t width, uint16_t height, uint8_t backlight_channel){
	w = width;
	h = height;

    disp.init();
	disp.setRotation(DISP_ROTATION);
	disp.invertDisplay(false);
	disp.fillScreen(TFT_WHITE);

	canvas.setColorDepth(DEFAULT_COLOR_DEPTH);
	if (heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < (float) DEFAULT_COLOR_DEPTH / 8.0 * w * h){
		canvas.createSprite(1, 1);					// make sure canvas is not nullptr, marking initializaion as failed
		return false;
	}
	sprite_buffer_ptr = (uint8_t *) canvas.createSprite(w, h);
	canvas.fillSprite(0);
	canvas.setTextFont(1);

	channel = backlight_channel;
	ledcSetup(channel, 10000, 8);
	ledcAttachPin(DISP_BACKLIGHT_PIN, channel);
	ledcWrite(channel, brightness);

	initialized = true;
	return true;
}

TFT_eSPI* Gamepad_display::get_display_reference(){
	return &disp;
}

Gamepad_canvas_t* Gamepad_display::get_canvas_reference(){
	return &canvas;
}

void Gamepad_display::update(){
	if(!initialized)
		return;
	canvas.pushSprite(0, 0);
}

void Gamepad_display::flip(){
	update();
}

void Gamepad_display::clear(){
	canvas.fillSprite(0);
}

void Gamepad_display::set_brightness(uint8_t brightness_){
	brightness = brightness_;
	ledcWrite(channel, brightness);
}

uint8_t Gamepad_display::get_brightness(){
	return brightness;
}

Gamepad_canvas_t* Gamepad_display::create_sprite(uint16_t width, uint16_t height, uint8_t color_depth){
	Gamepad_canvas_t *sprite = new Gamepad_canvas_t(&disp);

	sprite -> setColorDepth(color_depth);
	if (heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < (float) color_depth / 8.0 * width * height){
		delete sprite;
		return nullptr;
	}
	sprite -> createSprite(width, height);
	sprite -> fillSprite(0);
	sprite -> setTextFont(1);

	return sprite;
}

void Gamepad_display::display_sprite(Gamepad_canvas_t *sprite, uint16_t x, uint16_t y){
	if(!initialized || sprite == nullptr)
		return;
	sprite -> pushSprite(x, y);
}

void Gamepad_display::clear_sprite(Gamepad_canvas_t *sprite){
	sprite -> fillSprite(0);
}

void Gamepad_display::delete_sprite(Gamepad_canvas_t *sprite){
	sprite -> deleteSprite();
	delete sprite;
}