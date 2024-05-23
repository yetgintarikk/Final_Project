from flask import Flask, request,render_template
from multiprocessing import Queue
import logging
global CommandsQueue,SensorDataQueue
CommandsQueue = Queue()
SensorDataQueue = Queue()

webUI = Flask(__name__)
log = logging.getLogger('werkzeug')
log.disabled = True

@webUI.route('/')
def hello_world():
    return render_template("webserver.html")
    

@webUI.route('/commands')
def get_commands():
    print("getting commands")
    f = open("configs/commands.json", encoding="utf-8")
    jsonText = f.read()
    f.close()
    return jsonText


@webUI.route('/pins')
def get_states():
    f = open("configs/pins.json", encoding="utf-8")
    jsonText = f.read()
    f.close()
    return jsonText

@webUI.route('/sensor')
def get_sensor():
    if(SensorDataQueue.qsize() == 0):
        return [-1,-1]
    return SensorDataQueue.get()


@webUI.route('/command', methods=['POST'])
def get_command():
    CommandsQueue.put(request.data)
    return request.data