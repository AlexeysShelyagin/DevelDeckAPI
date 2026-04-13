#####################
Display
#####################

.. contents::
    :local:
    :depth: 2


Canvas
-----------------

The ``gamepad.canvas`` instance is the main drawing surface used to render graphics for the display. It represents an image buffer on which any :ref:`graphics functions <graphics_section>` can be composed before showing on the display.

Canvas can be cleared (filled black) using :cpp:func:`Gamepad::clear_canvas`.

It is implied to fully render frame on canvas and then :ref:`update the display <disp_update_section>`.

Since ``gamepad.canvas`` is is a pointer, its functions must be accessed via ``->``.

.. code-block:: cpp

    gamepad.clear_canvas();
    gamepad.canvas->fillRect(0, 0, 100, 100, TFT_RED);
    gamepad.canvas->setCursor(0, 0);



.. _disp_update_section:

Display Update
-----------------

Call :cpp:func:`Gamepad::update_display` for update. It is a procedure of transfering ``gamepad.canvas`` image buffer to the display. Only after that player would see the rendered image.

Since image buffer stores large amount of data, it **takes a while** to transfer it to the display.

.. note::
    It takes from ``23.7ms`` to ``29.3 ms`` (depending on contents) to update display.

.. note::
    Frequent updates can cause :ref:`flickering <flickering_section>`.



.. _disp_threaded_update_section:

Threaded Update
-----------------

Updates can cause lags if the calculations beteween updates take considering amount of time. In this case further optimization is needed.

The :cpp:func:`Gamepad::update_display_threaded` optimizes display update by performing transfer in the second core. This method is preferable in fps-sensetive appliacations. While being more optimized the method requires more from the user.

This section also applicable to :cpp:func:`Gamepad::update_layer_threaded`.

The typical aplication **timeline diagram** is presented below.

.. figure:: threaded_diagram.png
   :alt: Threaded update timeline diagram
   :width: 90%
   :align: center

   Common threaded update execution timeline

.. note::
    Frequent updates can cause :ref:`flickering <flickering_section>`.

Availability
^^^^^^^^^^^^^^^^^^

.. warning::
    It is not possible to update ``gamepad.canvas`` or use ``gamepad.game_files`` during threaded update due to image buffer memory region and SPI bus are busy. Interaction with them may cause **core fatal error**.

.. figure:: threaded_corruption.png
   :alt: Forbiden action diagram
   :width: 90%
   :align: center

   Unallowed parallel operations

To check if the parallel update is not running use :cpp:func:`update_display_threaded_available`. If the frame is ready but threaded update is busy, code **must wait** until it would be available.

Common examples
^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    /*
    * In this example fps would hold at around 40 FPS before calc_time reaches
    * 23ms (display update time), after that it will drop with the higher
    * "calculation" time. 
    */
    
    uint16_t calc_time = 1;
    uint64_t last_update = 0;

    void loop() {
        // delay will pretend lots of calculatons
        // calc_time changes over time to show the parallelization effect
        delay(calc_time / 10);
        if(++calc_time > 500)
            calc_time = 1;

        // wait until previous update finishes
        while(!gamepad.update_display_threaded_available());

        gamepad.clear_canvas();
        gamepad.canvas->fillRect(millis() / 10 % 320, 100, 10, 10, TFT_RED);  // running rectangle
        gamepad.canvas->setCursor(0, 0);
        float fps = 1000.0 / (millis() - last_update);
        // print current fps and calc_time (time program spent on calculations)
        gamepad.canvas->printf("fps: %f      calc_time: %d", fps, calc_time / 10);
        
        // start threaded update
        last_update = millis();
        gamepad.update_display_threaded();
    }




Window (region-wise) update
---------------------------

It is possible to transfer only a specific rectangular window of a canvas or layer to the display instead of updating the entire frame.

Both :cpp:func:`Gamepad::update_display` and :cpp:func:`Gamepad::update_display_threaded` support region-based updates. The region is defined via the following parameters:

- ``x0``, ``y0`` — starting (upper-left) point of the window to transfer (relative to canvas origin)
- ``w``, ``h`` — width and height of the window

Using partial updates can significantly reduce display refresh time when only a portion of the screen changes.

.. note::
    The displayed image may temporarily differ from the full canvas contents when partial updates are used.

.. note::
    Region updates are applied to **only one layer** (including the base canvas) and do not affect others. Coordinates are always relative to the layer position (``(0, 0)`` for ``gamepad.canvas``).

Optimization
^^^^^^^^^^^^^^^^^

Region-based updates are a **highly effective optimization technique** for **FPS-sensitive applications**. Below are common strategies for leveraging this feature.

Dynamic bounding box
`````````````````````````

The most common approach is to **track or compute** the bounding box of all changes that occur between frames, and update only that region.

For example:

- Moving objects - update only previous and new positions
- UI changes - update only affected widgets

Segment-wise rendering
````````````````````````

This approach involves **modifying the canvas while it is being transferred** to the display. The goal is to overlap rendering and transfer operations to reduce idle time.

Instead of updating a single large region, the image is divided into smaller blocks (tiles), and processed sequentially.

.. figure:: segmentwise_rendering.png
   :alt: Segement-wise update diagram
   :width: 60%
   :align: center

   Segement-wise update memory diagram

.. note::
    Only appliacations that use **pixel-by-pixel** or **fine-grained** rendering benefit significantly from this approach.

.. TODO: demonstration image

Key requirements for implementation:
  
- A **fine-grained renderer** (pixel-by-pixel or block-by-block)
- A **render-transfer** synchronization mechanism
- Use of :cpp:func:`Gamepad::update_display_threaded` for asynchronous transfer
- Proper handling of **flickering and tearing**



.. _graphics_section:

Graphics
-----------------
.. toctree::
   :maxdepth: 1

   graphics/eSPI.rst
   graphics/fonts.rst
   graphics/images.rst
   graphics/layers.rst



.. _flickering_section:

Flickering
-----------------



API reference
-----------------

.. doxygenfunction:: Gamepad::clear_canvas
.. doxygenfunction:: Gamepad::update_display
.. doxygenfunction:: Gamepad::update_display_threaded
.. doxygenfunction:: Gamepad::update_display_threaded_available