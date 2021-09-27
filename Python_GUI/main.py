#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""
GUI zur einfachen Ansteuerung des DualStepperControllers. Angepasst für die Objektpositionierung in den Radar-Messkammern der FHWS
"""

__author__ = "Michael Loose"
__copyright__ = "FHWS, Labor für Mikrowellentechnik"
__license__ = "Unset"
__version__ = "0.0.1"
__maintainer__ = "Michael Loose"
__email__ = "michael.loose@student.fhws.de"
__status__ = "Prototype"



# Built-in/Generic Imports
# import sys
import os
import sys

#Arbeitsordner auf Pfad des Programmes setzen
abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

# Libs
from PyQt5 import QtCore, QtGui, QtWidgets, uic
import queue

# Own modules

#from MyThreading import Worker
from Controller import singletonController

#from Plotting import myPolarPlotWidget


#MainWindow Klasse aus der vom QT Designer erzeugten Datei importieren, damit daran Ergänzungen vorgenommen werden können
class Ui_MainWindow(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        uic.loadUi("MainWindow.ui", self)
        self.actionEmergency_Stop.triggered.connect(self.EStop)
        self.actionEnable_Controls.triggered.connect(self.EStopReset)
        self.comPortActionElements = []
        self.scanCOMPorts()

    def showError(self, title="Fehler", message=None):
        # message = kwargs.get("msg", None)
        msg = QtWidgets.QMessageBox()
        msg.setIcon(QtWidgets.QMessageBox.Warning)
        msg.setWindowTitle(title)
        msg.setText(message)
        msg.setStandardButtons(QtWidgets.QMessageBox.Retry | QtWidgets.QMessageBox.Close)
        # msg.buttonClicked.connect(self.msgbtn)

        return msg.exec_()
        # retval = msg.exec_()
        # print("return: "+str(retval))
            

    def showWarning(self, title="Warnung", message=None):
        # message = kwargs.get("msg", None)
        msg = QtWidgets.QMessageBox()
        msg.setIcon(QtWidgets.QMessageBox.Warning)
        msg.setWindowTitle(title)
        msg.setText(message)
        msg.setStandardButtons(QtWidgets.QMessageBox.Ok)
        # msg.buttonClicked.connect(self.msgbtn)

        return msg.exec_()
        # retval = msg.exec_()
        # print("return: "+str(retval))

    def scanCOMPorts(self):
        #Verfügbare Com Ports auflisten und ins Auswahlmenü packen
        ports = singletonController.scanSerialDevices()
        self.menuCOM_Port.clear()
        group = QtWidgets.QActionGroup(self.menuCOM_Port)
        group.setExclusive(True)
        group.triggered.connect(self.connectMotorController)

        self.comPortActionElements = []

        for port in ports:
            self.comPortActionElements.append(QtWidgets.QAction(port, self.menuCOM_Port, checkable=True, checked = False))
            self.menuCOM_Port.addAction(self.comPortActionElements[-1])
            group.addAction(self.comPortActionElements[-1])
        
        self.menuCOM_Port.addSeparator()
        self.actionRefresh = QtWidgets.QAction("Refresh", self.menuCOM_Port, checkable=False)
        self.actionRefresh.triggered.connect(self.scanCOMPorts)
        self.menuCOM_Port.addAction(self.actionRefresh)

        if singletonController.MotorController.isConnected:
            for act in self.comPortActionElements:
                if singletonController.MotorController.serPort == act.text():
                    act.setChecked(True)

    
    @QtCore.pyqtSlot(QtWidgets.QAction)
    def connectMotorController(self, action):
        self.DriveControlWidget.zeroValuesDisplay()

        print('Verbinde: ', action.text())

        try:
            if singletonController.MotorController.isConnected:
                #Verbundener Port wurde ausgewählt: Trennen
                singletonController.disconnectMotorController() 
                if singletonController.MotorController.serPort == action.text():
                    for act in self.comPortActionElements:
                        act.setChecked(False)
                
                #Anderer Port wurde ausgewählt: Trennen und anderen Verbinden
                else:
                    singletonController.disconnectMotorController() 
                    singletonController.connectMotorController(action.text())
                    for act in self.comPortActionElements:
                        if singletonController.MotorController.serPort == act.text():
                            act.setChecked(True)
                        else:
                            act.setChecked(False)


            else:
                #Alle anderen Deaktivieren
                for act in self.comPortActionElements:
                    act.setChecked(False)

                action.setChecked(True)
                singletonController.connectMotorController(action.text())
        except Exception as err:
            self.showError("Verbindungsfehler", "{0}".format(err))
            #self.showError("Verbindungsfehler", "Am Seriellen Port wurde kein unterstützter Motorcontroller gefunden")

    def EStop(self):
        singletonController.EStop()

    def EStopReset(self):
        singletonController.EStopReset()
        
    def setupTimer(self):

        self.timer = QtCore.QTimer()
        self.timer.setInterval(100)
        self.timer.timeout.connect(self.timerFunction)
        self.timer.start()
        
        # Execute
       

    def timerFunction(self):
        try:
            retStr = singletonController.bucket.get(block=False)
            self.showWarning(message = retStr)
            self.scanCOMPorts()
        except queue.Empty:
            pass

        self.DriveControlWidget.refreshValuesDisplay()



if __name__ == '__main__':

    #import sys
    print (sys.version)


    os.environ["QT_AUTO_SCREEN_SCALE_FACTOR"] = "2"
    app = QtWidgets.QApplication(sys.argv)
    ui = Ui_MainWindow()
    #ui.setupPowerSensors()
    ui.setupTimer()

    ui.show()

    app.exec_()
    
    singletonController.cleanUp()
    sys.exit()