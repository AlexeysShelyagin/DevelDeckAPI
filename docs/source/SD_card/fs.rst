#########################################
File system (``game_files`` instance)
#########################################


.. contents::
    :local:
    :depth: 2


Overview
-----------------

To access files on the SD card, the ``gamepad.game_files`` instance is used.

.. important::
    Set ``bool GAME_FILES_REQUIRED = true;`` as a global variable. This flag is **required** to use ``game_files``. If the flag is set and no SD card is inserted, the main menu would be called before game.

.. note::
    The **root directory** of ``game_files`` corresponds to the **game directory** (the same directory that contains ``game.ini``). Accessing files from another game is not possible.

.. warning::
    Do not perform any SD card operations while ``gamepad.update_display_threaded()`` is running, as this may cause SPI bus corruption.

The ``game_files`` interface is intended for loading game assets and storing save data.

The filesystem itself is fully managed by the API. The ``game_files`` object acts as an interface for interacting with it, providing commands for file and directory operations.

Similar to a command-line shell, it maintains a **current working directory** and a **currently opened file**, which are used as the context for subsequent operations.


Managing files and directories
-------------------------------

Open directory
^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::open_dir`
- :cpp:func:`Gamepad_SD_card::open_parent_dir` currently does **not support** the ``/..`` path syntax.


Open / close file
^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::open_file`
- :cpp:func:`Gamepad_SD_card::close_file`


Create directory or file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::make_dir`
- :cpp:func:`Gamepad_SD_card::create_file`
- :cpp:func:`Gamepad_SD_card::open_file` — also creates and opens an empty file if it does not exist.

Rename
^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::rename` - renames a directory or file

Delete directory or file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::remove_dir` - typically used in the recursive mode
- :cpp:func:`Gamepad_SD_card::remove_file`

Directory or file info
^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::exists` - check if a file or directory exists
- :cpp:func:`Gamepad_SD_card::is_dir` - check if a path refers to a directory
- :cpp:func:`Gamepad_SD_card::current_dir` - absolute path of the currently opened directory
- :cpp:func:`Gamepad_SD_card::list_dir` - returns an ``std::vector`` of elements in the current directory
- :cpp:func:`Gamepad_SD_card::get_file_size`
- :cpp:func:`Gamepad_SD_card::get_file_reference` - returns an ``FS::File*`` pointer to an opened file

Common examples
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    bool GAME_FILES_REQUIRED = true;        // Game files on SD requirement flag

    void setup() {
        Serial.begin(115200);

        // Dirs and files creation
        gamepad.game_files.make_dir("dir1");
        gamepad.game_files.make_dir("dir1/dir2");
        gamepad.game_files.create_file("dir1/dir2/file.txt");

        // Renaming and deletion
        gamepad.game_files.open_dir("dir1/dir2");
        gamepad.game_files.make_dir("dir3");
        gamepad.game_files.rename("dir3", "delete me");
        gamepad.game_files.remove_dir("delete me");

        // Opening parent dir
        gamepad.game_files.open_parent_dir(2);

        // Listing dir
        Serial.println("Current dir contents: ");
        std::vector < File_name_t > dir = gamepad.game_files.list_dir();
        for(uint16_t i = 0; i < dir.size(); i++){
            Serial.print( (dir[i].type == IS_FILE) ? "FILE:\t" : "DIR:\t");
            Serial.println(dir[i].name);
        }

        // Objects checks
        Serial.print("file.txt exists?  ");
        Serial.println(gamepad.game_files.exists("dir1/dir2/file.txt"));    // Awaiting: 1
        Serial.print("Is dir2 a directory?  ");
        Serial.println(gamepad.game_files.is_dir("dir1/dir2"));             // Awaiting: 1
        Serial.print("Current dir:    ");
        String abs_dir = gamepad.game_files.current_dir();
        Serial.println(abs_dir);                                            // Your root dir
        gamepad.game_files.open_dir(abs_dir + "/dir1", true);
        
        gamepad.game_files.open_file("dir2/file.txt");
        Serial.println("file.txt size: ");
        Serial.println(gamepad.game_files.get_file_size());                 // Awaiting: 0 (empty file)
        File *file_ref = gamepad.game_files.get_file_reference();
        gamepad.game_files.close_file();
    }



