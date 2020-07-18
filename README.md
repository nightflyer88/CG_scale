# CG scale

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=R69PMKTCXQBUU&source=url)


Schwerpunktwaage auf Arduinobasis zum auswiegen von Flugmodellen. Es werden relativ wenige Bauteile benötigt, und sollte auch von Elektronikanfänger problemlos nachgebaut werden können.
Die wichtigsten Funktionen:

- unterstützt Waagen mit 2 oder 3 Wiegezellen
- unterstützt ESP8266 (auch Wifi Kit 8) und Arduino mit ATmega328, ATmega32u4
- automatische Kalibrierung anhand eines Referenzobjekts, dadurch kein mühsames eruieren der Kalibrierwerte
- Anzeige durch OLED Display
- Batteriespannnung kann gemessen werden
- Einstellungen werden durch ein Menü per serieller Schnittstelle vorgenommen 
- Beim ESP8266 können die Einstellungen auch bequem per Webpage vorgenommen werden
- Parameter werden dauerhaft im EEprom gespeichert, und müssen nach einem Softwareupdate nicht neu parametriert werden
- nur wenige Bauteile erforderlich, dadurch schnell und einfach aufgebaut

Weiter infos zum Aufbau findet man im [Wiki](https://github.com/nightflyer88/CG_scale/wiki)

![init](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgScale_init.jpeg)
![run](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgScale.jpeg)
![cgscale_home](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgscale_home.png)
![cgscale_models](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgscale_models.png)
![cgscale_settings](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgscale_settings.png)
