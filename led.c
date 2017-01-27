#include "ws2812/light_ws2812.h"
#include "usbdrv.h"
#include "led.h"
#include <stdlib.h>

#define LED_COUNT 90
#define LED_OFFSET_CORNER_0 0
#define LED_OFFSET_CORNER_1 23
#define LED_OFFSET_CORNER_2 44
#define LED_OFFSET_CORNER_3 67

#define ANIMATION_MODE_COUNT 5

enum animation_color_mode_t {
    ANIMATION_COLOR_MODE_SOLID = 0,
    ANIMATION_COLOR_MODE_RAINBOW = 1,
};

struct cHSV {
  uint8_t h;
  uint8_t s;
  uint8_t v;
};

// color config
static struct cRGB animation_color;
static enum animation_color_mode_t animation_color_mode;
static uint16_t animation_color_mode_data;
static uint8_t brightness = 100;

// animation config
static uint8_t animation_mode;
static uint16_t animation_data;
static uint8_t animation_delay;

// state
static uint8_t animation_wait;
static uint8_t anim_offset = 0;
static uint8_t rainbow_offset = 0;


struct cHSV hsv;

struct cRGB leds[LED_COUNT];

void leds_init() {
    hsv.h = 0;
    hsv.v = 100;
    hsv.s = 240;
    
    animation_mode = 1;
    animation_color.r = 0xff;
    animation_color.g = 0xff;
    animation_color.b = 0xff;
}

void leds_off() {
    animation_mode = 0;
    animation_color.r = 0;
    animation_color.g = 0;
    animation_color.b = 0;
    
    leds_set(animation_color);
    leds_write();
}

void leds_set(struct cRGB color) {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = color;
    }
}

void leds_write() {
    ws2812_setleds(leds, LED_COUNT);
}

struct cRGB hsv2rgb(struct cHSV hsv) 
{
  uint16_t r, g, b, h, s, v; //this function works with uint's between 0 and 255
  uint16_t f, p, q, t;

  struct cRGB colorRGB;

  h = hsv.h;
  s = hsv.s;
  v = hsv.v;

  //if saturation is 0, the color is a shade of grey
  if(s == 0) {
    b = v;
    g = b;
    r = g;
  }
  //if saturation > 0, more complex calculations are needed
  else
  {
    h *= 6; //to bring hue to a number between 0 and 6*255, better for the calculations
    f = h % 256;//the fractional part of h
    p = (v * (255 - s)) / 256;
    q = (v * (255 - (s * f)/256)) / 256;
    t = (v * (255 - (s * (255 - f))/256)) / 256;
    
    switch(h/256)
    {
      case 0: r=v; g=t; b=p; break;
      case 1: r=q; g=v; b=p; break;
      case 2: r=p; g=v; b=t; break;
      case 3: r=p; g=q; b=v; break;
      case 4: r=t; g=p; b=v; break;
      case 5: r=v; g=p; b=q; break;
      default: r = g = b = 0; break;
    }
  }
  colorRGB.r = r;
  colorRGB.g = g;
  colorRGB.b = b;

  return colorRGB;
}

void leds_set_brightness(uint8_t bright) {
    brightness = bright;
}

void leds_set_animation_mode(uint8_t mode, uint16_t data) {
    struct cRGB black = {0, 0, 0};
    leds_set(black);
    animation_mode = mode;
    animation_data = data;
    anim_offset = 0;
}

void leds_set_animation_color_mode(uint8_t mode, uint16_t data) {
    animation_color_mode = mode;
    animation_color_mode_data = data;
}

void leds_set_animation_color_delay(struct cRGB color, uint8_t delay) {
    animation_color = color;
    animation_delay = delay;
}

void leds_set_mode_random() {
    animation_mode = rand() % ANIMATION_MODE_COUNT;
    if (animation_mode == 2) {
        animation_data = (rand() % 9) + 1;
    } else if (animation_mode == 3) {
        animation_data = (rand() % LED_COUNT);
    } else if (animation_mode == 4) {
        animation_data = rand() % 9;
    }
    
    animation_color_mode = ANIMATION_COLOR_MODE_RAINBOW;
    animation_delay = (rand() % 5) + 5;
}

void leds_animate_frame() {
    if (animation_wait < animation_delay) {
        animation_wait++;
        return;
    }
    
    animation_wait = 0;

    struct cRGB black = {0, 0, 0};
    struct cRGB white = {0xff, 0xff, 0xff};

    if (animation_mode == 0) {
        // solid color
        leds_set(animation_color);
        leds_write();
    }
    if (animation_mode == 1) {
        // rainbow circling around
        for(int i = 0; i < LED_COUNT; i++) {
            hsv.h = (((i * 255) / LED_COUNT) + rainbow_offset) % 255;
            leds[i] = hsv2rgb(hsv);
            usbPoll();
        }
        
        leds_write();
        rainbow_offset += 1;
    }
    if (animation_mode == 2) {
        // colored dot running in a circle
        leds_set(black);
        
        if (animation_data == 0) animation_data = 1;
        
        for (int i = 0; i < animation_data; i++) {
            leds[(anim_offset + i) % LED_COUNT] = animation_color;
        }
        leds_write();
    }
    if (animation_mode == 3) {
        // fill from both sides
        struct cRGB color = animation_color;
        uint8_t pos = (anim_offset) % (LED_COUNT / 2);
        
        if (anim_offset >= (LED_COUNT / 2)) {
            color = black;
        }
        
        leds[(pos + animation_data) % LED_COUNT] = color;
        leds[(LED_COUNT - pos + animation_data) % LED_COUNT -1] = color;
        leds_write();
    }
    if (animation_mode == 4) {
        // wheel
        struct cRGB color = animation_color;
        uint8_t size = LED_COUNT / animation_data;
        uint8_t pos = (anim_offset) % size;
        
        if (anim_offset % (2 * size) >= size) {
            color = black;
        }
        
        for (int i = 0; i < animation_data; i++) {
            leds[pos + i * size] = color;
        }
        
        leds_write();
    }
    
    anim_offset += 1;
    if (anim_offset >= 90) {
        anim_offset = 0;
    }
    
    if (animation_color_mode == ANIMATION_COLOR_MODE_RAINBOW) {
        hsv.h += 1;
        animation_color = hsv2rgb(hsv);
    }
}


