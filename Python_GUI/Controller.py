import serial
from MotorController import MotorController
import queue

from PyQt5 import QtCore, QtGui, QtWidgets, uic
#from MyThreading import Worker
from time import sleep
from functools import partial

class Controller:
    def __init__(self):
        self.threadpool = QtCore.QThreadPool()
        self.MotorController = MotorController(callbackException=self.mcExceptionHandler)
        self.bucket = queue.Queue()
        self.turntableSpeed = 3
        self.lineardriveSpeed = 50


    def cleanUp(self):
        self.MotorController.disconnect()

    def scanSerialDevices(self):
        ports = serial.tools.list_ports.comports(include_links=False)
        for port in ports:
            print('Find port ' + port.device)

        self.availableSerialDevices = [port.device for port in ports]
        return self.availableSerialDevices

    def connectMotorController(self, device):
        self.MotorController.connect(device)

    def disconnectMotorController(self):
        self.MotorController.disconnect()
        self.MotorController = MotorController(callbackException=self.mcExceptionHandler)

    def mcExceptionHandler(self, exception):
        print(exception)
        print(type(exception))
        if self.bucket.empty():
            if type(exception) == ConnectionRefusedError:
                self.bucket.put("Notaus ist Aktiv. Bitte zuerst freigeben.")
            if type(exception) == ConnectionResetError:
                self.bucket.put("Keine Hardware verbunden. Bitte Seriellen Port auswählen")
        
            if type(exception) == ConnectionAbortedError:   
                self.bucket.put("Unerwarteter Verbindungsabbruch des Motorcontrollers")
                self.disconnectMotorController()


    def EStop(self):
        self.MotorController.EStop()

    def EStopReset(self):
        self.MotorController.EStopReset()


    def moveTurntable(self, speed, position):
        self.turntableSpeed = speed
        self.MotorController.setSpeed('t', self.turntableSpeed)
        self.MotorController.setPosition('t', position)

    def moveLineardrive(self, speed, position):
        self.lineardriveSpeed = speed
        self.MotorController.setSpeed('l', self.lineardriveSpeed)
        self.MotorController.setPosition('l', position)

    def zeroTurntablePosition(self):
        self.MotorController.zeroPosition('t')

    def zeroLineardrivePosition(self):
        self.MotorController.zeroPosition('l')

    def referenceDrives(self):

        self.MotorController.setSpeed('t', 12)
        self.MotorController.setSpeed('l', 100)

        self.MotorController.reference('t')
        self.MotorController.reference('l')

singletonController = Controller()
