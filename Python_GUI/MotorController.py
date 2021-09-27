import threading 
import serial
import serial.tools.list_ports

import datetime
import serial
import sys

class MotorController(threading.Thread):
    def __init__(self, callbackException = None):
        self.callbackException = callbackException
        super().__init__()
        self.isConnected = False
        self.buf = bytes()
        self.serPort = None
        self._ser = None

        self.error = None


        #Liste aus zu sendenden Befehlen, wird als Ringspeicher gesehen
        self.commandsToSend = []

        #Liste aus erwarteten Quittierungen
        #Erwartete Antwort, Bit ob Wert ausgelesen werden soll,  Callback Funktion
        #Erwartete Empfangsbestätigungen
        
        #self.commandConfirmationsRecieved = [["radar motor controller v1.0", False,  None]]
        self.commandConfirmationsRecieved = []
        #Erwartete Ausführungsbestätigungen. Reihenfolge ist egal.
        self.commandConfirmationsExecuted = []

        #Eingagangene Befehle
        self.incomingCommands = []

        #Flag, welche anzeigt ob der letzte Befehl vom Controller empfangen wurde
        self.clearToSend = True

        #Flags für den Bewegungszustand
        self.lineardriveIsMoving = False
        self.turntableIsMoving = False

        #Zwischenspeicher für den letzten empfangenen Wert. Wird nach dem Auslesen auf None gesetzt, damit erkannt wird ob ein neuer Wert vorhanden ist. 
        self.returnValue = None

        #Flags welche aussagen ob eine Positionsanfrage aussteht
        self.turntablePositionQueryPending = False
        self.lineardrivePositionQueryPending = False

        #Variablen in welche die aktuellen Antribspositionen gespeichert werden
        self.turntablePosition = 0
        self.lineardrivePosition = 0

        #Richtung der aktuellen Bewegung (Vorzeichen gibt Bewegungsrichtung an, 0 bedeutet keine Bewegung)
        self.turntableDirection = 0
        self.lineardriveDirection = 0

        #Geschwindigkeit der Antriebe
        self.turntableSpeed = 0
        self.lineardriveSpeed = 0
        #Zeitpunkt zu welchem die letzte Position vom Controller empfangen wurde (Für die lineare Interpolation)
        self.turntableLastPositionRecievedTime = None
        self.lineardriveLastPositionRecievedTime = None

        #Explizites Anfordern einer Positionsausgabe
        self.turntablePositionRequest = False
        self.lineardrivePositionRequest = False
        
        self.estActive = False

    def __del__(self):
        self.disconnect()
        
    def connect(self, port):

        #Für Linux: Entfernt das & Zeichen aus dem Pfad. Für Windows ist das unerheblich
        self.serPort = port.replace('&', '')
        self._ser = serial.Serial(self.serPort , 115200, timeout=5)
        if self._ser.is_open:
            self._ser.close()
        self._ser.open()
        self._ser.flushOutput()
        self._ser.flushInput()
        self.isConnected = True

        #Aktuelle Position einmalig abfragen
        self.sendSerialCommand("ap?", self.__processTurntablePosition)
        self.turntablePositionQueryPending = True
        self.sendSerialCommand("bp?", self.__processLineardrivePosition)
        self.lineardrivePositionQueryPending = True

        #Den Thread Starten
        self.start()

    def disconnect(self):
        #Nach Zurücksetzen dieses Flags Läuft der Thread aus
        self.serPort = None
        self.isConnected = False

    def sendSerialCommand(self, command, callbackRecieved = None):
        if self.isConnected:
            #Befehle nur weiterleiten wenn Notaus nicht aktiv
            if not self.estActive:
                command = command.lower()
                #print(command)
                self.commandsToSend.append(command)
                #Erwartete Antwort speichern
                #Erwartete Antwort, Ob ein Wert ausgelesen werden soll, Callback-Funktion
                self.commandConfirmationsRecieved.append([command[0:2]+"=", command[2] == "?",  callbackRecieved])

            else:
                self.callbackException(ConnectionRefusedError("Emergency Stop is Active"))

        else:
            self.callbackException(ConnectionResetError("Motor Controller is not Connected!"))

    
    #Main Loop vom Thread: Wird konstant ausgeführt
    def run(self):
        while self.isConnected:
            try:
                if not self.estActive:

                    #Anstehende Befehle lesen
                    while self._ser.in_waiting:
                        self.buf += self._ser.read()

                    #
                    if b'\r' in self.buf:
                        lines = self.buf.split(b'\r')
                        self.buf = lines[-1]  # Übrige Zeichen im Puffer wieder in den Puffer schreiben
                        for line in lines[:-1]:
                            
                            self.incomingCommands.append(line.strip().decode("ASCII").lower())
                            #Alle eingehenden Befehle ausgeben
                            #print(self.incomingCommands[-1])
                
                
                    if len(self.incomingCommands):
                        
                        
                        #Nach Rücksetzen des Not-Aus: Alles löschen
                        if ("esr" in self.incomingCommands):
                            self.incomingCommands = []
                            self.clearToSend = True

                        elif ("est" in self.incomingCommands):
                            print("E-Stop has been previously active")
                            self.incomingCommands.pop(0)
                            self.estActive = True
                            self.clearToSend = True

                        

                        #Überprüfen ob eine Ausführungsbestärigung(Ausrufezeichen) in den eingehenden Daten enthalten ist. Falls ja werden diese ausgewertet und unverzüglich entfernt
                        #Das ist nötig, weil sie prinzipiell zu beliebigen Zeitpunkten auftreten können und somit die Auswertung der Empfangsbestätigungen stören würde
                        incomingCommandsCopy = []
                        commandConfirmationsExecutedNames = [elem[0] for elem in self.commandConfirmationsExecuted]
                        for i, line in enumerate(self.incomingCommands):
                            
                            if line == "?":
                                pass

                            elif line[2] == "!":
                                #Flags setzen
                                if line == "ap!" or line == "th!":
                                    #Position ein letztes mal auslesen
                                    self.sendSerialCommand("ap?", self.__processTurntablePosition)
                                    self.turntablePositionQueryPending = True 
                                    #Bewegungsflag löschen
                                    self.turntableIsMoving = False
                                    self.turntableLastPositionRecievedTime = None

                                if line == "bp!" or line == "lh!":
                                    self.sendSerialCommand("bp?", self.__processLineardrivePosition)
                                    self.lineardrivePositionQueryPending = True
                                    self.lineardriveIsMoving = False           
                                    self.lineardriveLastPositionRecievedTime = None

                                
                                #Überprüfen bei den empfangenen Zeilen eine erwartete Zeile einer Callback-Funktion enthalten ist (Befehl erfolgreich ausgeführt)
                                #Wenn ja wird diese Zeile entfernt und die entsprechende Callback-Funktion getriggert
                                try:
                                    indexInConfirmationsExecutedNames = commandConfirmationsExecutedNames.index(line)
                                    #Element aus der Liste löschen und Callback Funktion ausführen
                                    self.commandConfirmationsExecuted.pop(indexInConfirmationsExecutedNames)[1]()
                                #Die Fälle abfangen, dass entweder die Ausführungsbestätigung in der Liste nicht gefunden wurde, oder keine Callback Funktion übergeben wurde.
                                #Gemäß dem Prinzip "Dont ask for Permission, Ask for Forgiveness"
                                except:
                                    pass
                            else:
                                incomingCommandsCopy.append(line)
                        self.incomingCommands = incomingCommandsCopy      



                        if len(self.commandConfirmationsRecieved) and len(self.incomingCommands):
                            #Ist die Antwort mit dem Erwarteten Wert identisch ODER handelt es sich um eine Anfrage nach einem Wert, welche in den ersten 2 Zeichen übereinstimmt?
                            """
                            isValidResponse = self.incomingCommands[0] == self.commandConfirmationsRecieved[0][0]
                            
                            #Bit ob Antwort erwartet wird ist gesetzt
                            if self.commandConfirmationsRecieved[0][1]:
                                #Nur die ersten 3 Zeichen vergleichen
                                isValidResponse =(self.incomingCommands[0][0:3] == self.commandConfirmationsRecieved[0][0][0:3])
                            if isValidResponse:
                            """
                            #Das ganze als Einzeiler verpackt
                            if (self.incomingCommands[0] == self.commandConfirmationsRecieved[0][0]) or (self.commandConfirmationsRecieved[0][1] and (self.incomingCommands[0][0:3] == self.commandConfirmationsRecieved[0][0][0:3])):
                            
                                #Soll ein Wert ausgelesen werden?
                                if self.commandConfirmationsRecieved[0][1]:
                                    valueString = self.incomingCommands[0][3:]
                                    #Callback-Funktion ausführen und Float übergeben. Falls keine Callback-Funktion vorhanden ist, soll das ignoriert werden
                                    try:
                                        self.commandConfirmationsRecieved[0][2](float(valueString))
                                    except:
                                        pass
                                #Callback ohne ausgelesenen Wert
                                else:
                                    try:
                                        self.commandConfirmationsRecieved[0][2]()
                                    except:
                                        pass

                                self.incomingCommands.pop(0)
                                self.commandConfirmationsRecieved.pop(0)
                                #Überprüfen ob der letzte Befehl erfolgreich quittiert wurde. Wenn dies der Fall was das clearToSend Flag setzen
                                self.clearToSend = True


                            elif (self.incomingCommands[0][0:3] == self.commandConfirmationsRecieved[0][0][0:2]+"?"):
                                print("Motor Controller recieved Illegal Argument! Command Ignored")
                                print(self.incomingCommands.pop(0))
                                print(self.commandConfirmationsRecieved.pop(0))
                                self.clearToSend = True
                            
                            else:
                                print(self.incomingCommands)
                                print(self.commandConfirmationsRecieved)
                                raise SystemError("Computer recieved mismatching Response from Motor Controller")
                                                    
                    #Position von aktuell bewegten Antrieben abfragen, sofern sich diese bewegen und keine aktuelle Anfrage mehr ausstehend
                    if (self.turntableIsMoving or self.turntablePositionRequest) and not self.turntablePositionQueryPending:
                        self.sendSerialCommand("ap?", self.__processTurntablePosition)
                        self.turntablePositionQueryPending = True
                        self.turntablePositionRequest = False

                    if (self.lineardriveIsMoving or self.lineardrivePositionRequest) and not self.lineardrivePositionQueryPending:
                        self.sendSerialCommand("bp?", self.__processLineardrivePosition)
                        self.lineardrivePositionQueryPending = True
                        self.lineardrivePositionRequest = False

                        
        

                    #Wenn Befehle zum senden anstehen, diese senden, sobald das clearToSend Flag gesetzt wurde
                    if len(self.commandsToSend) and self.clearToSend:
                        self.clearToSend = False
                        """
                        #Erwartete Antwort speichern
                        #Erwartete Antwort, Ob ein Wert ausgelesen werden soll, Callback-Funktion
                        self.commandConfirmationsRecieved.append([self.commandsToSend[0][0:2]+"=", self.commandsToSend[0][2] == "!",  None])
                        """
                        #Den Befehl der am untersten im Befehlspuffer steht senden.
                        self._ser.write(self.commandsToSend.pop(0).encode()+b'\r')
            except:
                self.isConnected = False
                #self.error = sys.exc_info()
                self.error = ConnectionAbortedError("Fehler in der Main-Loop. Wahrscheinlich Verbindungsabbruch")
        #Wenn die Schleife abgelaufen ist, also das Flag gelöscht wurde soll nach Ablauf die Verbindung getrennt werden.
        self._ser.close()
        if self.error is not None:
            self.callbackException(self.error)


    def reference(self, drive, callbackExecuted = None, callbackRecieved = None):
        if drive == "t":
            self.turntableIsMoving = True
            self.commandConfirmationsExecuted.append(["ar!", callbackExecuted])
            self.sendSerialCommand("ar=", callbackRecieved)
        if drive == "l":
            self.lineardriveIsMoving = True            
            self.commandConfirmationsExecuted.append(["br!", callbackExecuted])
            self.sendSerialCommand("br=", callbackRecieved)


    def setSpeed(self, drive, value, callbackRecieved = None):
        if drive == "t":
            self.sendSerialCommand("as=%.2f" % (value), callbackRecieved)
            self.turntableSpeed = value
        if drive == "l":
            self.sendSerialCommand("bs=%.2f" % (value), callbackRecieved)
            self.lineardriveSpeed = value


    def setPosition(self, drive, value, callbackExecuted = None, callbackRecieved = None):
        if drive == "t":
            self.turntableIsMoving = True
            self.commandConfirmationsExecuted.append(["ap!", callbackExecuted])
            self.sendSerialCommand("ap=%.2f" % (value), callbackRecieved)
        if drive == "l":
            self.lineardriveIsMoving = True
            self.commandConfirmationsExecuted.append(["bp!", callbackExecuted])
            self.sendSerialCommand("bp=%.2f" % (value), callbackRecieved)

    def EStop(self):
        self._ser.write("est".encode()+b'\r')
        self.estActive = True


    def EStopReset(self):
        if self.estActive:
            self._ser.write("esr".encode()+b'\r')
            self.commandsToSend = []
            self.commandConfirmationsRecieved = []
            self.commandConfirmationsExecuted = []
            self.turntablePositionQueryPending = False
            self.lineardrivePositionQueryPending = False
            self.estActive = False

    """                    
                            
    def setPositionAndBlockUntilDone(self, drive, value):
        if drive == "t":
            command = "ap=%.2f" % (value)
        if drive == "l":
            command = "bp=%.2f" % (value)


        self.sendSerialCommand(command)
        
        #Auf Empfangsbestätigung warten
        while True:
            inLine = self.readSerialLine()
            print(inLine)
            if(len(inLine)>2):
                if inLine[2] == '!':
                    break

        #Auf Bestätigung der erreichten Position warten
        while True:
            inLine = self.readSerialLine()
            print(inLine)
            if(len(inLine)>2):
                if inLine[2] == '=':
                    break
  
        return 0
    """
    def getSpeed(self, drive, callbackRecieved = None):
        if drive == "t":
            command = "as?"
        if drive == "l":
            command = "bs?"
        self.sendSerialCommand(command, callbackRecieved)


    def getPosition(self, drive, interpolate = False, callbackRecieved = None):
        if drive == "t":
            #command = "ap?"
            if interpolate and self.turntableIsMoving:
                if self.turntableLastPositionRecievedTime:
                    deltaT = datetime.datetime.now() - self.turntableLastPositionRecievedTime
                    interpolatedPosition = self.turntablePosition + (deltaT.microseconds * self.turntableSpeed * 1e-6 * self.turntableDirection)
                    #print(str(deltaT) + "|"+str(self.turntablePosition)+"|"+str(interpolatedPosition))
                    return interpolatedPosition
            else:
                return self.turntablePosition
        if drive == "l":
            #command = "bp?"
            return self.lineardrivePosition
    """
        #Wurde keine Callback Funktion übergeben soll die Funktion selbst den Wert zurückgeben
        if callbackRecieved != None:
            self.sendSerialCommand(command, callbackRecieved)
            return
        else:
            self.sendSerialCommand(command, self.__setReturnValue)
            #Warte auf Antwort
            #TBD Timeout einbauen
            while self.returnValue == None:
                pass
            #Wert wieder zurücksetzen
            retval = self.returnValue
            self.returnValue = None
            return retval
    """

    def zeroPosition(self, drive):
        if drive == "t":
            self.sendSerialCommand("az=")
            self.turntablePositionRequest = True

        if drive == "l":
            self.sendSerialCommand("bz=")
            self.lineardrivePositionRequest = True

        return 0

    def __setReturnValue(self, value):
        self.returnValue = value

    def __processTurntablePosition(self, value):
        self.turntableLastPositionRecievedTime = datetime.datetime.now()
        self.turntablePositionQueryPending = False
        #Bewegungsrichtung setzen
        if self.turntablePosition < value:
            self.turntableDirection = 1
        elif self.turntablePosition > value:
            self.turntableDirection = -1
        else:
            self.turntableDirection = 0
        
        self.turntablePosition = value

    def __processLineardrivePosition(self, value):
        #if self.lineardriveLastPositionRecievedTime is not None:
        #    print((datetime.datetime.now() - self.lineardriveLastPositionRecievedTime).total_seconds()*1000)
        self.lineardriveLastPositionRecievedTime = datetime.datetime.now()
        self.lineardrivePositionQueryPending = False

        #Bewegungsrichtung setzen
        if self.lineardrivePosition < value:
            self.lineardriveDirection = 1
        elif self.lineardrivePosition > value:
            self.lineardriveDirection = -1
        else:
            self.lineardriveDirection = 0
        self.lineardrivePosition = value

