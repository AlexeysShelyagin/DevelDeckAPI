*******************************
Custom fonts
*******************************


.. contents::
    :local:
    :depth: 2




Overview
------------------

It is possible to render any font on the display using the ``.VLW`` format.  
A font can be loaded either from a file or from a byte array.

There are ``FONTS_MAX_N = 8`` slots available for different fonts.  
A user can load a font into any available slot or unload it during runtime.

.. note::
    If the font is loaded from a file, it can consume a significant amount of RAM.  
    Font data stored in ``PROGMEM`` does not allocate additional memory.

The font can be selected by **its ID**.  
Once selected, it replaces the default font.

The default font always has **id = 0** and cannot be unloaded.


Custom font usage
-----------------------

A font can be loaded into the API using :cpp:func:`Gamepad_canvas_t::loadFont`. You must specify the slot ID where the font will be stored.

Previously loaded fonts can be unloaded (freeing heap memory) by ID using :cpp:func:`Gamepad_canvas_t::unloadFont`.

To set a font as active, use :cpp:func:`Gamepad_canvas_t::setFont`.

You can check the currently active font with :cpp:func:`Gamepad_canvas_t::getFontID`.

PROGMEM font
^^^^^^^^^^^^^^^^^^

The ``.vlw`` data can be stored in ``PROGMEM`` and loaded at runtime. The data **must be** stored in a ``const uint8_t[] PROGMEM`` variable.

Common examples
```````````````````

.. code-block:: cpp

    // font containing only 'A' character
    const uint8_t font_demo[] PROGMEM = {
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x0F, 
        0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 
        0x01, 0x00, 0x00, 0x32, 0xA8, 0xE5, 0xFE, 0xA9, 0x00, 0x00, 0x00, 0x72, 0x71, 0x00, 0x00, 0x59, 
        0xF9, 0xEF, 0xB1, 0xB9, 0xEE, 0xF3, 0xC4, 0xAA, 0xDA, 0x03, 0x00, 0x19, 0xF3, 0x92, 0x0A, 0x0D, 
        0x34, 0x2C, 0xDB, 0xFF, 0xFF, 0xA8, 0x00, 0x00, 0x75, 0x9A, 0x00, 0x59, 0xF2, 0xFF, 0xF8, 0x2C, 
        0xED, 0xFF, 0xA8, 0x00, 0x00, 0x9E, 0x2B, 0x00, 0x8F, 0x32, 0x9C, 0x78, 0x00, 0xDE, 0xFF, 0xA8, 
        0x00, 0x00, 0x8D, 0x13, 0x00, 0x01, 0x16, 0xB5, 0x03, 0x00, 0xD4, 0xFF, 0xA8, 0x00, 0x00, 0x43, 
        0x48, 0x00, 0x00, 0xA5, 0x2A, 0x00, 0x00, 0xCB, 0xFF, 0xA8, 0x00, 0x00, 0x00, 0x84, 0x05, 0x58, 
        0x79, 0x00, 0x00, 0x00, 0xC1, 0xFF, 0xA8, 0x00, 0x00, 0x00, 0x19, 0x8F, 0xEB, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xA8, 0x00, 0x00, 0x00, 0x01, 0xAE, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xE7, 0xFF, 
        0xA8, 0x00, 0x00, 0x00, 0x6A, 0xA0, 0x01, 0x00, 0x00, 0x00, 0x00, 0xB8, 0xFF, 0xA8, 0x00, 0x00, 
        0x22, 0xF4, 0xFF, 0xD6, 0x60, 0x05, 0x00, 0x2F, 0xD8, 0xFF, 0xCE, 0x1C, 0x07, 0x96, 0x34, 0x8D, 
        0xF8, 0xFF, 0xE3, 0x7C, 0x47, 0x6E, 0xFF, 0xFF, 0xAB, 0x0B, 0xCD, 0x06, 0x00, 0x1E, 0xA0, 0xC1, 
        0x1E, 0x00, 0x06, 0xC5, 0x90, 0x02, 0x00
    };

    void setup(){
        gamepad.clear_canvas();

        // load font to id 0
        gamepad.canvas->loadFont(font_demo, 1);
        // use loaded font
        gamepad.canvas->setFont(1);

        gamepad.canvas->print("A A A");
        gamepad.update_display();

        gamepad.main_loop();
    }

Load VLW file
^^^^^^^^^^^^^^^^^^

Load VLW file
^^^^^^^^^^^^^^^^^^

To load a font from a ``.vlw`` file, place it in the game folder on the SD card. The file is opened using ``gamepad.game_files`` and passed to :cpp:func:`Gamepad_canvas_t::loadFont` via :cpp:func:`Gamepad_SD_card::get_file_reference`.

.. note::
    Loading a font from the SD card takes time because it must be transferred to RAM. It is recommended to load fonts during initialization.

Common examples
```````````````````

You can :download:`download font for this example <cambria 20.vlw>`

.. code-block:: cpp

    bool GAME_FILES_REQUIRED = true;

    void setup() {
        // open VLW file
        gamepad.game_files.open_file("cambria 20.vlw");
        // load font to slot 1 from an opened file
        gamepad.canvas->loadFont(gamepad.game_files.get_file_reference(), 1);
        gamepad.game_files.close_file();

        gamepad.clear_canvas();
        gamepad.canvas->println("This is a standart font");
        gamepad.canvas->setFont(1);   // select loaded font
        gamepad.canvas->println("This is a loaded from VLW file smooth font");
        gamepad.update_display();

        gamepad.main_loop();
    }




Preparing VLW data
-----------------------

Creating VLW file
^^^^^^^^^^^^^^^^^^^

Creating VLW file
^^^^^^^^^^^^^^^^^^^

The font must first be converted into the ``.VLW`` format.

1. Download the ``.ttf`` font file.
2. Use a converter (recommended: `VLW font creator <https://vlw-font-creator.m5stack.com/>`_).
3. Upload the font, then select size and style. You can also **limit the symbol range** if only specific characters are needed.

.. note::
    It is **strongly recommended** to limit the symbol range. Large ``.vlw`` files waste PROGMEM or RAM.

.. note::
    The VLW format does not support multiple styles. For different sizes, create separate files (e.g. ``Arial_20.vlw``, ``Arial_12.vlw``).

4. *(optional)* Place the file in the game folder for access via ``gamepad.game_files``.

PROGMEM array generation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The generated ``.vlw`` file can be converted into a byte array and stored in ``PROGMEM``.

1. Use an online converter such as `File to hex converter <https://tomeko.net/online_tools/file_to_hex.php?lang=en>`_.
2. Create a header file (e.g. ``montserrat_20.h``) and paste the data:

.. code-block:: cpp

    const uint8_t montserrat20[] PROGMEM = {
        // paste your data here
    };

3. Include the file and load the font:

.. code-block:: cpp

    #include "montserrat_20.h"
    // ...
    gamepad.canvas->loadFont(montserrat20, 1);



API reference
--------------------

Functions
^^^^^^^^^^^^^^^

.. doxygenfunction:: Gamepad_canvas_t::loadFont(File*, uint8_t)
.. doxygenfunction:: Gamepad_canvas_t::loadFont(const uint8_t[], uint8_t)
.. doxygenfunction:: Gamepad_canvas_t::unloadFont
.. doxygenfunction:: Gamepad_canvas_t::setFont
.. doxygenfunction:: Gamepad_canvas_t::getFontID
