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


/**
 * @brief FS object data container
 * 
 */
struct Dir_entry_t{
    String name;
    bool type;      /** Type according to `FS_obj_type` */
    String path;    /** Absolute path */
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



    /**
     * @brief Get list of all files and dirs in the current dir
     * 
     * @return std::vector < file_name_t >: `std::vector` filled with `file_name_t` data of each element 
     */
    std::vector < Dir_entry_t > list_dir();

    /**
     * @brief Get absolute path to currently opened dir
     * 
     * @return String: path string
     */
    String current_dir();

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
     * @brief Create file
     * 
     * @param path path to the file
     * @param absolute set true if path is absolute
     * @return true: success
     * @return false: failed
     */
    bool make_file(String path, bool absolute = false);

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
     * @brief Get direct access to `FS::file`
     * 
     * @return File*: pointer to the opened file
     */
    File *file_ref();

    /**
     * @brief Get size of opened file
     * 
     * @return int 
     */
    int file_size();

    /**
     * @brief Save performed file changes (flush())
     * 
     */
    void save_file();

    /**
     * @brief Check if there is data to read in file
     * 
     * @return true 
     * @return false 
     */
    bool file_available();
    

    /**
     * @brief Read byte data from file
     * 
     * @param start_pos starting position (in bytes)
     * @param chunk_size reading size (in bytes)
     * @return uint8_t*: pointer to an array with file data and size equal to `chunk_size`
     * @return nullptr if failed
     */
    uint8_t *read(int start_pos = -1, int chunk_size = 1);

    /**
     * @brief Read all file as text
     * 
     * @return String: text of file
     */
    String read_as_string();

    /**
     * @brief Read file as text until `\n`
     * 
     * @return String: line of text
     */
    String getline();

    /**
     * @brief Read raw variable data from file
     * 
     * @tparam T type of variable to read
     * @param start_pos starting position (in bytes)
     * @return T*: pointer to read variable
     * @return nullptr if failed
     */
    template < class T >
    T *read_variable(int start_pos = -1){
        return reinterpret_cast < T* > ( read(start_pos, sizeof(T)) );
    }


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
    bool write(void *data, size_t size, int start_pos = -1);

    /**
     * @brief Print a string to the end of file
     * 
     * @param text
     */
    size_t print(String text = "");

    /**
     * @brief Print a string with newline to the end of file
     * 
     * @param text
     */
    size_t println(String text = "");

    /**
     * @brief Print formated string into file
     * 
     * @tparam Args 
     * @param format 
     * @param args 
     * @return size_t 
     */
    template<typename... Args>
    size_t printf(const char *format, Args&&... args);


    /**
     * @brief Decode opened file as PNG to the `Image_raw16_t` container
     * 
     * @param img (ptr) variable where decoded image would be stored
     * @param alpha_channel: set false to ignore alpha channel
     * @return Image_raw16_t: decoded image
     */
    bool read_PNG(Image_raw16_t *img, bool alpha_channel = false);

    /**
     * @brief Decode opened file as PNG to the `Image_raw16_t` container
     * 
     * @param img variable where decoded image would be stored
     * @param alpha_channel: set false to ignore alpha channel
     * @return Image_raw16_t: decoded image
     */
    bool read_PNG(Image_raw16_t &img, bool alpha_channel = false);

    /**
     * @brief Encode `Image_raw16_t` to file
     * 
     * @param img (ptr) image data
     * @param start_pos position in file to write (inb bytes)
     */
    void write_raw16(Image_raw16_t *img, int start_pos = -1);

    /**
     * @brief Encode `Image_raw16_t` to file
     * 
     * @param img image data
     * @param start_pos position in file to write (inb bytes)
     */
    void write_raw16(Image_raw16_t &img, int start_pos = -1);

    /**
     * @brief Read already decoded PNG from file
     * 
     * @param img (ptr) image variable to write onto it
     * @param start_pos data chunk start position (in bytes)
     * @return Image_raw16_t: resulting image
     */
    bool read_raw16(Image_raw16_t *img, int start_pos = -1);

    /**
     * @brief Read already decoded PNG from file
     * 
     * @param img image variable to write onto it
     * @param start_pos data chunk start position (in bytes)
     * @return Image_raw16_t: resulting image
     */
    bool read_raw16(Image_raw16_t &img, int start_pos = -1);
};

#endif