#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "requests.h"       /* The custom request numbers we use */

#include "ws2812/light_ws2812.h"
#include "led.h"


// Digispark Internal LED
#define LED_PORT_OUTPUT PORTB
#define LED_PORT_DDR DDRB
#define PIN_LED     PB1
#define DELAY_MS    2000

static uint8_t do_reset = 0;
static usbRequest_t *rq;
static uint8_t auto_mode = 1;
static uint16_t auto_mode_delay = 10000;
static uint16_t auto_mode_count = 0;


void __attribute__((noreturn)) reset(void) { /* reset to bootloader */
    leds_off(); // disable all leds

    cli(); // disable interrupts which could mess with changing prescaler
    CLKPR = 0b10000000; // enable prescaler speed change
    CLKPR = 0; // set prescaler to default (16mhz) mode required by bootloader
    void (*ptrToFunction)(); // allocate a function pointer
    ptrToFunction = 0x0000; // set function pointer to bootloader reset vector
    (*ptrToFunction)(); // jump to reset, which bounces in to bootloader
    for(;;);
}

int main(void) {
    cli();
    
    // Init LED pin as output
    LED_PORT_DDR |= (1 << PIN_LED);
    // Light up LED
    LED_PORT_OUTPUT |= (1 << PIN_LED);
    
    uchar   i;

    //wdt_enable(WDTO_1S);
    wdt_disable();
    /* If you don't use the watchdog, replace the call above with a wdt_disable().
     * On newer devices, the status of the watchdog (on/off, period) is PRESERVED
     * OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    LED_PORT_DDR |= _BV(PIN_LED);   /* make the LED bit an output */
    sei();
    leds_init();
    for(;;){                /* main event loop */
        if (do_reset) {
            reset();
        }
        wdt_reset();
        usbPoll();
        leds_animate_frame();
        
        _delay_ms(10);
        
        auto_mode_count++;
        if (auto_mode && auto_mode_count == auto_mode_delay) {
            auto_mode_count = 0;
            leds_set_mode_random();
        }
        
        // Check for USB bus reset
        /*if (!(USBIN & USBMASK)) {
            struct cRGB black = {0, 0, 0};
            leds_set(black);
            leds_write();
            while(!(USBIN & USBMASK));
            usbDeviceConnect();
        }*/
    }
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[12])
{
    rq = (void *)data;

    if(rq->bRequest == USB_RQ_ONOFF) { /* echo -- used for reliability tests */
        if (rq->wValue.word == 0) {
            leds_off();
        } else {
            leds_init();
        }
    } else if (rq->bRequest == USB_RQ_MODE_AUTO) {
        uint8_t mode = rq->wValue.bytes[0];
        auto_mode = mode;
    } else if (rq->bRequest == USB_RQ_ANIMATION_COLOR_MODE) {
        uint8_t mode = rq->wValue.bytes[0];
        uint16_t data = rq->wIndex.word;
        leds_set_animation_color_mode(mode, data);
    } else if (rq->bRequest == USB_RQ_ANIMATION_MODE) {
        leds_set_animation_mode(rq->wValue.bytes[0], rq->wIndex.word);
    } else if (rq->bRequest == USB_RQ_ANIMATION_COLOR_DELAY) {
        struct cRGB color;
        color.r = rq->wValue.bytes[0];
        color.g = rq->wValue.bytes[1];
        color.b = rq->wIndex.bytes[0];
        uint8_t delay = rq->wIndex.bytes[1];
        leds_set_animation_color_delay(color, delay);
    } else if (rq->bRequest == USB_RQ_BOOTLOADER) { /* enter bootloader */
        do_reset = 1;
        return 0;
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
    return 1;
}

