from pymycobot.mycobot import MyCobot
from init import init
from init import port

mc = MyCobot(port)
init(mc)

while True:
    print(mc.get_coords())