Reading and writing files
---------------------------

.. note::
    For functions with a read/write ``position`` parameter, pass ``-1`` to use the current cursor position. ``-1`` is the default value.

.. note::
    Only **static** data types can be written to or read from files using :cpp:func:`Gamepad_SD_card::file_read_variable` and :cpp:func:`Gamepad_SD_card::file_write`. Pointer contents are **not** handled.

Most functions return a ``bool`` status indicating whether the operation was **successful** (``0`` - FAILED, ``1`` - SUCCESS). These values **should not be ignored** to avoid reading from or writing to invalid data locations.

Reading
^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::seek` - change cursor position
- :cpp:func:`Gamepad_SD_card::pos` - get cursor position
- :cpp:func:`Gamepad_SD_card::file_available` - check if the cursor is at EOF
- :cpp:func:`Gamepad_SD_card::file_read` - read an N-byte data chunk
- :cpp:func:`Gamepad_SD_card::file_read_variable` - read **static** data type
- :cpp:func:`Gamepad_SD_card::file_read_string`- read the entire file as ``String``
- :cpp:func:`Gamepad_SD_card::file_getline` - read a line as ``String`` until **newline**
- :cpp:func:`Gamepad_SD_card::file_read_PNG` - decode PNG into ``Image_raw16_t``
- :cpp:func:`Gamepad_SD_card::read_raw16` - read a RAW image



Writing
^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::seek` - change cursor position
- :cpp:func:`Gamepad_SD_card::pos` - get cursor position
- :cpp:func:`Gamepad_SD_card::save_file` - save current changes without closing file
- :cpp:func:`Gamepad_SD_card::file_write` - write an N-byte data chunk
- :cpp:func:`Gamepad_SD_card::file_print` - print a ``String``
- :cpp:func:`Gamepad_SD_card::file_println` - print a ``String`` with a newline
- :cpp:func:`Gamepad_SD_card::write_raw16` - write a RAW image


Common examples
^^^^^^^^^^^^^^^^^^^

Overall
`````````````````

.. code-block:: cpp

    struct MyData_t{
        int score;
        vec2 pos;
    };

    void read_write_example(){
        // ================WRITING FILE===================
        gamepad.game_files.open_file("test.bin", "w");              // Open for write

        char array[4] = {'a', 'b', 'c', 'd'};
        gamepad.game_files.file_write(array, 4);                    // Writing data chunk

        gamepad.game_files.save_file();                             // Save in the middle of writing
        
        gamepad.game_files.file_println();                          // String + new line

        MyData_t example = {1000, vec2(10, 5)};
        gamepad.game_files.file_write(&example, sizeof(example));   // Write some data type to file

        gamepad.game_files.file_print(": struct data");             // Print string

        gamepad.game_files.close_file();                            // Close saves automatically


        // ================READING FILE===================
        gamepad.game_files.open_file("test.bin");                   // Open for read

        Serial.println( gamepad.game_files.file_read_string() + "\n");  // Read all file as string
        
        gamepad.game_files.seek(0);                                 // Return cursor to 0
        Serial.println( gamepad.game_files.file_getline() );        // Read as string until newline

        Serial.println( gamepad.game_files.pos() );                 // Current cursor position

        // Read some data type from file
        MyData_t *from_file = gamepad.game_files.file_read_variable < MyData_t > ();
        if(from_file != nullptr){                                   // Check if read successfully
            Serial.println(from_file->score);
            Serial.print(from_file->pos.x);
            Serial.print(" ");
            Serial.println(from_file->pos.y);
        }

        while(gamepad.game_files.file_available())                  // Read data until EOF
            Serial.print((char) *gamepad.game_files.file_read());   // Write each byte as char
    }


Dynamic data types
`````````````````````

