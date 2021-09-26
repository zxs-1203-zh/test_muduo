from pymycobot.mycobot import MyCobot
from port import port
from pymycobot.genre import Angle


mc = MyCobot(port)
mc.send_angle(Angle.J1.value, 90, 10)

