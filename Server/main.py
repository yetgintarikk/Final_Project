from multiprocessing import Process
from server import Server
from flaskServer import webUI,CommandsQueue, SensorDataQueue

IP_ADRESS = "YOUR_SERVER_IP_ADRESS"
SERVER_PORT = YOUR_SERVER_PORT

def ServerProcessAudio(cmdQ, sensorDataQueue):
    server = Server(cmdQ,sensorDataQueue)
    server.listen(IP_ADRESS,SERVER_PORT)

if __name__ == '__main__':
    print("Start")
    proc_server_audio = Process(target=ServerProcessAudio, args=(CommandsQueue,SensorDataQueue,))

    proc_server_audio.start()
    webUI.run(host=IP_ADRESS)
    proc_server_audio.join() # proc_server_audio processinin işini sonlandırmasını bekler
    print("End")





