# I2C Touchpad Driver for Haiku

This project aims to develop an I2C touchpad driver for the Haiku operating system, specifically targeting Huawei MateBook laptops.

## Project Status

This driver is currently in development and is not yet functional. It is being created as part of an effort to improve hardware support for Haiku on modern laptops.

## Features (Planned)

- I2C communication with the touchpad
- Basic touch input support
- Multi-touch gestures (future enhancement)
- Configurable settings (sensitivity, tap-to-click, etc.)

## Requirements

- Haiku operating system
- Development tools for Haiku
- A compatible I2C touchpad (initially targeting Huawei MateBook laptops)

## Building the Driver

1. Clone this repository:
   ```
   git clone https://github.com/yourusername/haiku-i2c-touchpad-driver.git
   cd haiku-i2c-touchpad-driver
   ```

2. Build the driver:
   ```
   make
   ```

3. Install the driver (requires root privileges):
   ```
   make install
   ```

## Usage

Once the driver is installed and functional, it should be automatically loaded by Haiku when a compatible touchpad is detected. You may need to restart your system or manually load the driver:

```
loaddriver /boot/home/config/non-packaged/add-ons/kernel/drivers/bin/i2c_touchpad
```

## Contributing

Contributions are welcome! If you'd like to help improve this driver, please fork the repository and submit a pull request with your changes.

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgements

- The Haiku development community for their documentation and support
- [Any other acknowledgements or credits]

## Contact

[Your Name] - [Your Email or GitHub profile]

Project Link: [https://github.com/atomozero/haiku-i2c-touchpad-driver](https://github.com/atomozero/haiku-i2c-touchpad-driver)
