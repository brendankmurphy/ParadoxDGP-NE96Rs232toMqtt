# ParadoxDGP-NE96Rs232toMqtt
Absolute hack job modification of Margelis' ParadoxRs232toMqtt project as an adaptation to work for Paradox DGP-NE96 alarm panels

Materials used:
  - NodeMCU ESP8266 board
  - 5V to 3.3V logic level converter

1) Use 5v power supply from alarm panel for Vin on ESP8266
2) Using Logic level converter to drop to safe 3.3v, wire RX/TX pins of ESP8266 to first two pins of Serial Port on the alarm panel.

Remaining set up follows the Margelis ParadoxRs232toMqtt project

For Home Assistant integration, I used the following configuration items:

```
binary_sensor:
  - platform: mqtt
    state_topic: "paradox/status"
    name: Pardox Status
    unique_id: paradoxstatus
    payload_on: online
    payload_off: offline
  - platform: mqtt
    state_topic: "paradox/ha/zone1"  # Use for each zone sensor, e.g. zone 1 - n
    name: Zone 1
    device_class: door

alarm_control_panel:
  - platform: mqtt
    name: Paradox
    state_topic: "paradox/ha/Arm"
    command_topic: "paradox/in"
    availability_topic: "paradox/status"
    payload_disarm: "{\"password\" :\"nnnn\", \"Command\":\"arm\", \"Subcommand\":\"60\"}" # Enter your panel password for nnnn
    payload_arm_home: "{\"password\":\"nnnn\", \"Command\":\"arm\", \"Subcommand\":\"30\"}"
    payload_arm_away: "{\"password\":\"nnnn\", \"Command\":\"arm\", \"Subcommand\":\"20\"}"
    payload_available: online
    payload_not_available: offline
    code_arm_required: false
    code_disarm_required: false
    retain: true
```
