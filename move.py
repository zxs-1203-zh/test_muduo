from pymycobot.mycobot import MyCobot
from pymycobot.genre import Angle, Coord
from init import init
import time

def mov1(mc : MyCobot):
    init(mc)
    time.sleep(10)
    mc.send_angle(Angle.J1.value, 90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J1.value, -90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J1.value, 0, 30)
    time.sleep(5)
    mc.send_angle(Angle.J2.value, 90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J2.value, -90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J2.value, 0, 30)
    time.sleep(5)
    mc.send_angle(Angle.J3.value, 90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J3.value, -90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J3.value, 0, 30)
    time.sleep(5)
    mc.send_angle(Angle.J4.value, 90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J4.value, -90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J4.value, 0, 30)
    time.sleep(5)
    mc.send_angle(Angle.J5.value, 90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J5.value, -90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J5.value, 0, 30)
    time.sleep(5)
    mc.send_angle(Angle.J6.value, 90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J6.value, -90, 30)
    time.sleep(5)
    mc.send_angle(Angle.J6.value, 0, 30)
    time.sleep(5)

def mov2(mc : MyCobot):
    init(mc)

def mov3(mc : MyCobot):
    init(mc)

