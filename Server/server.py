import socket
from transcribe import Transcriber
import parserclass
import stateFollower

class Server:
    def __init__(self, cmdQ,sensorDataQueue) -> None:
        self.sensorDataQueue = sensorDataQueue
        self.sumdata = bytes()
        self.cnt = 0
        self.transcriber = Transcriber()
        self.parser = parserclass.Parser()
        self.stateFollower = stateFollower.StateFollower()
        self.cmdQ = cmdQ
        self.lastSensor = [0,0]

    def sendDataToDevice(self,data):
        self.connection.sendall(data.encode("latin-1"))

    def __data_recived(self,client_address, data):

        if not self.isAudio(data):
            if (data[10] == 255):
                self.sensorDataQueue.put(self.lastSensor)
            else:
                self.lastSensor = [data[10], data[11]]
                self.sensorDataQueue.put([data[10], data[11]])
            return
        
        self.sumdata += data
        self.cnt += 1

        if(self.cnt > 250 * 4):  # 250 * duration(s)
            words = self.transcriber.transcribeAudioApi(self.sumdata)
            parsed_data = self.parser.parse(words)
            command = self.stateFollower.castCommand(parsed_data)
            if command != "xxxx":
                self.sendDataToDevice(command)
            self.cnt = 0
            self.sumdata = bytes()

    def __connected(self, client_address):
        all_cmds = self.stateFollower.sync_states()
        for cmd in all_cmds:
            self.cmdQ.put(cmd.encode("latin-1"))
        print('connected to ', client_address)

    def __closed(self, client_address):
        print("disconnected from ",client_address)

    def listen(self, host,port):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (host,port)
        sock.bind(server_address)
        sock.listen(1)
        print('Listening on {} port {}...'.format(*server_address))
        while True:
            sw_send_cnt = 0
            self.connection, client_address = sock.accept()
            try:
                self.__connected(client_address)
                while True:
                    sw_send_cnt +=1
                    if(self.cmdQ.qsize() > 0 and sw_send_cnt > 25):
                        sw_send_cnt = 0
                        server_cmd = self.cmdQ.get().decode("latin-1")
                        esp_cmd = self.stateFollower.castServerCommand(server_cmd)
                        self.sendDataToDevice(esp_cmd)

                    data = self.connection.recv(128)
                    if data:
                        self.__data_recived(client_address, data)
                    else:
                        break
            finally:
                self.__closed(client_address)
                self.connection.close()
    
    def isAudio(self, data):
        for i in range(10):
            if(i%2 == 0):
                if(data[i] != 0):
                    return True
            else:
                if(data[i] != 1):
                    return True
        return False