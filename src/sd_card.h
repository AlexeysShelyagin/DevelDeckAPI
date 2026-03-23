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

enum FS_obj_type{
    IS_DIR,
    IS_FILE
};


/**
 * @brief FS object data container
 * 
 */
struct File_name_t{
    String name;
    bool type;      /** Type according to `FS_obj_type` */
    String path;    /** Absolute path */
};

class Gamepad_SD_card{
    File dir;
    File file;
    String root = "";
    bool initialized = false;

    bool check_root_level(String path);
    bool process_path(String &path, bool absolute);
public:
    enum SD_status_t{
        SD_OK,
        SD_FAILED,
        SD_DISCONNECT
    };

    Gamepad_SD_card() = default;
    ~Gamepad_SD_card();

    uint8_t init(String root_limit = "/");



    /**
     * @brief Open directory
     * 
     * @param path string with path to the dir
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool open_dir(String path, bool absolute = false);

    /**
     * @brief Open parent dir 
     * 
     * @param levels number of levels going up in the file tree
     * @return true: success
     * @return false: failed
     */
    bool open_parent_dir(uint8_t levels = 1);

    /**
     * @brief Get list of all files and dirs in the current dir
     * 
     * @return std::vector < file_name_t >: `std::vector` filled with `file_name_t` data of each element 
     */
    std::vector < File_name_t > list_dir();

    /**
     * @brief Get absolute path to currently opened dir
     * 
     * @return String: path string
     */
    String current_dir();

    /**
     * @brief Create directory
     * 
     * @param path path to new directory
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool make_dir(String path, bool absolute = false);

    /**
     * @brief Delete directory
     * 
     * @param path path to directory
     * @param recursive set true for recursive deletion
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool remove_dir(String path, bool recursive = false, bool absolute = false);



    /**
     * @brief Check if dir/file exists
     * 
     * @param path path to directory
     * @param absolute set true if path is absolute
     * @return true: exsits
     * @return false: does not exist
     */
    bool exists(String path, bool absolute = false);

    /**
     * @brief Checks if element is dir or file
     * 
     * @param path path to the element
     * @param absolute set true if path is absolute
     * @return true: element is dir
     * @return false: element is file
     */
    bool is_dir(String path, bool absolute = false);
    


    /**
     * @brief Open file
     * 
     * @note If file does not exist it would be created
     * 
     * @note `FS::file` opened file reference is stored inside `Gamepad::game_files`
     * 
     * @param path path to the file
     * @param mode "r" | "w" | "a" - read, write or append modes. Read is default
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool open_file(String path, const char *mode = "r", bool absolute = false);

    /**
     * @brief Open file
     * 
     * @note If file does not exist it would be created
     * 
     * @note `FS::file` opened file reference is stored inside `Gamepad::game_files`
     * 
     * @param path path to the file
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool open_file(String path, bool absolute);

    /**
     * @brief Close opened file
     * 
     * @note File saved automatically on close
     */
    void close_file();

    /**
     * @brief Get direct access to `FS::file`
     * 
     * @return File*: pointer to the opened file
     */
    File *get_file_reference();



    /**
     * @brief Check if there is data to read in file
     * 
     * @return true 
     * @return false 
     */
    bool file_available();
    
    /**
     * @brief Read data from file
     * 
     * @param start_pos starting position (in bytes)
     * @param chunk_size reading size (in bytes)
     * @return uint8_t*: pointer to an array with file data and size equal to `chunk_size`
     * @return nullptr if failed
     */
    uint8_t *file_read(int start_pos = -1, int chunk_size = 1);

    /**
     * @brief Read all file as text
     * 
     * @return String: text of file
     */
    String file_read_string();

    /**
     * @brief Read file as text until `\n`
     * 
     * @return String: line of text
     */
    String file_getline();


    /**
     * @brief Read raw variable data from file
     * 
     * @tparam T type of variable to read
     * @param start_pos starting position (in bytes)
     * @return T*: pointer to read variable
     * @return nullptr if failed
     */
    template < class T >
    T *file_read_variable(int start_pos = -1){
        return (T*) file_read(start_pos, sizeof(T));
    }

    /**
     * @brief Get size of opened file
     * 
     * @return int 
     */
    int get_file_size();


    /**
     * @brief Move cursor to the position in file
     * 
     * @param position cursor position
     * @return true: success
     * @return false: failed
     */
    bool seek(int position);

    /**
     * @brief Get current cursor position
     * 
     * @return int: position
     */
    int pos();

    /**
     * @brief Write data to file
     * 
     * @note Use file_write(`&var`, sizeof(`var`)) to write variable of any type
     * 
     * @param data pointer to data variable
     * @param size size of data chunk (in bytes)
     * @param start_pos position in file to write. Set -1 to write to the end of file
     * @return true: success
     * @return false: failed
     */
    bool file_write(void *data, size_t size, int start_pos = -1);

    /**
     * @brief Write a string to the end of file
     * 
     * @param text 
     * @return true: success
     * @return false: failed
     */
    bool file_print(String text = "");

    /**
     * @brief Write a string with newline to the end of file
     * 
     * @param text 
     * @return true: success
     * @return false: failed
     */
    bool file_println(String text = "");


    /**
     * @brief Save performed file changes
     * 
     */
    void save_file();



    /**
     * @brief Create file
     * 
     * @param path path to the file
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool create_file(String path, bool absolute = false);

    /**
     * @brief Delete file
     * 
     * @param path path to the file
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool remove_file(String path, bool absolute = false);
    
    /**
     * @brief Rename file/dir
     * 
     * @param curren_path path to the existing element
     * @param new_path new path of the element
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool rename(String curren_path, String new_path, bool absolute = false);



    /**
     * @brief Decode opened file as PNG to the `Image_raw16_t` container
     * 
     * @param alpha_channel: set false to ignore alpha channel
     * @return Image_raw16_t: decoded image
     */
    bool file_read_PNG(Image_raw16_t &img, bool alpha_channel = false);

    /**
     * @brief Encode `Image_raw16_t` to file
     * 
     * @param img image data
     * @param start_pos position in file to write (inb bytes)
     */
    void file_write_raw16(Image_raw16_t &img, int start_pos = -1);

    /**
     * @brief Read already decoded PNG from file
     * 
     * @param start_pos data chunk start position (in bytes)
     * @return Image_raw16_t: resulting image
     */
    bool file_read_raw16(Image_raw16_t &img, int start_pos = -1);
};

#endif