.. code-block:: cpp
    
    String name = "cat";
    gamepad.game_files.file_write(&name, sizeof(name));                         // Ambiglous
    // ...
    String *read_name = gamepad.game_files.file_read_variable < String > ();    // Ambiglous

    // Because String buffer is dynamic, it won't be stored in file


Images
```````````````````

.. code-block:: cpp

    void draw_PNG_example(){
        if(!gamepad.game_files.open_file("sample.png"))
            return;
        // Decode PNG from file (with alpha enabled)
        Image_raw16_t png;
        gamepad.game_files.file_read_PNG(png, true);
        gamepad.game_files.close_file();

        // Write and read RAW images
        gamepad.game_files.open_file("decoded.bin", "a");   // Open for rw
        gamepad.game_files.write_raw16(png, 0);
        Image_raw16_t from_decoded;
        gamepad.game_files.file_read_raw16(from_decoded, 0);
        gamepad.game_files.close_file();

        Serial.print("Image width:\t");
        Serial.println(png.w);
        Serial.print("Image height:\t");
        Serial.println(png.h);
        Serial.print("Alpha layer: ");
        Serial.println(png.alpha);

        // Draw from raw16
        gamepad.canvas->pushImage(0, 0, png);             // From PNG file
        gamepad.canvas->pushImage(png.w + 20, 0, png);    // From raw file
        gamepad.update_display();
    }



API reference
-----------------

Functions
^^^^^^^^^^^^^^^^

.. doxygenfunction:: Gamepad_SD_card::open_dir
.. doxygenfunction:: Gamepad_SD_card::open_parent_dir
.. doxygenfunction:: Gamepad_SD_card::open_file(String, bool)
.. doxygenfunction:: Gamepad_SD_card::open_file(String, const char*, bool)
.. doxygenfunction:: Gamepad_SD_card::close_file
.. doxygenfunction:: Gamepad_SD_card::make_dir
.. doxygenfunction:: Gamepad_SD_card::create_file
.. doxygenfunction:: Gamepad_SD_card::rename
.. doxygenfunction:: Gamepad_SD_card::remove_dir
.. doxygenfunction:: Gamepad_SD_card::remove_file
.. doxygenfunction:: Gamepad_SD_card::exists
.. doxygenfunction:: Gamepad_SD_card::is_dir
.. doxygenfunction:: Gamepad_SD_card::current_dir
.. doxygenfunction:: Gamepad_SD_card::list_dir
.. doxygenfunction:: Gamepad_SD_card::get_file_size
.. doxygenfunction:: Gamepad_SD_card::get_file_reference

.. doxygenfunction:: Gamepad_SD_card::seek
.. doxygenfunction:: Gamepad_SD_card::pos
.. doxygenfunction:: Gamepad_SD_card::file_available
.. doxygenfunction:: Gamepad_SD_card::save_file
    
.. doxygenfunction:: Gamepad_SD_card::file_read
.. doxygenfunction:: Gamepad_SD_card::file_read_variable
.. doxygenfunction:: Gamepad_SD_card::file_read_string
.. doxygenfunction:: Gamepad_SD_card::file_getline
.. doxygenfunction:: Gamepad_SD_card::file_read_PNG
.. doxygenfunction:: Gamepad_SD_card::file_read_raw16

.. doxygenfunction:: Gamepad_SD_card::file_write
.. doxygenfunction:: Gamepad_SD_card::file_print
.. doxygenfunction:: Gamepad_SD_card::file_println
.. doxygenfunction:: Gamepad_SD_card::file_write_raw16



Structures
^^^^^^^^^^^^^^^^

.. doxygenstruct:: File_name_t
    :members:
    :undoc-members:

Enumerations
^^^^^^^^^^^^^^^^

.. doxygenenum:: FS_obj_type