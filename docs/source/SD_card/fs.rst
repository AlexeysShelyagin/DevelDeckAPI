#########################################
File system (``game_files`` instance)
#########################################


.. contents::
    :local:
    :depth: 2


Overview
-----------------

To access files on the SD card, the ``ddeck.game_files`` instance is used.

.. important::
    Set ``bool GAME_FILES_REQUIRED = true;`` as a global variable. This flag is **required** to use ``game_files``. If the flag is set and no SD card is inserted, the main menu would be called before game.

.. note::
    The **root directory** of ``game_files`` corresponds to the **game directory** (the same directory that contains ``game.ini``). Accessing files from another game is not possible.

.. warning::
    Do not perform any SD card operations while ``ddeck.update_display_threaded()`` is running, as this may cause SPI bus corruption.

The ``game_files`` interface is intended for loading game assets and storing save data.

The filesystem itself is fully managed by the API. The ``game_files`` object acts as an interface for interacting with it, providing commands for file and directory operations.

Similar to a command-line shell, it maintains a **current working directory** and a **currently opened file**, which are used as the context for subsequent operations.


Managing files and directories
-------------------------------

Open directory
^^^^^^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::open_dir`
- :cpp:func:`DD_SD_card::open_parent_dir` currently does **not support** the ``/..`` path syntax.


Open / close file
^^^^^^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::open_file`
- :cpp:func:`DD_SD_card::close_file`


Create directory or file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::make_dir`
- :cpp:func:`DD_SD_card::make_file`
- :cpp:func:`DD_SD_card::open_file` — also creates and opens an empty file if it does not exist.

Rename
^^^^^^^^^

- :cpp:func:`DD_SD_card::rename` - renames a directory or file

Delete directory or file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::remove_dir` - typically used in the recursive mode
- :cpp:func:`DD_SD_card::remove_file`

Directory or file info
^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::exists` - check if a file or directory exists
- :cpp:func:`DD_SD_card::is_dir` - check if a path refers to a directory
- :cpp:func:`DD_SD_card::current_dir` - absolute path of the currently opened directory
- :cpp:func:`DD_SD_card::list_dir` - returns an ``std::vector`` of elements in the current directory
- :cpp:func:`DD_SD_card::file_size`
- :cpp:func:`DD_SD_card::file_ref` - returns an ``FS::File*`` pointer to an opened file
  
.. note::
    Currently oppened file can be referenced using ``current_file()`` macros.

Common examples
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    bool GAME_FILES_REQUIRED = true;        // Game files on SD requirement flag

    void setup() {
        Serial.begin(115200);

        // Dirs and files creation
        ddeck.game_files.make_dir("dir1");
        ddeck.game_files.make_dir("dir1/dir2");
        ddeck.game_files.make_file("dir1/dir2/file.txt");

        // Renaming and deletion
        ddeck.game_files.open_dir("dir1/dir2");
        ddeck.game_files.make_dir("dir3");
        ddeck.game_files.rename("dir3", "delete me");
        ddeck.game_files.remove_dir("delete me");

        // Opening parent dir
        ddeck.game_files.open_parent_dir(2);

        // Listing dir
        Serial.println("Current dir contents: ");
        std::vector < Dir_entry_t > dir = ddeck.game_files.list_dir();
        for(uint16_t i = 0; i < dir.size(); i++){
            Serial.print( (dir[i].type == IS_FILE) ? "FILE:\t" : "DIR:\t");
            Serial.println(dir[i].name);
        }

        // Objects checks
        Serial.print("file.txt exists?  ");
        Serial.println(ddeck.game_files.exists("dir1/dir2/file.txt"));    // Awaiting: 1
        Serial.print("Is dir2 a directory?  ");
        Serial.println(ddeck.game_files.is_dir("dir1/dir2"));             // Awaiting: 1
        Serial.print("Current dir:    ");
        String abs_dir = ddeck.game_files.current_dir();
        Serial.println(abs_dir);                                            // Your root dir
        ddeck.game_files.open_dir(abs_dir + "/dir1", true);
        
        ddeck.game_files.open_file("dir2/file.txt");
        Serial.println("file.txt size: ");
        Serial.println(ddeck.game_files.file_size());                 // Awaiting: 0 (empty file)
        File *file_ref = ddeck.game_files.file_ref();
        ddeck.game_files.close_file();
    }



Reading and writing files
---------------------------

.. note::
    For functions with a read/write ``position`` parameter, pass ``-1`` to use the current cursor position. ``-1`` is the default value.

.. note::
    Only **static** data types can be written to or read from files using :cpp:func:`DD_SD_card::read_variable` and :cpp:func:`DD_SD_card::write`. Pointer contents are **not** handled.

Most functions return a ``bool`` status indicating whether the operation was **successful** (``0`` - FAILED, ``1`` - SUCCESS). These values **should not be ignored** to avoid reading from or writing to invalid data locations.

Reading
^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::seek` - change cursor position
- :cpp:func:`DD_SD_card::pos` - get cursor position
- :cpp:func:`DD_SD_card::file_available` - check if the cursor is at EOF
- :cpp:func:`DD_SD_card::read` - read an N-byte data chunk
- :cpp:func:`DD_SD_card::read_variable` - read **static** data type
- :cpp:func:`DD_SD_card::read_as_string`- read the entire file as ``String``
- :cpp:func:`DD_SD_card::getline` - read a line as ``String`` until **newline**
- :cpp:func:`DD_SD_card::read_PNG` - decode PNG into ``Image_raw16_t``
- :cpp:func:`DD_SD_card::read_raw16` - read a RAW image



