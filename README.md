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

### Aufbau

Je nach gewünschtem Messbereich können die Wiegezellen nach belieben dimensioniert werden, solche gibt es von 1-200Kg. Die Zellen dürfen unterschiedlich gross sein. Die Messwandler der Wiegezellen müssen einen HX711 Chip enthalten. Als CPU wird grundsätzlich ein Arduino mit Atmega32u4 oder Atmega328 Prozessor empfohlen, andere Arduinos sollten jedoch auch funktionieren.

Materialliste:


Schema:


Mechanische Anordnung:


### Firmware laden

Um die Firmware auf das Arduino Board zu laden, wird die Arduino IDE benötig. Nachdem die Arduino IDE auf dem Computer installiert wurde, müssen noch zwei Bibliotheken installiert werden. Unter Sketch > Bibliotheken einbinden > Bibliotheken verwalten... gelangt man in den Bibliotheksverwalter. Nun nach "HX711" suchen und die lib von Olav Kallhovd installieren:

Danach nach "u8g2" suchen, und die lib von Oliver installieren:

Nun das Arduino Board per USB am Computer anschliessen, und unter Werkzeuge das entsprechende Board sowie Port auswählen. Wenn die Datei "CG_scale.ino" in der Arduino IDE bereits geöffnet wurde, kann nun oben links (Pfeil) die Firmware hochgeladen werden. Wurde der Uploadvorgang ohne Fehlermeldung abgeschlossen, hat mans schon geschafft !

### Parameter Einstellungen

Die Parameter werden per serieller Schnittstellen parametriert. Das Arduino Board per USB am Computer anschliessen, und unter Werkzeuge das entsprechende Board sowie Port auswählen. Danach den Seriellen Monitor öffnen, unter Werkzeuge > Serieller Monitor. Es erscheint ein simples Text Menü. Man gibt die Menünummer ein (mit Enter bestätigen), und es erscheint der entsprechende Wert den man ändern möchte.

Zuerst müssen die mechanischen Dimensionen definiert werden, also Menü 1-4. Sind die Messwiederstände vorhanden, kann die Batteriespannung aktiviert werden mit Menü 11.  


### Kalibrierung

Zur Kalibrierung gibt es zwei Möglichkeiten:

1. Manuell
Man kalibriert anhand eines Referenzgewichts jede einzelne Wiegezelle. Das Gewicht auf eine Wiegezelle legen und den entsprechenden Kalibrierfaktor anpassen (Menü 8-10) bis der angezeigte Wert mit dem Referenzgewicht übereinstimmt. Die Werte der einzelnen Wiegezellen werden unter Menü 12 angezeigt. 

2. Automatisch
Anhand eines Referenzobjekts. Zum Beispiel ein Modell, von dem das genaue Gewicht und der genaue Schwerpunkt bekannt ist. Alternativ kann auch ein Brett mit einem Gewicht verwendet werden. Das Brett sollte symmetrisch sein und das Gewicht mittig auf dem Brett positioniert sein. Ist das Gewicht und der Schwerpunkt von unserem Referenzobjekt bekannt, müssen diese Parameter in Menü 5 und 6 eingegeben werden. Nun das Referenzobjekt auf die Waage legen, und unter Menü 7 den Kalibriervorgang starten. 
