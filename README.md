# EspTelegramStat

Il Muro interattivo di Fablab Torino, costruito dall'Arduino User Group di Torino, prevede la visualizzazione in tempo reale dei messaggi che le varie community del Fablab si scambiano attraverso la piattaforma Telegram. Ad animare il tutto un bot che si chiama @comarebot

Il bot gira su una Wemos D1 mini (esp8266) che si collega alle API di Telegram e conta i messaggi dei gruppi inseriti nel codice. Perchè il bot possa leggere i messaggi bisogna includerlo nelle chat e disabilitare la group privacy dal BotFather. 

Per mostrare le statistiche sui gruppi il bot utilizza due modalità: una grafica usando delle "stelle" di neopixel che si illuminano ogni volta che un messaggio viene inviato ad un gruppo e tramite i comandi di telegram. Basta chiedere /status@comarebot su una delle chat per ottenere un breve riassunto delle statistiche.

Per compilare sulla scheda Wemos D1 mini (driver usb https://wiki.wemos.cc/downloads) avrete bisogno del core esp8266 per Arduino: https://github.com/esp8266/Arduino La versione usata è la 2.2.0, le istruzioni per l'installazione le trovate nel repository sopra.

Il collegamento a Telegram è gestito da https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot e https://github.com/bblanchon/ArduinoJson : installatele dal gestore delle librerie interno dell'ide Arduino

I led vengono controllati con la libreria AdafruitNeoPixel https://github.com/adafruit/Adafruit_NeoPixel : la trovate nel gestore delle librerie di Arduino
