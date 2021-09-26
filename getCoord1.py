from pymycobot.mycobot import MyCobot
import port
import move
import threading

mc = MyCobot(port.port)
threadMov = threading.Thread(target = move.mov1, args = (mc,))
threadMov.start()
while True:
    print(mc.get_coords())

