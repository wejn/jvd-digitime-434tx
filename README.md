# Transmitter for "JVD Digi Clock" (434 MHz)

This repository hosts a transmitter that implements the "Philips-temperature"
protocol used by our digi clock + thermometer combo:

![JVD Digi Clock](https://wejn.org/assets/2021-digiclock-upgrade/jvd-digi-clock.jpg)

It is a rebranded "Electronics Tomorrow LTD." model no. 7904.
(according to the label on the back)

According to [rtl_433](https://github.com/merbanan/rtl_433/) this clock's
outdoor temperature sensor uses the
[philips_aj3650](https://github.com/merbanan/rtl_433/blob/master/src/devices/philips_aj3650.c)
protocol to transmit the temperature data.

The transmitter code here is targeted at an ESP32 chip augmented with a cheap
[434 MHz transmitter](https://www.digikey.com/en/products/detail/sparkfun-electronics/WRL-10534/5673761).


More information can be found in the [Putting an old digital clock (with an
outdoor thermometer) on steroids](https://wejn.org/2021/05/putting-old-temp-clock-on-steroids/)
article.

## Wiring

All you need to do is plug `Data in` pin of the 433 MHz transmitter to
the `GPIO_NUM_5` on the ESP32:

![wiring](https://wejn.org/assets/2021-digiclock-upgrade/wiring.jpg)

(the green wire is a very crude antenna)


## Building

This repository uses [platformio](https://platformio.org/), so all you need
to do is:

``` sh
pip3 install platformio
export INFRA_WIFI_SSID=my-wifi
export INFRA_WIFI_PASS=password
pio run
```

to get a firmware image.

## Credits

* Author: Michal Jirku (wejn.org)
* License: GPL v2, unless stated otherwise in the file