def printFcn(*args):
    print(args)


class MyTimer:
    def __init__(self):
        self.tStart = None
    def start(self):
        self.tStart = datetime.datetime.now()
    def printTimeDifference(self):
        deltaT = datetime.datetime.now()-self.tStart
        #print(deltaT)

if __name__ == '__main__':
    from time import sleep

    ports = serial.tools.list_ports.comports(include_links=False)
    for port in ports:
        print('Find port ' + str(port))

    dev = [port.device for port in ports]
    controller = MotorController()
    
    controller.connect(dev[0])
    sleep(1)
    print("GO")
    controller.setSpeed("l", 10)
    controller.setSpeed("t", 10)

    myTimer = MyTimer()
    #controller.setPosition("l", 100, lambda: print("Lineartrieb Verfahren"), lambda: print("Lineartrieb Bewegung gestartet"))
    #controller.setPosition("t", 100, lambda: print("Drehtisch Verfahren"), lambda: print("Drehtisch Bewegung gestartet"))
    controller.setPosition("l", 100, myTimer.printTimeDifference, myTimer.start)
    #controller.setPosition("t", 10, lambda: printTimeDifference(tStart), lambda: print("Drehtisch Bewegung gestartet"))
    """
    controller.getSpeed("t", printFcn)
    controller.getSpeed("l", printFcn)
    
    sleep(0.1)
    controller.getPosition("l", printFcn)
    sleep(0.1)
    controller.getPosition("l", printFcn)
    sleep(0.1)
    controller.getPosition("l", printFcn)
    sleep(0.1)
    print(controller.getPosition("l"))
    sleep(0.1)
    print(controller.getPosition("l"))
    sleep(0.1)
    print(controller.getPosition("l"))
    
    #rw.reference("l", lambda: print("Referenziert"))
    #rw.sendSerialCommand("bs=100")
    #rw.sendSerialCommand("LH=")
    """
    sleep(11)
    controller.setSpeed("t", 10)
    controller.setPosition("t", 0)
    controller.setSpeed("l", 100)
    controller.setPosition("l", 0)
    sleep(1)
    controller.disconnect()
