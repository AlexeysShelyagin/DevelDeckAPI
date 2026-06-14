#ifndef DD_SD_CARD_H
#define DD_SD_CARD_H

#include "SD.h"
#include "FS.h"
#include <PNGdec.h>
#include <vector>

#include "config.h"
#include "image.h"

#ifndef SD_CS_PIN
#define SD_CS_PIN 21
#endif

enum FS_obj_type : uint8_t{
    IS_FILE = 0,
    IS_DIR = 1
};

struct Dir_entry_t{
    String name;
    bool type;
    String path;
};



class DD_SD_card{
    File dir;
    File file;
    String root = "";
    bool initialized = false;

    bool inside_root(String &path);
    bool resolve_path(String &path, bool absolute);

public:
    enum SD_status_t : uint8_t{
        SD_OK,
        SD_FAILED,
        SD_DISCONNECT
    };

    DD_SD_card() = default;
    ~DD_SD_card();

    uint8_t init(String root_limit = "/");

    std::vector < Dir_entry_t > list_dir();
    String current_dir();
    bool exists(String path, bool absolute = false);
    bool is_dir(String path, bool absolute = false);

    bool open_dir(String path, bool absolute = false);
    bool open_parent_dir(uint8_t levels = 1);

    bool open_file(String path, const char *mode = "r", bool absolute = false);
    bool open_file(String path, bool absolute);
    void close_file();

    bool make_dir(String path, bool absolute = false);
    bool remove_dir(String path, bool recursive = false, bool absolute = false);
    
    bool make_file(String path, bool absolute = false);
    bool remove_file(String path, bool absolute = false);
    bool rename(String curren_path, String new_path, bool absolute = false);
    


    File *file_ref();
    int file_size();
    void save_file();
    bool file_available();
    
    uint8_t *read(int start_pos = -1, int chunk_size = 1);
    String read_as_string();
    String getline();

    template < class T >
    T *read_variable(int start_pos = -1){
        return reinterpret_cast < T* > ( read(start_pos, sizeof(T)) );
    }

    bool seek(int position);
    int pos();
    bool write(void *data, size_t size, int start_pos = -1);
    size_t print(String text = "");
    size_t println(String text = "");
    template<typename... Args>
    size_t printf(const char *format, Args&&... args);

    bool read_PNG(Image_raw16_t *img, bool alpha_channel = false);
    bool read_PNG(Image_raw16_t &img, bool alpha_channel = false);
    void write_raw16(Image_raw16_t *img, int start_pos = -1);
    void write_raw16(Image_raw16_t &img, int start_pos = -1);
    bool read_raw16(Image_raw16_t *img, int start_pos = -1);
    bool read_raw16(Image_raw16_t &img, int start_pos = -1);
};

#endif