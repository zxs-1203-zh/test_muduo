from pymycobot.mycobot import MyCobot
from port import port

mc = MyCobot(port)
mc.release_all_servos()

