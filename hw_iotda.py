# coding: utf-8

from huaweicloudsdkcore.auth.credentials import BasicCredentials
from huaweicloudsdkiotda.v5.region.iotda_region import IoTDARegion
from huaweicloudsdkcore.exceptions import exceptions
from huaweicloudsdkiotda.v5 import *




class hw_iot():
    """HUAWEI IoTDA API Pack"""

    def __init__(self, cfg):
        """Init: connect to IoTDA"""
        credentials = BasicCredentials(cfg['ak'], cfg['sk'])

        self.client = IoTDAClient.new_builder() \
            .with_credentials(credentials) \
            .with_region(IoTDARegion.value_of(cfg['IoTARegion'])) \
            .build()

    def getDeviceInfo(self, device_id):
        """Device Info with specified ID"""
        try:
            request = ShowDeviceRequest()
            request.device_id = device_id
            return self.client.show_device(request)
        except exceptions.ClientRequestException as e:
            print(e.status_code)
            print(e.request_id)
            print(e.error_code)
            print(e.error_msg)
            return {}

    def getDeviceMessage(self, device_id):
        """Message device upload"""
        try:
            request = ShowDeviceShadowRequest()
            request.device_id = device_id
            return self.client.show_device_shadow(request)
        except exceptions.ClientRequestException as e:
            print(e.status_code)
            print(e.request_id)
            print(e.error_code)
            print(e.error_msg)
            return {}

    def sendDeviceCommand(self, dev_id, srv, cmd, par):
        try:
            request = CreateCommandRequest()
            request.device_id = dev_id
            request.body = DeviceCommandRequest(
                paras=par,
                command_name=cmd,
                service_id=srv,
            )
            response = self.client.create_command(request)
            print(response)
        except exceptions.ClientRequestException as e:
            print(e.status_code)
            print(e.request_id)
            print(e.error_code)
            print(e.error_msg)


class hw_device():

    def __init__(self, cfg):
        self.cfg = cfg
        self.hw = hw_iot(self.cfg)

    def getDeviceInfo(self):
        return self.hw.getDeviceInfo(self.cfg['device_id']).to_dict()

    def getDeviceMessage(self):
        return self.hw.getDeviceMessage(self.cfg['device_id']).to_dict()

    def beepON(self):
        self.hw.sendDeviceCommand(self.cfg['device_id'], self.cfg['service_id'], 
                "Smoke_Control_Beep", {"Beep":"ON"})

    def beepOFF(self):
        self.hw.sendDeviceCommand(self.cfg['device_id'], self.cfg['service_id'], 
                "Smoke_Control_Beep", {"Beep":"OFF"})

    def doorON(self):
        self.hw.sendDeviceCommand(self.cfg['device_id'], self.cfg['service_id'], 
                "Door_Control", {"Door": "ON"})

    def doorOFF(self):
        self.hw.sendDeviceCommand(self.cfg['device_id'], self.cfg['service_id'], 
                "Door_Control", {"Door": "OFF"})





if __name__ == "__main__":
    from config import config
    hw = hw_device(config)

    # print(type(hw.getDeviceInfo()))
    # print('----------------------------')
    # print(hw.getDeviceMessage())
    # print('----------------------------')
    # hw.doorON()
    # hw.doorOFF()
    # hw.beepON()
    hw.beepOFF()


    