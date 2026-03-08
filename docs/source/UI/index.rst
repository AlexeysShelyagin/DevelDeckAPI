#####################
Standart UI
#####################

.. contents::
    :local:
    :depth: 2



Overview
-------------------

Gamepad API provides basic built-in UI functions for minimal necessary interactions with gamepad. It gives user possibility to change sytem settings and load different game from SD card.

UI functions are functions of ``gamepad`` instance.

.. note::
   Each gamepadAPI UI function call discards unhandled ``button events``.

Main menu
-------------------

Main menu is an initial gamepad menu where player can e.g. start game, change game or go to settings.

:cpp:func:`Gamepad::Gamepad::main_menu` is used to call main menu.

.. warning::
   It is a **game developer's responsibility** to include :cpp:func:`Gamepad::Gamepad::main_menu` call in game. In other case it would be impossible to change game and settings (only forced call would work).

.. note::
   Main menu can be called forcively by:
      1. Hold **menu button** before gamepad power on
      2. Hold **menu button** for ``~5s`` until main menu appears

Common examples
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   while(gamepad.buttons.event_available()){
      uint8_t *event = gamepad.buttons.get_button_event();

      if(event[MENU_BUT_ID] == BUT_PRESSED)
         gamepad.main_menu();
   }



File manager
-------------------

:cpp:func:`Gamepad::file_manager` is an embeded file manager for player file selection. This function returns ``String`` with **absolute path** to the file/folder.

.. note::
   File manager ``root`` directory is a **game files directory**. It is not possible to manage another game files.

.. note::
   Works only when game files initialized via ``gamepad.game_files_required()``.

Common examples
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   void load_game_from_save(){
      String path = gamepad.file_manager();

      Serial.println(path);

      // ...
   }



Game selection menu
-------------------

Game can be changed to another without main menu call, but directly with a use of :cpp:func:`Gamepad::Gamepad::select_game_menu`.



Settings menu
-------------------

Gamepad settings menu can be opened without main menu call, but directly with a use of :cpp:func:`Gamepad::Gamepad::settings_menu`.



API reference
-------------------

Functions
^^^^^^^^^^^^^^^^

.. doxygenfunction:: Gamepad::main_menu
.. doxygenfunction:: Gamepad::file_manager
.. doxygenfunction:: Gamepad::select_game_menu
.. doxygenfunction:: Gamepad::settings_menu