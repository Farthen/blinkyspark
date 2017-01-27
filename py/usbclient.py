import usb1
import sys

VENDOR_ID = 0x16c0
PRODUCT_ID = 0x05dc


class USBBlinky:
    def __init__(self):
        pass
    
    def on_off(self, on):
        if on:
            value = 1
        else:
            value = 0
        self.send_command(0, value)
    
    def send_command(self, cmd, value, data=0, payload=[]):
        with self.handle.claimInterface(0):
            print("Command {}: {}, {}".format(cmd, value, data))
            self.handle.controlWrite(usb1.REQUEST_TYPE_VENDOR, cmd, value, data, payload, 0)

    def connect(self):
        self.context = usb1.USBContext()
        self.context.open()
        dev = None
        for device in self.context.getDeviceIterator(skip_on_error=True):
            if device.getVendorID() == VENDOR_ID and device.getProductID() == PRODUCT_ID:
                if device.getSerialNumber() == 'BL8K':
                    dev = device
        if dev is None:
            print(":(")
            return None
        self.handle = dev.open()
    
    def close(self):
        self.handle.close()
    
    def set_animation(self, animation, data=0):
        blinky.send_command(0x4, animation, data)
    
    def set_animation_color_mode(self, mode, data=0):
        blinky.send_command(0x3, mode, data)
    
    def set_color(self, r, g, b, delay=0):
        blinky.send_command(0x5, g * 256 + r, delay * 256 + b)
    
    def reset(self):
        blinky.send_command(0xff, 0x0000, 0x0000)


if __name__ == '__main__':
    blinky = USBBlinky()
    blinky.connect()
    
    if sys.argv[1] == 'off':
        blinky.on_off(0)
    elif sys.argv[1] == 'on':
        blinky.on_off(1)
    elif sys.argv[1] == 'animation':
        data = 0
        if len(sys.argv) > 3:
            data = int(sys.argv[3])
        blinky.set_animation(int(sys.argv[2]), data)
    elif sys.argv[1] == 'fade':
        data = 0
        if len(sys.argv) > 3:
            data = int(sys.argv[3])
        blinky.set_animation_color_mode(int(sys.argv[2]), data)
    elif sys.argv[1] == 'color':
        delay = 0
        if len(sys.argv) > 5:
            delay = int(sys.argv[5])
        blinky.set_color(int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), delay)
    elif sys.argv[1] == 'reset':
        blinky.reset()
    blinky.close()


