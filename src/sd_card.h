#ifndef GAMEPAD_SD_CARD_H
#define GAMEPAD_SD_CARD_H

#include "SD.h"
#include "FS.h"
#include <PNGdec.h>
#include <vector>

#include "config.h"
#include "image.h"

#ifndef SD_CS_PIN
#define SD_CS_PIN 21
#endif

enum file_type{
    IS_FOLDER,
    IS_FILE
};

struct file_name_t{
    String name;
    bool type;
    String path;
};

class Gamepad_SD_card{
    File dir;
    File file;
    String root = "";
    bool initialized = false;

    bool check_root_level(String path);
    bool process_path(String &path, bool absolute);
public:
    enum SD_status{
        SD_OK,
        SD_FAILED,
        SD_DISCONNECT
    };

    Gamepad_SD_card() = default;
    ~Gamepad_SD_card();

    uint8_t init(String root_limit = "/");

    bool open_dir(String path, bool absolute = false);
    bool open_root_dir();
    std::vector < file_name_t > list_dir();
    String current_dir();
    bool make_dir(String path, bool absolute = false);
    bool remove_dir(String path, bool absolute = false);

    bool exists(String path, bool absolute = false);
    bool is_dir(String path, bool absolute = false);
    
    bool open_file(String path, const char *mode = "r", bool absolute = false);
    bool open_file(String path, bool absolute);
    void close_file();
    File *get_file_reference();

    bool file_available();
    
    uint8_t *file_read(int start_pos = 0, int chunk_size = 0);
    String file_read_string();
    String file_getline();

    template < class T >
    T *file_read_variable(int start_pos = 0){
        return (T*) file_read(start_pos, sizeof(T));
    }

    int get_file_size();

    bool file_write(void *data, size_t size, int start_pos = -1);
    bool file_print(String text);
    bool file_println(String text);

    void save_file();

    bool create_file(String path, bool absolute = false);
    bool remove_file(String path, bool absolute = false);
    
    bool rename(String curren_path, String new_path, bool absolute = false);

    Image_raw16_t file_read_PNG(bool alpha_channel = false);
    void write_raw_PNG(Image_raw16_t &img, int start_pos = 0);
    Image_raw16_t read_raw_PNG(int start_pos = 0);
};

#endif