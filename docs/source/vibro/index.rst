#####################
Vibro
#####################

.. contents::
    :local:
    :depth: 2



Overview
-----------------

The vibration motor can be controlled through the ``ddeckvibro`` instance. All vibration handling is performed in a separate subprocess, ensuring that motor control does **not block or interrupt** the main application logic.

The vibration strength can be adjusted in the range ``0`` to ``255``.

Any ongoing vibration can be stopped immediately using :cpp:func:`DD_vibro::disable`.



Vibration
-----------------

The function :cpp:func:`DD_vibro::enable` enables vibration with specified strength going **forever**, until :cpp:func:`DD_buzzer::stop` is called.

The function :cpp:func:`DD_vibro::enable_for_time` activates vibration for a specified duration (in milliseconds) and automatically stops after the time expires. This is useful for short feedback events such as button press feedback or in-game actions.

Common examples
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   while(ddeckbuttons.event_available()){
      uint8_t *event = ddeckbuttons.get_button_event();

      if(event[A_BUT_ID] == BUT_PRESSED)
         ddeckvibro.enable(255);
      
      if(event[A_BUT_ID] == BUT_RELEASED)
         ddeckvibro.disable();

   }

.. code-block:: cpp

    // Damage tactile feedback
   void player_hit(uint8_t damage){
      uint8_t vibro_strength = damage * 25;     // e.g. damage is 1 to 10 points
      ddeckbuzzer.enable_for_time(
        40,                                     // 40ms
        vibro_strength                          // vibro strength depends on damage  
    );

      // ...
   }



Periodic Vibration
-----------------------

The function :cpp:func:`DD_vibro::enable_periodic` allows generating
a repeating vibration pattern with pauses between pulses.

Function parameters:

- ``time_enabled`` - vibration duration in milliseconds
- ``time_disabled`` - pause duration between vibrations (ms)
- ``repeat_times`` - number of vibration pulses
- ``strength_`` - vibration strength (0-255, default 255)

This can be useful for warning signals or UI notifications.

Common examples
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    void phone_call_effect(){
        ddeckvibro.enable_periodic(
            400,        // 400ms pulse
            600,        // 600ms delay beween pulses
            10,         // repeat 10 times
            255
        );
    }



API reference
-----------------------

Functions
^^^^^^^^^^^^^^^^

.. doxygenfunction:: DD_vibro::enable
.. doxygenfunction:: DD_vibro::disable
.. doxygenfunction:: DD_vibro::enable_for_time
.. doxygenfunction:: DD_vibro::enable_periodic