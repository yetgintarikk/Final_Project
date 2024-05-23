from multiprocessing import Process, Queue
from server import Server
from flaskServer import webUI,CommandsQueue, SensorDataQueue

IP_ADRESS = "YOUR_IP_ADRESS" # "192.168.24.99"
SERVER_PORT = YOUR_SERVER_PORT # 7070

def ServerProcessAudio(cmdQ, sensorDataQueue):
    server = Server(cmdQ,sensorDataQueue)
    server.listen(IP_ADRESS,SERVER_PORT)

if __name__ == '__main__':
    print("Start")
    proc_server_audio = Process(target=ServerProcessAudio, args=(CommandsQueue,SensorDataQueue,))

    proc_server_audio.start()
    webUI.run(host=SERVER_PORT)
    proc_server_audio.join() # proc_server_audio processinin işini sonlandırmasını bekler
    print("End")