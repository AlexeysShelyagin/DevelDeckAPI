*******************************
TFT_eSPI
*******************************

.. contents::
    :local:
    :depth: 2


Overview
-----------------------

The gamepad canvas graphics module inherits all the functionality from the `TFT_eSPI by Bodmer <https://github.com/Bodmer/TFT_eSPI>`_ and extends it.

``ddeckcanvas`` instance and layers **support any graphics functions** from ``TFT_eSPI`` and ``TFT_eSprite``.


Common examples
^^^^^^^^^^^^^^^^^^^^^

Simple example
```````````````````

.. code-block:: cpp

    // Fill background
    ddeckcanvas->fillSprite(TFT_NAVY);

    // Draw some rectangles
    ddeckcanvas->drawRect(10, 10, 100, 50, TFT_WHITE);
    ddeckcanvas->fillRect(120, 10, 80, 50, TFT_RED);
    ddeckcanvas->drawRoundRect(10, 70, 100, 50, 10, TFT_YELLOW);
    ddeckcanvas->fillRoundRect(120, 70, 80, 50, 10, TFT_GREEN);

    // Draw circles and ellipses
    ddeckcanvas->drawCircle(60, 160, 30, TFT_ORANGE);
    ddeckcanvas->fillCircle(160, 160, 30, TFT_CYAN);
    ddeckcanvas->drawEllipse(260, 160, 40, 20, TFT_MAGENTA);

    // Draw lines
    ddeckcanvas->drawLine(0, 0, 320, 240, TFT_WHITE);
    ddeckcanvas->drawFastHLine(0, 230, 320, TFT_LIGHTGREY);
    ddeckcanvas->drawFastVLine(310, 0, 240, TFT_LIGHTGREY);

    // Draw triangles
    ddeckcanvas->drawTriangle(50, 200, 90, 200, 70, 150, TFT_ORANGE);
    ddeckcanvas->fillTriangle(200, 200, 240, 200, 220, 150, TFT_PINK);

    // Draw text in different fonts, colors, sizes
    ddeckcanvas->setTextColor(TFT_WHITE, TFT_NAVY);
    ddeckcanvas->setTextSize(2);
    ddeckcanvas->drawString("TFT_eSPI Demo", 80, 20);

    ddeckcanvas->setTextColor(TFT_YELLOW, TFT_NAVY);
    ddeckcanvas->setTextSize(3);
    ddeckcanvas->drawString("Shapes & Text", 60, 50);

    // Draw numbers and floating points
    ddeckcanvas->setTextColor(TFT_GREEN, TFT_NAVY);
    ddeckcanvas->setTextSize(2);
    ddeckcanvas->drawNumber(12345, 10, 220);
    ddeckcanvas->drawFloat(3.14159, 3, 100, 220);

    // Update display
    ddeckupdate_display();


Shape-based drawings
```````````````````````

.. code-block:: cpp

    // Fill background with a custom sky-blue color
    ddeckcanvas->fillSprite(ddeckcanvas->color565(135, 206, 235)); // sky blue

    // Draw a sun with gradient-ish effect
    for (int r = 40; r > 0; r -= 5) {
        ddeckcanvas->fillCircle(280, 50, r, ddeckcanvas->color565(255, 255 - r*3, 0)); // yellow-orange gradient
    }

    // Draw clouds using white and light gray
    ddeckcanvas->fillEllipse(60, 50, 40, 20, TFT_WHITE);
    ddeckcanvas->fillEllipse(90, 55, 35, 15, ddeckcanvas->color565(240, 240, 240));
    ddeckcanvas->fillEllipse(130, 45, 50, 25, ddeckcanvas->color565(250, 250, 250));

    // Draw ground
    ddeckcanvas->fillRect(0, 180, 320, 60, ddeckcanvas->color565(34, 139, 34)); // forest green

    // Draw a tree using brown trunk and green leaves
    ddeckcanvas->fillRect(50, 130, 20, 50, ddeckcanvas->color565(139, 69, 19)); // brown trunk
    ddeckcanvas->fillCircle(60, 120, 30, ddeckcanvas->color565(34, 139, 34));  // green foliage

    // Draw a house
    ddeckcanvas->fillRect(200, 130, 80, 50, ddeckcanvas->color565(178, 34, 34)); // red walls
    ddeckcanvas->fillTriangle(200, 130, 280, 130, 240, 90, ddeckcanvas->color565(165, 42, 42)); // roof brown
    ddeckcanvas->drawRect(220, 150, 20, 30, TFT_BLACK); // door
    ddeckcanvas->drawRect(245, 140, 20, 20, TFT_WHITE); // window

    // Draw text with shadow effect
    ddeckcanvas->setTextSize(2);
    ddeckcanvas->setCursor(62, 12);
    ddeckcanvas->setTextColor(TFT_DARKGREY);
    ddeckcanvas->print("My Little Scene");
    ddeckcanvas->setCursor(60, 10);
    ddeckcanvas->setTextColor(TFT_WHITE);
    ddeckcanvas->print("My Little Scene");

    // Update display
    ddeckupdate_display();



