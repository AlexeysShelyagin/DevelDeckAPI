#####################
Buzzer
#####################

.. contents::
    :local:
    :depth: 2



Overview
-----------------

The buzzer can be accessed through the ``gamepad.buzzer`` instance. All sound generation is handled in a separate subprocess, ensuring that audio playback does **not block or interrupt** the main application logic.

The gamepad uses a **passive buzzer**. It can generate a single frequency (tone) at a time, and the volume can be adjusted. By playing tones in sequence, simple 8-bit style melodies and sound effects can be created.

Any sound currently being played can be stopped at any time using :cpp:func:`Gamepad_buzzer::stop`.

.. note::
   If a new playback request is received while another tone or sequence is already playing, the new request is ignored.

.. note::
   The recommended frequency range is ``10 Hz`` to ``7 kHz``. Although the human hearing range extends up to approximately 20 kHz, the buzzer volume decreases significantly above 6 kHz.



Playing a Tone
-----------------

The function :cpp:func:`Gamepad_buzzer::play_tone` plays a tone at the specified frequency **forever**, until :cpp:func:`Gamepad_buzzer::stop` is called.

The function :cpp:func:`Gamepad_buzzer::play_for_time` plays a tone for a specified duration (in milliseconds) and stops automatically. This is useful for short sound effects and notifications.

Common examples
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   while(gamepad.buttons.event_available()){
      uint8_t *event = gamepad.buttons.get_button_event();

      if(event[A_BUT_ID] == BUT_PRESSED)
         gamepad.buzzer.play_tone(400);
      
      if(event[A_BUT_ID] == BUT_RELEASED)
         gamepad.buzzer.stop();

   }

.. code-block:: cpp

   void jump(){
      // Jump sound effect
      gamepad.buzzer.play_for_time(200, 30);

      // ...
   }



Playing a note sequence
--------------------------

The buzzer can play simple melodies using :cpp:func:`Gamepad_buzzer::play_sequence`.

A sequence consists of multiple elements, where each element contains ``frequency`` (tone) and ``duration`` (milliseconds). Elements are played sequentially.

Sequences can be stored either in a ``std::vector`` or in a ``uint16_t[]`` array.

Vector sequence
^^^^^^^^^^^^^^^^

In this approach, the sequence is stored in ``std::vector < Buzzer_element_t >``

The :cpp:struct:`Buzzer_element_t` structure contains  ``freq`` and ``timing`` fields.

This method is useful for dynamically building or modifying melodies.

Piano example
``````````````````````
.. code-block:: cpp

   // Define sequence vector to store melody
   std::vector < Buzzer_element_t > notes;

   void main_loop(){
      // Reading player inputs
      while(gamepad.buttons.event_available()){
         uint8_t *event = gamepad.buttons.get_button_event();
         
         Buzzer_element_t note;
         note.timing = 200;   // Each note lasts 200 ms


         // Add different notes to sequence on player button choise
         if(event[DOWN_BUT_ID] == BUT_PRESSED){
               note.freq = 262;            // C4
               notes.push_back(note);
         }
         if(event[LEFT_BUT_ID] == BUT_PRESSED){
               note.freq = 294;            // D4
               notes.push_back(note);
         }
         if(event[RIGHT_BUT_ID] == BUT_PRESSED){
               note.freq = 330;            // E4
               notes.push_back(note);
         }
         if(event[UP_BUT_ID] == BUT_PRESSED){
               note.freq = 349;            // F4
               notes.push_back(note);
         }

         // Play the whole melody on A pressed
         if(event[A_BUT_ID] == BUT_PRESSED)
               gamepad.buzzer.play_sequence(notes);
         // Clear sequence on B pressed
         if(event[B_BUT_ID] == BUT_PRESSED)
               notes.clear();
         
      }
   }



Array sequence
^^^^^^^^^^^^^^^^

In this approach, a ``uint16_t []`` array is used. Each sequence element is stored as a pair:

- First value: frequency
- Second value: duration

See code example for clarity.

.. note::
   The sequence length parameter represents the number of **frequency-duration pairs**, not the total array length. For a static array, the number of elements can be calculated as ``sizeof(array) / 4``

In the static array case, the melody can be stored in ``PROGMEM`` andplayed directly from flash memory. It can also be loaded from an SD card file as a byte array.

Music example
`````````````````

.. code-block:: cpp

   #define NOTE_E2  82
   #define NOTE_AS2 117
   #define NOTE_B2  123
   #define NOTE_C3  131
   #define NOTE_D3  147
   #define NOTE_E3  165

   const uint16_t PROGMEM music[] = {
      NOTE_E2, 250, NOTE_E3, 125, NOTE_E2, 250, NOTE_D3, 125, NOTE_E2, 250, 
      NOTE_C3, 125, NOTE_E2, 250, NOTE_AS2, 125, NOTE_E2, 250, NOTE_B2, 125, NOTE_C3, 125,
      NOTE_E2, 250, NOTE_E3, 125, NOTE_E2, 250, NOTE_D3, 125, NOTE_E2, 250,
      NOTE_C3, 125, NOTE_E2, 250, NOTE_AS2, 500
   };

   void game_loop(){
      gamepad.buzzer.play_sequence(
         (uint16_t*)&music,            // pointer to the sequence
         sizeof(music) / 4             // N of notes from array size
      );

      delay(5000);
   }



Buzzer volume
-----------------

Buzzer volume is stored in gamepad settings and is changed by a player. However it can be changed in game code.

:cpp:func:`Gamepad_buzzer::change_volume` function change buzzer volume from ``0`` to ``BUZZER_VOLUME_LEVELS``.

.. note::
   The changed volume **does not modify** the user settings and will be discarded after the gamepad reset.

Current volume can be retrieved with :cpp:func:`Gamepad_buzzer::get_volume`.


API reference
-----------------

Functions
^^^^^^^^^^^^^^^^

.. doxygenfunction:: Gamepad_buzzer::play_tone
.. doxygenfunction:: Gamepad_buzzer::stop
.. doxygenfunction:: Gamepad_buzzer::play_for_time
.. doxygenfunction:: Gamepad_buzzer::play_sequence(std::vector<Buzzer_element_t> sequence)
.. doxygenfunction:: Gamepad_buzzer::play_sequence(uint16_t *data, uint32_t size)
.. doxygenfunction:: Gamepad_buzzer::change_volume
.. doxygenfunction:: Gamepad_buzzer::get_volume

Structures
^^^^^^^^^^^^^^^^
.. doxygenstruct:: Buzzer_element_t
   :members:
   :undoc-members: