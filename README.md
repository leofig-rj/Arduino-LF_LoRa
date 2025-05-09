#  Arduino-LF_LoRa

![Project Stage][project-stage-shield]
![Maintenance][maintenance-shield]


This [Arduino][arduino] library is intended to create [LoRa][lora] devices that communicate with the [LoRa2MQTT][github_lora2mqtt], Home Assistant AddOn.

This uses the [LoRa][lora_lib] library to interface with the radios.

## Compatible Hardware

It is limited to ESP32 processors (all versions) because it uses specific resources of this platform.

## Examples

The examples can be used for initial experience with the device / LoRa2MQTT pair.

The example [LF_LoRa_USB_Adapter_01][ex_usb] is to flash the USB adapter to be connected to the Home Assistant server and allow connection to devices.

Each example contains a corresponding LoRa MQTT configuration file. This example .ino / .py file pair serves as a basis for developing new devices.

They are:

- [LF_LoRa_Model_TEST01.ino][ex_01_ino] / [test01.py][ex_01_py]

- [LF_LoRa_Model_TEST02.ino][ex_02_ino] / [test02.py][ex_02_py]

- [LF_LoRa_Model_TEST03.ino][ex_03_ino] / [test03.py][ex_03_py]

## New Devices

New devices can be developed based on the above examples.
The .py configuration file for LoRa2MQTT should be placed in the /Config/lora2mqtt/models folder of the Home Assistant server.

### Note

The configuration files for the examples are already included in LoRa2MQTT and the new ones should be placed in /Config/lora2mqtt/models.

## License

This libary is [licensed][license] under the [MIT Licence][mit].

<!-- Markdown link -->
[project-stage-shield]: https://img.shields.io/badge/project%20stage-development%20beta-red.svg
[maintenance-shield]: https://img.shields.io/maintenance/yes/2025.svg
[github_lora2mqtt]: https://github.com/leofig-rj/leofig-hass-addons
[github_leofig-rj]: https://github.com/leofig-rj
[arduino]:https://arduino.cc/
[lora]:https://www.lora-alliance.org/
[lora_lib]:https://github.com/sandeepmistry/arduino-LoRa
[ex_usb]:https://github.com/leofig-rj/Arduino-LF_LoRa/tree/main/examples/LF_LoRa_USB_Adapter_01
[ex_01_ino]:https://github.com/leofig-rj/Arduino-LF_LoRa/tree/main/examples/LF_LoRa_Model_TEST01
[ex_01_py]:https://github.com/leofig-rj/leofig-hass-addons/blob/main/lora2mqtt/rootfs/usr/bin/models/test01.py
[ex_02_ino]:https://github.com/leofig-rj/Arduino-LF_LoRa/tree/main/examples/LF_LoRa_Model_TEST02
[ex_02_py]:https://github.com/leofig-rj/leofig-hass-addons/blob/main/lora2mqtt/rootfs/usr/bin/models/test02.py
[ex_03_ino]:https://github.com/leofig-rj/Arduino-LF_LoRa/tree/main/examples/LF_LoRa_Model_TEST03
[ex_03_py]:https://github.com/leofig-rj/leofig-hass-addons/blob/main/lora2mqtt/rootfs/usr/bin/models/test03.py
[license]:https://github.com/leofig-rj/leofig-hass-addons/blob/main/LICENSE
[mit]:https://en.wikipedia.org/wiki/MIT_License