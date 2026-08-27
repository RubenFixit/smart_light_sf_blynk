# Smart Chicken Coop Light

ESPHome migration of the original SparkFun Blynk Board chicken-coop light controller.

Project background: https://rubenfixit.com/posts/smart-chicken-coop-light/

## ESPHome

The initial ESPHome configuration is in [`chicken-coop-light.yaml`](chicken-coop-light.yaml).

It currently maps the original hardware as follows:

- GPIO5: PWM output to the MOSFET controlling the coop light
- GPIO2 / GPIO14: onboard I2C bus
- Si7021: onboard temperature and humidity sensor
- A0: external TMP36 temperature sensor

Copy `secrets.example.yaml` to `secrets.yaml` and fill in the local Wi-Fi credentials before compiling or flashing.

```sh
cp secrets.example.yaml secrets.yaml
esphome run chicken-coop-light.yaml
```

Scheduling and automatic/manual mode are intentionally not implemented on-device. Those behaviors are expected to move to Home Assistant automations, while ESPHome exposes the light and sensors as native entities.

## Legacy Blynk firmware

The original Blynk firmware is retained in [`smart_light_sf_blynk.ino`](smart_light_sf_blynk.ino) for reference during the migration. The QR-code image is also retained as a historical artifact.
