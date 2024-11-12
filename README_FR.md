# CG scale

Le micrologiciel CG Scale est open source et disponible gratuitement. Si vous l'aimez, soutenez le projet par un don et aidez-le à continuer à se développer.

[![Donate](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/Paypal.png)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=R69PMKTCXQBUU&source=url)

Balance basée sur Arduino pour peser des modèles réduits d'avions. Relativement peu de composants sont nécessaires et devraient être faciles à recréer, même pour les débutants en électronique.
Les fonctions les plus importantes :

- prend en charge les balances avec 2 ou 3 cellules de pesée
- prend en charge ESP8266 (également Wifi Kit 8) et Arduino avec ATmega328, ATmega32u4
- Calibrage automatique basé sur un objet de référence, ce qui signifie qu'il n'y a pas de détermination laborieuse des valeurs de calibrage
- Affichage via écran OLED
- La tension de la batterie peut être mesurée
- Les réglages s'effectuent via un menu via l'interface série
- Avec l'ESP8266, les réglages peuvent également être effectués facilement via la page Web
- Les paramètres sont enregistrés de manière permanente dans l'EEprom et ne doivent pas être reparamétrés après une mise à jour du logiciel
- Seulement quelques composants requis, ce qui rend l'assemblage rapide et facile

De plus amples informations sur l'assemblage peuvent être trouvées dans le [Wiki](https://github.com/ZINKTiti/CG_scale-1/wiki)

![init](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgScale_init.jpeg)
![run](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgScale.jpeg)
![cgscale_home](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgscale_home.png)
![cgscale_models](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgscale_models.png)
![cgscale_settings](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/cgscale_settings.png)
![cgscale_models](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/CG_Scale_wood.jpeg)
![cgscale_models](https://github.com/nightflyer88/CG_scale/blob/master/Doc/img/CG_Scale_ASW20.jpeg)
