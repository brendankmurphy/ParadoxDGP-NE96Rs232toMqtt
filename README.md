# ParadoxDGP-NE96Rs232toMqtt
Modification of Margelis ParadoxRs232toMqtt project adapted to work for Paradox DGP-NE96 alarm panels

Materials used:
  - NodeMCU ESP8266 board
  - 5V to 3.3V logic level converter

1) Use 5v power supply from alarm panel for Vin on ESP8266
2) Using Logic level converter to drop to safe 3.3v, wire RX/TX pins of ESP8266 to first two pins of Serial Port on the alarm panel.

Remaining set up follows the Margelis ParadoxRs232toMqtt project