Writing
^^^^^^^^^^^^^^

- :cpp:func:`DD_SD_card::seek` - change cursor position
- :cpp:func:`DD_SD_card::pos` - get cursor position
- :cpp:func:`DD_SD_card::save_file` - save current changes without closing file
- :cpp:func:`DD_SD_card::write` - write an N-byte data chunk
- :cpp:func:`DD_SD_card::print` - print a ``String``
- :cpp:func:`DD_SD_card::println` - print a ``String`` with a newline
- :cpp:func:`DD_SD_card::printf` - print a formated string
- :cpp:func:`DD_SD_card::write_raw16` - write a RAW image


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
        ddeck.game_files.open_file("test.bin", "w");              // Open for write

        char array[4] = {'a', 'b', 'c', 'd'};
        ddeck.game_files.write(array, 4);                    // Writing data chunk

        ddeck.game_files.save_file();                             // Save in the middle of writing
        
        ddeck.game_files.println();                          // String + new line

        MyData_t example = {1000, vec2(10, 5)};
        ddeck.game_files.write(&example, sizeof(example));   // Write some data type to file

        ddeck.game_files.print(": struct data");             // Print string

        ddeck.game_files.close_file();                            // Close saves automatically


        // ================READING FILE===================
        ddeck.game_files.open_file("test.bin");                   // Open for read

        Serial.println( ddeck.game_files.read_as_string() + "\n");  // Read all file as string
        
        ddeck.game_files.seek(0);                                 // Return cursor to 0
        Serial.println( ddeck.game_files.getline() );        // Read as string until newline

        Serial.println( ddeck.game_files.pos() );                 // Current cursor position

        // Read some data type from file
        MyData_t *from_file = ddeck.game_files.read_variable < MyData_t > ();
        if(from_file != nullptr){                                   // Check if read successfully
            Serial.println(from_file->score);
            Serial.print(from_file->pos.x);
            Serial.print(" ");
            Serial.println(from_file->pos.y);
        }

        while(ddeck.game_files.file_available())                  // Read data until EOF
            Serial.print((char) *ddeck.game_files.read());   // Write each byte as char
    }


