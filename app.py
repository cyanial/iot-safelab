from flask import Flask, jsonify, send_from_directory

from hw_iotda import hw_device
from config import config

import os


app = Flask(__name__, static_folder='web/build')
# app.config['JSON_AS_ASCII'] = False

hw = hw_device(config)

# Serve React App
@app.route('/', defaults={'path': ''})
@app.route('/<path:path>')
def serve(path):
    if path != "" and os.path.exists(app.static_folder + '/' + path):
        return send_from_directory(app.static_folder, path)
    else:
        return send_from_directory(app.static_folder, 'index.html')
# RESTful API

@app.route('/api/v1/devinfo', methods=['GET'])
def get_devinfo():
    # dic = {
    #     "device_id": "61839c96d0a1830285b994f6_20211104", 
    #     "shadow": [
    #         {
    #             "service_id": "Smoke", 
    #             "desired": {}, 
    #             "reported": {
    #                 "properties": {
    #                     "Smoke_Value": "19.713",
    #                     "BeepStatus": "OFF", 
    #                     "DoorStatus": "ON", 
    #                     "Temperature": 27, 
    #                     "Humidity": 52, 
    #                     "Luminance": 36
    #                     },
    #                 "event_time": "20211104T133133Z"
    #             }, 
    #             "version": 679
    #         }
    #     ]
    # }
    return jsonify(hw.getDeviceInfo())

@app.route('/api/v1/devmesg', methods=['GET'])
def get_devmsg():
    return jsonify(hw.getDeviceMessage())

@app.route('/api/v1/beepon', methods=['POST'])
def beep_on():
    hw.beepON()
    return 'BEEP ON'

@app.route('/api/v1/beepoff', methods=['POST'])
def beep_off():
    hw.beepOFF()
    return 'BEEP OFF'

@app.route('/api/v1/dooron', methods=['POST'])
def door_on():
    hw.doorON()
    return 'DOOR ON'
    
@app.route('/api/v1/dooroff', methods=['POST'])
def door_off():
    hw.doorOFF()
    return 'DOOR OFF'

if __name__ == '__main__':
    app.run()