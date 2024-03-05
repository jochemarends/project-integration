#include <array>
#include <cstdint>
#include <iostream>

#include "driver/gpio.h"

std::array<std::uint8_t, 3> led_pins{
    GPIO_NUM_36, 
    GPIO_NUM_39,
    GPIO_NUM_34,
};

void blink() {
    for (std::uint8_t pin : led_pins) {
        
    }
}

extern "C" void app_main() try {
    blink()
}
catch (GPIOException& e) {
    std::cerr << e.what() << '\n';
}

