from PyQt5 import QtWidgets, uic
from Controller import singletonController

class DriveControlWidget(QtWidgets.QWidget):
    def __init__(self, parent = None):
        QtWidgets.QWidget.__init__(self, parent)

        uic.loadUi("DriveControlWidget.ui", self)

        #Antriebssteuerung
        #self.doubleSpinBoxLineardrivePosition.valueChanged.connect(self.controller.setLineardriveSpeed)
        #self.doubleSpinBoxLineardriveSpeed.valueChanged.connect(self.controller.setLineardriveSpeed)
        #self.doubleSpinBoxTurntablePosition.valueChanged.connect(self.controller.setTurntablePosition)
        #self.doubleSpinBoxTurntableSpeed.valueChanged.connect(self.controller.setTurntableSpeed)
        self.pushButtonMoveTurntable.clicked.connect(lambda: singletonController.moveTurntable(speed = self.doubleSpinBoxTurntableSpeed.value(), position = self.doubleSpinBoxTurntablePosition.value()))
        self.pushButtonMoveLineardrive.clicked.connect(lambda: singletonController.moveLineardrive(speed = self.doubleSpinBoxLineardriveSpeed.value(), position = self.doubleSpinBoxLineardrivePosition.value()))        
        self.pushButtonZeroTurntable.clicked.connect(singletonController.zeroTurntablePosition)
        self.pushButtonZeroLineardrive.clicked.connect(singletonController.zeroLineardrivePosition)
        self.pushButtonReference.clicked.connect(singletonController.referenceDrives)


    def zeroValuesDisplay(self):
        if singletonController.MotorController.isConnected:
            self.lcdTurntablePosition.display("%.2f" % 0)
            self.lcdLineardrivePosition.display("%.2f" % 0)


    def refreshValuesDisplay(self):
        if singletonController.MotorController.isConnected:
            self.lcdTurntablePosition.display("%.2f" % (singletonController.MotorController.getPosition("t", interpolate=False)))
            self.lcdLineardrivePosition.display("%.2f" % (singletonController.MotorController.getPosition("l", interpolate=False)))