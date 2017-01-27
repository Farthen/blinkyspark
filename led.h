#include "ws2812/light_ws2812.h"

void leds_set(struct cRGB color);
void leds_init();
void leds_off();
void leds_write();
void leds_animate_frame();
void leds_set_animation_mode(uint8_t mode, uint16_t data);
void leds_set_brightness(uint8_t brightness);
void leds_set_mode_random();
void set_animation_color_mode(uint8_t mode, uint16_t data);
void leds_set_animation_color_delay(struct cRGB color, uint8_t delay);
