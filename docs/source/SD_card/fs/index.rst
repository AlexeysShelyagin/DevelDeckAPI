#########################################
File system (``game_files`` instance)
#########################################


.. contents::
    :local:
    :depth: 2


Overview
-----------------

To access files on the SD card, the ``gamepad.game_files`` instance is used.

.. note::
    Call ``gamepad.game_files_required()`` before game initialization. This flag is **required** to use ``game_files``. If the flag is set and no SD card is inserted, the game will not start.

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
- :cpp:func:`Gamepad_SD_card::open_parent_dir`


Open / close file
^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::open_file`
- :cpp:func:`Gamepad_SD_card::close_file`


Create directory or file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::make_dir`
- :cpp:func:`Gamepad_SD_card::create_file` or :cpp:func:`Gamepad_SD_card::open_file` creates and opens empty files

Rename
^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::rename` - renames directory or file

Delete directory or file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::remove_dir`
- :cpp:func:`Gamepad_SD_card::remove_file`

Directory or file info
^^^^^^^^^^^^^^^^^^^^^^^^^^

- :cpp:func:`Gamepad_SD_card::exists`
- :cpp:func:`Gamepad_SD_card::is_dir`
- :cpp:func:`Gamepad_SD_card::current_dir`
- :cpp:func:`Gamepad_SD_card::list_dir`
- :cpp:func:`Gamepad_SD_card::get_file_size`
- :cpp:func:`Gamepad_SD_card::get_file_reference`

Common examples
^^^^^^^^^^^^^^^^^^^



Reading files
-----------------

- :cpp:func:`Gamepad_SD_card::file_read`
- :cpp:func:`Gamepad_SD_card::file_read_string`
- :cpp:func:`Gamepad_SD_card::file_getline`
- :cpp:func:`Gamepad_SD_card::file_read_variable`
- :cpp:func:`Gamepad_SD_card::file_read_PNG`
- :cpp:func:`Gamepad_SD_card::read_raw16`
- :cpp:func:`Gamepad_SD_card::file_available`



Writing files
-----------------

- :cpp:func:`Gamepad_SD_card::file_write`
- :cpp:func:`Gamepad_SD_card::file_print`
- :cpp:func:`Gamepad_SD_card::file_println`
- :cpp:func:`Gamepad_SD_card::write_raw16`
- :cpp:func:`Gamepad_SD_card::save_file`



API reference
-----------------

Functions
^^^^^^^^^^^^^^^^

.. doxygenclass:: Gamepad_SD_card
    :members:

.. doxygenstruct:: File_name_t
    :members:
    :undoc-members:

.. doxygenenum:: FS_obj_type