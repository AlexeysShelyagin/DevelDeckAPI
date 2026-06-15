*******************************
Game folder structure
*******************************


General concept
-----------------

To make a game on an SD card be recognizable by the DevelDeck, it is **necessary** to create a **game folder** containing all the resources. The folder name should match the game name for better navigation by the player.

The minimum required file structure is as follows:

.. code-block:: yaml

    game folder:
        - game.ini
        - firmware.bin

Where:

- ``game.ini`` - the game configuration file, serving as a marker for the DevelDeck.
- ``firmware.bin`` - the executable game source code file.

A game folder can contain additional resource files such as icon, sprites, audio sequences, save files, etc.

Here is an example of an extended game folder structure:

.. code-block:: yaml

    game folder:
        - game.ini
        - firmware.bin
        - icon.png
        resourses:
            - "*any files*"
        saves:
            - "*player saves*"


``game.ini`` File
-----------------

The ``game.ini`` file is the core configuration for the game. It is parsed by the DevelDeck API to display game information and locate all necessary files for installation.

Obligatory Fields
^^^^^^^^^^^^^^^^^^^^^

- ``name`` - *string*, the game name.
- ``game_path`` - *string*, the **relative path** to the ``.bin`` game executable.

Optional Fields
^^^^^^^^^^^^^^^^^^^^^

- ``description`` - *string*, a description to show in the game selection menu. Can be **multiline**; use ``\`` at the end of a line to continue.
- ``icon`` - *string*, **relative path** to the game icon. **Must be a PNG file**.
- ``minimum_flash`` - *int (MB)*, the minimum flash size required on the ESP32 to install the game.



Common examples
^^^^^^^^^^^^^^^^^^^

.. code-block:: ini
    :name: game.ini

    name = Sample game

    description = This is description\
    which can be multiline\
    to fit the screen

    game_path = firmware.bin

    icon = resource/icon.png

    minimum_flash = 4

Game icon
-------------------

The game icon is displayed in the game selection menu. Requirements:

- File format must be ``.png``.
- Maximum dimensions: ``64x64`` pixels.
- An alpha channel is **allowed**.