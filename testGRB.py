from pymycobot.mycobot import MyCobot

port = "/dev/serial/by-id/usb-Silicon_Labs_CP2104_USB_to_UART_Bridge_Controller_023EDF4B-if00-port0"

mc = MyCobot(port)
mc.set_color(255, 0, 0)
