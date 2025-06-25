# nucyrosp
    make nucyrosp:via
```
Flashing example for this keyboard:

    make nucyrosp:via:flash


- waveshare RP2040 zero
- pmw3610 optical sensor
- elite joystick
- 0.91"oled
- scrolling wheel
- 32 key


## Bootloader
Enter the bootloader in 3 ways:
* **Bootmagic reset**: Hold down the upper left key while plugging in USB
* **Physical reset button**: Press the RST button twice, rapidly
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
