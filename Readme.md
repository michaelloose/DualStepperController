Stepper Controller
==================

Protokoll
---------
- Aktuell hinterlegt in Projektphasendoku 1. Wird hier ergänzt

Konfiguration:
- CFG1.OFFS=42
- CFG1.STPU=1000

Zu erledigende Aufgaben
------------------------

- Software Not Aus Implementieren Befehl stoppt alle Bewegungen und deaktiviert den Enable Pin der Controller
- Positionsanzeige über I²C Display
- Steuerung über Hardware Joystick
- ID vom Gerät auf Anfrage ausgeben
- Konstanten in EEPROM hinterlegen und über Befehle einstellbar machen:
    - Anzahl Schritte pro Einheit
    - Offset vom Referenzschalter zu Nullposition