# CG scale

Schwerpunktwaage auf Arduinobasis zum auswiegen von Flugmodellen. Es werden relativ wenige Bauteile benötigt, und sollte auch von Elektronikanfänger problemlos nachgebaut werden können.
Die wichtigsten Funktionen:

  - unterstützt Waagen mit 2 oder 3 Wiegezellen
  - automatische Kalibrierung anhand eines Referenzobjekts, dadurch kein mühsames eruieren der Kalibrierwerte
  - Anzeige durch OLED Display
  - Batteriespannnung kann gemessen werden
  - Einstellungen werden durch ein Menü per serieller Schnittstelle vorgenommen 
  - Parameter werden dauerhaft im EEprom gespeichert, und müssen nach einem Softwareupdate nicht neu parametriert werden
  - nur wenige Bauteile erforderlich, dadurch schnell und einfach aufgebaut

### Übersicht

Anzeige auf dem OLED Display:
![init](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgScale_init.jpeg)
![run](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgScale.jpeg)

### Aufbau

Je nach gewünschtem Messbereich können die Wiegezellen nach belieben dimensioniert werden, solche gibt es von 1-200Kg. Die Zellen dürfen unterschiedlich gross sein. Die Messwandler der Wiegezellen müssen einen HX711 Chip enthalten. Als CPU wird grundsätzlich ein Arduino Micro mit Atmega32u4 empfohlen, andere Arduinos z.B. mit Atmega328 Prozessor sollten jedoch auch funktionieren.

Materialliste:

- 1x OLED Display 1.3" I2C
- 1x Arduino Micro Atmega32u4
- 3x Loadcell & HX711 board 


Schema:
![schema](https://raw.githubusercontent.com/nightflyer88/CG_scale/master/Doc/CG_scale_schema.png)

Mechanische Anordnung:
![mechanic](https://raw.githubusercontent.com/nightflyer88/CG_scale/master/Doc/CG_scale_mechanics.png)

### Firmware laden

Um die [Firmware](https://github.com/nightflyer88/CG_scale/archive/master.zip) auf das Arduino Board zu laden, wird die [Arduino IDE](https://www.arduino.cc/en/main/software) benötig. Nachdem die Arduino IDE auf dem Computer installiert wurde, müssen noch zwei Bibliotheken installiert werden. Unter Sketch > Bibliotheken einbinden > Bibliotheken verwalten... gelangt man in den Bibliotheksverwalter. Nun nach "HX711" suchen und die lib von Olav Kallhovd installieren:
![hx711](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/hx711_lib.png)

Danach nach "u8g2" suchen, und die lib von Oliver installieren:
![u8g2](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/u8g2_lib.png)

Nun das Arduino Board per USB am Computer anschliessen, und unter Werkzeuge das entsprechende Board sowie Port auswählen. Wenn die Datei "CG_scale.ino" in der Arduino IDE bereits geöffnet wurde, kann nun oben links (Pfeil) die Firmware hochgeladen werden. Wurde der Uploadvorgang ohne Fehlermeldung abgeschlossen, hat mans schon geschafft !

### Parameter Einstellungen

Die Parameter werden per serieller Schnittstellen parametriert. Das Arduino Board per USB am Computer anschliessen, und unter Werkzeuge das entsprechende Board sowie Port auswählen. Danach den Seriellen Monitor öffnen, unter Werkzeuge > Serieller Monitor. Es erscheint ein simples Text Menü. Man gibt die Menünummer ein (mit Enter bestätigen), und es erscheint der entsprechende Wert den man ändern möchte.

![menu](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/serial_menu.png)

Zuerst müssen die mechanischen Dimensionen definiert werden, also Menü 1-4. Die Batteriespannung wird in Menü 11 aktiviert. Alle Parameter werden im internen EEprom gespeichert und gehen auch nach einem Softwareupdatenicht verloren.


### Kalibrierung

Zur Kalibrierung gibt es zwei Möglichkeiten:

#### 1. Manuell

Man kalibriert anhand eines Referenzgewichts jede einzelne Wiegezelle. Das Gewicht auf eine Wiegezelle legen und den entsprechenden Kalibrierfaktor anpassen (Menü 8-10) bis der angezeigte Wert mit dem Referenzgewicht übereinstimmt. Die Werte der einzelnen Wiegezellen werden unter Menü 12 angezeigt. 

#### 2. Automatisch

Anhand eines Referenzobjekts. Zum Beispiel ein Modell, von dem das genaue Gewicht und der genaue Schwerpunkt bekannt ist. Alternativ kann auch ein Brett mit einem Gewicht verwendet werden. Das Brett sollte symmetrisch sein und das Gewicht mittig auf dem Brett positioniert sein. Ist das Gewicht und der Schwerpunkt von unserem Referenzobjekt bekannt, müssen diese Parameter in Menü 5 und 6 eingegeben werden. Nun das Referenzobjekt auf die Waage legen, und unter Menü 7 den Kalibriervorgang starten. 
