*******************************
TFT_eSPI
*******************************

.. contents::
    :local:
    :depth: 2


Overview
-----------------------

The DevelDeck's canvas graphics module inherits all the functionality from the `TFT_eSPI by Bodmer <https://github.com/Bodmer/TFT_eSPI>`_ and extends it.

``ddeck.canvas`` instance and layers **support any graphics functions** from ``TFT_eSPI`` and ``TFT_eSprite``.


Common examples
^^^^^^^^^^^^^^^^^^^^^

Simple example
```````````````````

.. code-block:: cpp

    // Fill background
    ddeck.canvas->fillSprite(TFT_NAVY);

    // Draw some rectangles
    ddeck.canvas->drawRect(10, 10, 100, 50, TFT_WHITE);
    ddeck.canvas->fillRect(120, 10, 80, 50, TFT_RED);
    ddeck.canvas->drawRoundRect(10, 70, 100, 50, 10, TFT_YELLOW);
    ddeck.canvas->fillRoundRect(120, 70, 80, 50, 10, TFT_GREEN);

    // Draw circles and ellipses
    ddeck.canvas->drawCircle(60, 160, 30, TFT_ORANGE);
    ddeck.canvas->fillCircle(160, 160, 30, TFT_CYAN);
    ddeck.canvas->drawEllipse(260, 160, 40, 20, TFT_MAGENTA);

    // Draw lines
    ddeck.canvas->drawLine(0, 0, 320, 240, TFT_WHITE);
    ddeck.canvas->drawFastHLine(0, 230, 320, TFT_LIGHTGREY);
    ddeck.canvas->drawFastVLine(310, 0, 240, TFT_LIGHTGREY);

    // Draw triangles
    ddeck.canvas->drawTriangle(50, 200, 90, 200, 70, 150, TFT_ORANGE);
    ddeck.canvas->fillTriangle(200, 200, 240, 200, 220, 150, TFT_PINK);

    // Draw text in different fonts, colors, sizes
    ddeck.canvas->setTextColor(TFT_WHITE, TFT_NAVY);
    ddeck.canvas->setTextSize(2);
    ddeck.canvas->drawString("TFT_eSPI Demo", 80, 20);

    ddeck.canvas->setTextColor(TFT_YELLOW, TFT_NAVY);
    ddeck.canvas->setTextSize(3);
    ddeck.canvas->drawString("Shapes & Text", 60, 50);

    // Draw numbers and floating points
    ddeck.canvas->setTextColor(TFT_GREEN, TFT_NAVY);
    ddeck.canvas->setTextSize(2);
    ddeck.canvas->drawNumber(12345, 10, 220);
    ddeck.canvas->drawFloat(3.14159, 3, 100, 220);

    // Update display
    ddeck.update_display();


Shape-based drawings
```````````````````````

.. code-block:: cpp

    // Fill background with a custom sky-blue color
    ddeck.canvas->fillSprite(ddeck.canvas->color565(135, 206, 235)); // sky blue

    // Draw a sun with gradient-ish effect
    for (int r = 40; r > 0; r -= 5) {
        ddeck.canvas->fillCircle(280, 50, r, ddeck.canvas->color565(255, 255 - r*3, 0)); // yellow-orange gradient
    }

    // Draw clouds using white and light gray
    ddeck.canvas->fillEllipse(60, 50, 40, 20, TFT_WHITE);
    ddeck.canvas->fillEllipse(90, 55, 35, 15, ddeck.canvas->color565(240, 240, 240));
    ddeck.canvas->fillEllipse(130, 45, 50, 25, ddeck.canvas->color565(250, 250, 250));

    // Draw ground
    ddeck.canvas->fillRect(0, 180, 320, 60, ddeck.canvas->color565(34, 139, 34)); // forest green

    // Draw a tree using brown trunk and green leaves
    ddeck.canvas->fillRect(50, 130, 20, 50, ddeck.canvas->color565(139, 69, 19)); // brown trunk
    ddeck.canvas->fillCircle(60, 120, 30, ddeck.canvas->color565(34, 139, 34));  // green foliage

    // Draw a house
    ddeck.canvas->fillRect(200, 130, 80, 50, ddeck.canvas->color565(178, 34, 34)); // red walls
    ddeck.canvas->fillTriangle(200, 130, 280, 130, 240, 90, ddeck.canvas->color565(165, 42, 42)); // roof brown
    ddeck.canvas->drawRect(220, 150, 20, 30, TFT_BLACK); // door
    ddeck.canvas->drawRect(245, 140, 20, 20, TFT_WHITE); // window

    // Draw text with shadow effect
    ddeck.canvas->setTextSize(2);
    ddeck.canvas->setCursor(62, 12);
    ddeck.canvas->setTextColor(TFT_DARKGREY);
    ddeck.canvas->print("My Little Scene");
    ddeck.canvas->setCursor(60, 10);
    ddeck.canvas->setTextColor(TFT_WHITE);
    ddeck.canvas->print("My Little Scene");

    // Update display
    ddeck.update_display();



TFT_eSPI extentions (for DevelDeck)
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
        Graphics_params_t params_to_restore = ddeck.canvas->graphicsParams();

        // Function changes parameters inside
        ddeck.canvas->setCursor(0, 0);
        ddeck.canvas->setTextColor(TFT_RED);
        // ...
        //

        // Restore parameters
        ddeck.canvas->setGraphicsParams(params_to_restore);
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