TFT_eSPI extentions (for gamepad)
-----------------------------------

Graphics parameters saving
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To avoid multiline redefinitions of graphic styles DevelDeck-API provides cleaner solution. :cpp:struct:`Graphics_params_t` is a container for multiple graphics parameters which can be stored in a single variable. It stores:

- ``font_id`` - custom font id. Uses :cpp:func:`DD_canvas_t::setFont`
- ``text_size``, ``text_colors`` - text parameters. Uses :cpp:func:`TFT_eSPI::setTextSize`, :cpp:func:`TFT_eSPI::setTextColor`
- ``wrap_x``, ``wrap_y`` - text wrap parameters. Uses :cpp:func:`TFT_eSPI::setTextWrap`
- ``cur_x``, ``cur_y`` - text text cursor. Uses :cpp:func:`TFT_eSPI::setCursor`
- ``orig_x``, ``orig_y`` - text zero origin. Uses :cpp:func:`TFT_eSPI::setOrigin`


Currently used parameters can be read into variable via :cpp:func:`DD_canvas_t::graphicsParams`.

Parameters can be applied using :cpp:func:`DD_canvas_t::DD_canvas_t::setGraphicsParams`.

Parameters can be reset to default :cpp:func:`DD_canvas_t::DD_canvas_t::setDefaultGraphicsParams`.

Common examples
`````````````````````

.. code-block:: cpp

    void render_block() {
        // Save previously used parameters
        Graphics_params_t params_to_restore = ddeckcanvas->graphicsParams();

        // Function changes parameters inside
        ddeckcanvas->setCursor(0, 0);
        ddeckcanvas->setTextColor(TFT_RED);
        // ...
        //

        // Restore parameters
        ddeckcanvas->setGraphicsParams(params_to_restore);
    }



Text formating
^^^^^^^^^^^^^^^^^^^^

- The line spacing can be adjusted using :cpp:func:`DD_canvas_t::setLineSpacing`. It uses a multiplier value relative to the font height.


Custom fonts
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Check :doc:`Custom fonts docs <fonts>` for details.

Rendering images
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Check :doc:`images docs <images>` for details.

|



API reference
------------------------------

Functions
^^^^^^^^^^^^^^

.. doxygenfunction:: DD_canvas_t::setDefaultGraphicsParams
.. doxygenfunction:: DD_canvas_t::setGraphicsParams
.. doxygenfunction:: DD_canvas_t::graphicsParams

.. doxygenfunction:: DD_canvas_t::setLineSpacing

Structures
^^^^^^^^^^^^^^

.. doxygenstruct:: Graphics_params_t
    :members:
    :undoc-members:

|



TFT_eSPI reference
------------------------------

.. doxygenclass:: TFT_eSPI
    :project: TFT_eSPI-docs
    :members:
    :undoc-members:

TFT_eSprite reference
^^^^^^^^^^^^^^^^^^^^^^^^^^
.. doxygenclass:: TFT_eSprite
    :project: TFT_eSPI-docs
    :members:
    :undoc-members: