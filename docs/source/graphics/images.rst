*******************************
Images
*******************************

.. contents::
    :local:
    :depth: 2



Overview
-------------------

DevelDeck-API provides various functionality for storing, decoding and rendering images. Since any graphics data consume much memory it is always a challenge on **optimization of memory and time**.


Useful tips
^^^^^^^^^^^^^^^^^

Here are some tips on optimizing the image workflow:

- **Sprites** which are used **frequently** and have small dimensions can be stored as **RAWS in PROGMEM** for quicker access.
- **Large** and high bitdepth images can be stored in ``.png`` files on the **SD card**
- Render **Large** files directly from SD card.
- **Small** but less important images/sprites can be predecoded from ``.png`` into ``raw16_t/raw8_t`` and stored in binary files on the **SD card**. It can save time on loading them. Does **not** save time for large images, because the decoding speed is higher than reading from SD card.
- Prefer **layers instead canvas** for images. They will **match** the image **bitdepth** better. E.g. you can store multiple layers ``N*Mpx 16bit``, while ``gamepad.canvas`` would be reduced to ``4bit`` and contain only non important graphics.

.. admonition:: Developer notes
    :class: dev-notes

    .. container:: toggle, toggle-hidden

        Welcome to the hell! Image rendering is a world of weird optimizations and workarounds. Because, honestly, render hundreds of kilobytes of pixels per second on a processor with 520KB RAM total is quite a pain in the ass. Images love memory and computational power (for decoding). We have neither memory nor performance :).

        Let's do a simple math. ``320px * 240px * 2bytes(16bit color) ~ 150KB`` of memory per one display-size image. ESP32 heap won't even offer you a single memory block of this size. It is closer to reality to store ``~50KB`` (about 160x160px) bitmaps (if you use :cpp:class:`Image_raw16_t`).

        If your idea allows you, you can shrink your images to 8bit color or even 4bit. Which would be enough for a game background for example.

        TODO: write about canvas bitdepth reduction

        You can try to draw directly from a PNG or raw file during the frame rendering. But the SD card read speed is limited (roughly ``1.4MB/s`` if you are lucky). It can be faster to load raw image from file, but for larger images PNG decoding usually works faster.

        Another problem is that SD card and display share the same SPI bus. So, unfortunately, you can no read the image file during :cpp:func:`Gamepad::update_display_threaded`. So it is recommended to use the precious heap space to store frequently used sprites.

        It is common approach to perform image rendering on different ESP32 core while the main one will handle user input, and calculations for the game. It is possible to render image to the specific ``layer`` while the main thread is rendering on a ``canvas``.


Bitmap containers
-------------------

``Image_raw16_t``
^^^^^^^^^^^^^^^^^^

``Image_raw8_t``
^^^^^^^^^^^^^^^^^^


Rendering images
-------------------


PNG files
-------------------


API reference
-------------------

Data containers
^^^^^^^^^^^^^^^^^

.. doxygenclass:: Image_raw16_t
    :members:
.. doxygenclass:: Image_raw8_t
    :members:


Functions
^^^^^^^^^^^^^^^^^

``gamepad.canvas``
`````````````````````

.. doxygenfunction:: Gamepad_canvas_t::pushImage(int32_t, int32_t, Image_raw16_t&)
.. doxygenfunction:: Gamepad_canvas_t::pushImage(int32_t, int32_t, Image_raw8_t&)
.. doxygenfunction:: Gamepad_canvas_t::pushMaskedImage
.. doxygenfunction:: Gamepad_canvas_t::drawPNGFromFile

``gamepad.game_files``
``````````````````````````

.. doxygenfunction:: Gamepad_SD_card::file_read_PNG
.. doxygenfunction:: Gamepad_SD_card::file_read_raw16
.. doxygenfunction:: Gamepad_SD_card::file_write_raw16