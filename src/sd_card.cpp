#include <sd_card.h>

Gamepad_SD_card::~Gamepad_SD_card(){
    dir.close();
    file.close();
}

bool Gamepad_SD_card::check_root_level(String path){
    if(path.length() < root.length())
        return 0;
    
    return (path.substring(0, root.length()) == root);
}

bool Gamepad_SD_card::process_path(String &path, bool absolute){
    if(path[0] != '/' && path.length() != 0)
        path = "/" + path;
    
    if(!absolute)
        path = dir.path() + path;

    if(!check_root_level(path))
        return 0;
    
    return 1;
}

uint8_t Gamepad_SD_card::init(String root_limit){
    if(initialized)
        return SD_OK;

    if(!SD.begin(SD_CS_PIN))
        return SD_FAILED;
        
    if(SD.cardType() == CARD_NONE)
        return SD_DISCONNECT;

    initialized = true;
    root = root_limit;

    if(!open_dir(root_limit, 1)){
        initialized = false;
        return SD_FAILED;
    }

    return SD_OK;
}

bool Gamepad_SD_card::open_dir(String path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;
    
    dir = SD.open(path);
    
    if(!dir)
        return 0;
    return dir.isDirectory();
}

bool Gamepad_SD_card::open_root_dir(){
    if(!initialized)
        return 0;
    
    String current_path = dir.path();
    uint16_t i = current_path.length() - 1;

    while(i != -1 && current_path[i] != '/')
        i--;

    if(i < 0)
        return 0;

    String res_path = current_path.substring(0, i);
    if(!check_root_level(res_path))
        return 0;
    
    return open_dir(res_path, 1);
} 

std::vector < file_name_t > Gamepad_SD_card::list_dir(){
    std::vector < file_name_t > list;

    if(!initialized)
        return list;

    File file = dir.openNextFile();
    while(file){
        file_name_t tmp = {file.name(), (file.isDirectory()) ? IS_FOLDER : IS_FILE, file.path()};
        list.push_back(tmp);

        file = dir.openNextFile();
    }

    return list;
}

String Gamepad_SD_card::current_dir(){
    return dir.path();
}

bool Gamepad_SD_card::make_dir(String path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;

    return SD.mkdir(path);
}

bool Gamepad_SD_card::remove_dir(String path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;

    return SD.rmdir(path);
}

bool Gamepad_SD_card::exists(String path, bool absolute){
    if(absolute)
        return SD.exists(path);
    return SD.exists(dir.path() + path);
}

bool Gamepad_SD_card::is_dir(String path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;

    File tmp;
    tmp = SD.open(path);

    return tmp.isDirectory();
}

bool Gamepad_SD_card::open_file(String path, const char *mode, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;
    
    file = SD.open(path, mode);

    return (file);
}

bool Gamepad_SD_card::open_file(String path, bool absolute){
    return open_file(path, "r", absolute);
}

void Gamepad_SD_card::close_file(){
    file.close();
}

File *Gamepad_SD_card::get_file_reference(){
    return &file;
}

bool Gamepad_SD_card::file_available(){
    return file.available();
}

uint8_t *Gamepad_SD_card::file_read(int start_pos, int chunk_size){
    if(!file)
        return nullptr;
    
    if(chunk_size == 0){
        chunk_size = file.size();
        start_pos = 0;
    }

    if(start_pos >= file.size() || start_pos + chunk_size > file.size())
        return nullptr;

    uint8_t *data = new uint8_t[chunk_size];
    
    file.seek(start_pos);
    file.read(data, chunk_size);

    return data;
}

String Gamepad_SD_card::file_read_string(){
    if(!file)
        return "";
    
    return file.readString();
}

String Gamepad_SD_card::file_getline(){
    if(!file)
        return "";

    return file.readStringUntil('\n');
}

int Gamepad_SD_card::get_file_size(){
    if(!file)
        return 0;
    
    return file.size();
}

bool Gamepad_SD_card::file_write(void *data, size_t size, int start_pos){
    if(!file)
        return 0;
    
    uint8_t *data_ptr = (uint8_t *) data;
    if(start_pos != -1)
        file.seek(start_pos);
    return file.write(data_ptr, size);
}

bool Gamepad_SD_card::file_print(String text){
    if(!file)
        return 0;
    
    return file.print(text);
}

bool Gamepad_SD_card::file_println(String text){
    if(!file)
        return 0;
    
    return file.println(text);
}

void Gamepad_SD_card::save_file(){
    if(!file)
        return;
    
    file.flush();
}

bool Gamepad_SD_card::create_file(String path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;

    File newfile;
    newfile = SD.open(path, FILE_WRITE);
    newfile.close();

    return 1;
}

bool Gamepad_SD_card::remove_file(String path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(path, absolute))
        return 0;

    return SD.remove(path);
}

bool Gamepad_SD_card::rename(String curren_path, String new_path, bool absolute){
    if(!initialized)
        return 0;
    
    if(!process_path(curren_path, absolute))
        return 0;
    if(!process_path(new_path, absolute))
        return 0;
    
    return SD.rename(curren_path, new_path);
}




File *PNG_SD_file_ptr;
PNG *png_SD;

void *PNG_SD_init(const char *filename, int32_t *size){
	*size = PNG_SD_file_ptr -> size();
	return PNG_SD_file_ptr;
}

int32_t PNG_SD_read(PNGFILE *handle, uint8_t *buffer, int32_t length){
	if(!(*PNG_SD_file_ptr))
		return 0;
	return PNG_SD_file_ptr -> read(buffer, length);
}

int32_t PNG_SD_seek(PNGFILE *handle, int32_t position){
	if(!(*PNG_SD_file_ptr))
		return 0;
	return PNG_SD_file_ptr -> seek(position);
}

int PNG_SD_draw(PNGDRAW *pDraw){
    Image_raw16_t *params = (Image_raw16_t *) pDraw -> pUser;

    uint16_t *img_buff_ptr = params -> img_buffer;
	png_SD -> getLineAsRGB565(pDraw, img_buff_ptr + (pDraw -> y * pDraw -> iWidth), PNG_RGB565_BIG_ENDIAN, 0xffffffff);

	if(params -> alpha){
		uint8_t *alpha_buff_ptr = params -> alpha_buffer;
        uint16_t line_size = (pDraw -> iWidth + 7) >> 3;
		png_SD -> getAlphaMask(pDraw, alpha_buff_ptr + (pDraw -> y * line_size), 255);
	}
    

	return 1;
}

Image_raw16_t Gamepad_SD_card::file_read_PNG(bool alpha_channel){
    Image_raw16_t img;
    if(!file)
        return img;
    
    PNG_SD_file_ptr = &file;
	String name = PNG_SD_file_ptr -> name();
	String ext = name.substring(name.length() - 4, name.length());
	if(ext != ".png" && ext != ".PNG")
		return img;
    
    if (heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT) < sizeof(PNG))
		return img;
    png_SD = new PNG;

    int status = png_SD -> open(name.c_str(), PNG_SD_init, NULL, PNG_SD_read, PNG_SD_seek, PNG_SD_draw);
    
    img.create(
        png_SD -> getWidth(), 
        png_SD -> getHeight(), 
        (alpha_channel && png_SD -> hasAlpha())
    );

	if(status == PNG_SUCCESS)
		status = png_SD -> decode(&img, 0);

    delete png_SD;
    return img;
}

void Gamepad_SD_card::write_raw_PNG(Image_raw16_t &img, int start_pos){
    if(!file)
        return;
    
    uint64_t t = millis();

    file.seek(start_pos);
    file.write((uint8_t *)&img.w, sizeof(uint16_t));
    file.write((uint8_t *)&img.h, sizeof(uint16_t));
    file.write((uint8_t *)&img.alpha, sizeof(bool));

    uint8_t *img_ptr = (uint8_t *) img.img_buffer;
    uint8_t *a_ptr = (uint8_t *) img.alpha_buffer;

    file.write(img_ptr, img.w * img.h * 2);
    
    if(img.alpha)
        file.write(a_ptr, img.alpha_buff_size);
}

Image_raw16_t Gamepad_SD_card::read_raw_PNG(int start_pos){
    Image_raw16_t img;
    if(!file)
        return img;
    uint64_t t = millis();

    uint8_t *vars_ptr = new uint8_t[5];
    file.seek(start_pos);
    file.read(vars_ptr, 5);

    img.create(
        *(uint16_t *) (vars_ptr),
        *(uint16_t *) (vars_ptr + 2),
        *(bool *) (vars_ptr + 4)
    );
    delete vars_ptr;

    uint8_t *img_ptr = new uint8_t[img.w * img.h * 2];
    uint8_t *a_ptr = nullptr;

    file.seek(start_pos + 5);

    file.read(img_ptr, img.w * img.h * 2);
    
    if(img.alpha){
        a_ptr = new uint8_t[img.alpha_buff_size];
        file.seek(start_pos + 5 + img.w * img.h * 2);
        file.read(a_ptr, img.alpha_buff_size);
    }

    img.img_buffer = (uint16_t *) img_ptr;
    img.alpha_buffer = a_ptr;
    
    return img;
}