Dynamic data types
`````````````````````

.. code-block:: cpp
    
    String name = "cat";
    ddeck.game_files.write(&name, sizeof(name));                         // Ambiglous
    // ...
    String *read_name = ddeck.game_files.read_variable < String > ();    // Ambiglous

    // Because String buffer is dynamic, it won't be stored in file


Images
```````````````````

.. code-block:: cpp

    void draw_PNG_example(){
        if(!ddeck.game_files.open_file("sample.png"))
            return;
        // Decode PNG from file (with alpha enabled)
        Image_raw16_t png;
        ddeck.game_files.read_PNG(png, true);
        ddeck.game_files.close_file();

        // Write and read RAW images
        ddeck.game_files.open_file("decoded.bin", "a");   // Open for rw
        ddeck.game_files.write_raw16(png, 0);
        Image_raw16_t from_decoded;
        ddeck.game_files.read_raw16(from_decoded, 0);
        ddeck.game_files.close_file();

        Serial.print("Image width:\t");
        Serial.println(png.w);
        Serial.print("Image height:\t");
        Serial.println(png.h);
        Serial.print("Alpha layer: ");
        Serial.println(png.alpha);

        // Draw from raw16
        ddeck.canvas->pushImage(0, 0, png);             // From PNG file
        ddeck.canvas->pushImage(png.w + 20, 0, png);    // From raw file
        ddeck.update_display();
    }



API reference
-----------------

Functions
^^^^^^^^^^^^^^^^

.. doxygenfunction:: DD_SD_card::list_dir
.. doxygenfunction:: DD_SD_card::current_dir
.. doxygenfunction:: DD_SD_card::exists
.. doxygenfunction:: DD_SD_card::is_dir
.. doxygenfunction:: DD_SD_card::open_dir
.. doxygenfunction:: DD_SD_card::open_parent_dir
.. doxygenfunction:: DD_SD_card::open_file(String, bool)
.. doxygenfunction:: DD_SD_card::open_file(String, const char*, bool)
.. doxygenfunction:: DD_SD_card::close_file
.. doxygenfunction:: DD_SD_card::make_dir
.. doxygenfunction:: DD_SD_card::remove_dir
.. doxygenfunction:: DD_SD_card::make_file
.. doxygenfunction:: DD_SD_card::remove_file
.. doxygenfunction:: DD_SD_card::rename

.. doxygenfunction:: DD_SD_card::file_ref
.. doxygenfunction:: DD_SD_card::file_size
.. doxygenfunction:: DD_SD_card::save_file
.. doxygenfunction:: DD_SD_card::file_available

.. doxygenfunction:: DD_SD_card::seek
.. doxygenfunction:: DD_SD_card::pos
    
.. doxygenfunction:: DD_SD_card::read
.. doxygenfunction:: DD_SD_card::read_as_string
.. doxygenfunction:: DD_SD_card::getline
.. doxygenfunction:: DD_SD_card::read_variable
.. doxygenfunction:: DD_SD_card::read_PNG(Image_raw16_t*, bool)
.. doxygenfunction:: DD_SD_card::read_PNG(Image_raw16_t&, bool)
.. doxygenfunction:: DD_SD_card::read_raw16(Image_raw16_t*, int)
.. doxygenfunction:: DD_SD_card::read_raw16(Image_raw16_t&, int)

.. doxygenfunction:: DD_SD_card::write
.. doxygenfunction:: DD_SD_card::print
.. doxygenfunction:: DD_SD_card::println
.. doxygenfunction:: DD_SD_card::printf
.. doxygenfunction:: DD_SD_card::write_raw16(Image_raw16_t*, int)
.. doxygenfunction:: DD_SD_card::write_raw16(Image_raw16_t&, int)



Structures
^^^^^^^^^^^^^^^^

.. doxygenstruct:: Dir_entry_t
    :members:
    :undoc-members:

Enumerations
^^^^^^^^^^^^^^^^

.. doxygenenum:: FS_obj_type