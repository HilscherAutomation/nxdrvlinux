
## Linux cifX plugin interface

The libcifx plugins allow an interface independant access to netX devices. It is based on the cifX toolkit HW function interface.

Currently a SPI sample interface is provided, which is able to work with Linux SPI devices.

The easiest way is to build the driver with SPI plugin support enabled (manual way -> build option: SPM_PLUGIN).

The driver's installation process will automatically create a plugin directory in the driver's configuration path (default: "/opt/cifx/plugins").

After the installation adapt the SPI plugin configuration (/opt/cifx/plugins/netx-spm/config0) according to your needs and run the application to access a SPI device.

| parameter      |               |
| -------------- |:-------------:|
| Device         | Name of the SPI device to open (e.g. /dev/spidev0.0 -> Device=spidev0.0)
| Speed          | Maximum speed to configure the driver (e.g. 25Mhz -> Speed=25000000)
| Mode           | SPI mode 0 to 3
| ChunkSize      | Chunk size (maximum size of transfer after which a new transfer will be automatically setup in bytes). If set to 0, no transfer splitting will be executed. e.g. Split transfers in case it is larger than 250 byte => ChunkSize=250
| Irq            | Path to irq file, e.g. /sys/class/gpio/gpio1/value

## IRQ configuration

If using GPIO-based IRQ, only edge may be available while the netX works level oriented.
Therefore, the driver internal compensates this but requires the following GPIO settings:
```
direction=in
edge=rising
active_low=1
```