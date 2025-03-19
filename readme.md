# Overview

This repository provides the following components:

| component                                                | license       | description
| ----------------------------------------                 |:-------------:|:-------------:|
| [libcifx](libcifx/)                                      | [MIT](libcifx/LICENSE.md)                | User space driver for cifX/netX devices ([more information](https://hilscher.atlassian.net/l/cp/JFCHnF6h)).
|  - [cifX toolkit](libcifx/Toolkit/)                      | [SLA](libcifx/Toolkit/LICENSE.md)        | OS independant device handling abstraction of netX devices. It is referenced by the user space library ([more information](https://hilscher.atlassian.net/l/cp/bBwTb2Wc)).
| [uio_netx](uio_netx/)                                    | [GPLv-2-only](uio_netx/LICENSE.txt)      | kernel mode driver (required for memory mapped devices)
| [ax99100](cifx_m2/)                                      | [GPLv-2-only](cifx_m2/LICENSE.txt)       | kernel mode driver (requried for cifX M.2 devices)
| [cifX examples](examples/)                               | [MIT](LICENSE.md)                        | cifX driver example applications (API, TCP server).
|  - [marshaller toolkit](examples/tcpserver/Marshaller/)  | in clarification                         | OS independant implementation of the netXtransport protocol device(/server) side. It is referenced by the TCP server ([more information](https://hilscher.atlassian.net/l/cp/e4W3zr1Y)).


# Driver architecture

![](doc/drv_overview.png)

The Linux cifX driver provides support of multiple devices of the [cifX product portfolio](https://www.hilscher.com/products/pc-cards-for-industrial-ethernet-fieldbus).
The driver consists of a user space and a kernel space component. The follwing table gives an overview which components are required depending on the hardware.

| hardware                       | kernel driver                          | user space driver (compile option (1)) |
| ------------------------------ |:--------------------------------------:|:--------------------------------------:|
| PCI based host interface       | uio_netx or vfio-pci (2)               | libcifx (VFIO if vfio-pci)             |
| SPI based host interface       | spidev                                 | libcifx (SPM_PLUGIN)                   |
| ISA or other memory mapped     | optional: uio_netx                     | libcifx                                |
| cifX M.2 device                | ax99100                                | libcifx (SPM_PLUGIN)                   |

Note that this documentation currently provides only a short overview. It will be updated step by step. Transitionally refer to the [superseded driver's documentation](doc/OBSOLETE-cifX-Device-Driver-Linux-DRV-15-EN.pdf). Commands mentioned there
may not work 1:1 since the driver's folder structure changed but it provides background information and still some valid hints.

For SPI support use the driver's SPM plugin. It provides an easy integration for SPI devices. The plugin need to be enabled during build, since it's disabled by default.

(1) [Compile options in detail](#Compile-options-libcifx-userspace-library)<br>
(2) [The difference between uio_netx and vfio-pci](#PCI-host-interface)

<br>

# Requirements

 - CMake (min 2.8.12)
 - libpthread, librt
 - optional: Linux kernel header (only required when building the kernel module uio_netx)
 - optional: libpciaccess-dev (only required when uio based PCI devices are accessed, more info [PCI host interface](#PCI-host-interface))
 - optional: libnl-3/libnl-cli-3 (only required when VIRTETH is enabled)

```
sudo apt-get install cmake
```
```
sudo apt-get install linux-headers-$(uname -r)
```
```
sudo apt-get install libpciaccess-dev
```
```
sudo apt-get install libnl-3-dev libnl-cli-3-dev
```

<br>

# Simple driver installation in one step

You can run the driver installation by simply executing the script 'build_and_install_driver'.

Enter the directory containing the script 'build_and_install_driver' execute it and follow the instructions (root during installation requested).

<b>NOTE: During installation depending on your configuration the script will automatically switch the driver assignment of ALL found Hilscher PCI devices (uio_netx/vfio-pci).
To prevent this run the [manual driver installation](#Manual-driver-installation) and the manual assignment.</b>

```
./build_install_driver
```

In case a more advanced setup is required or any installation trouble run the [setup step by step](#Manual-driver-installation).

<br>

# <a id="Manual-driver-installation"></a>Manual driver installation

## Build of the user space library libcifx

### <a id="Compile-options-libcifx-userspace-library"></a>Compile options libcifx userspace library

| parameter                      | description   |
| ------------------------------ |:-------------:|
| DEBUG                          | Build with debug messages enabled.
| DISABLE_LIB_PCIACCESS          | Disables link to libciaccess. Note that only VFIO PCI devices can than be accessed in this case.
| DMA                            | Enables DMA support.
| HWIF                           | Enables support for custom hardware interface.
| NO_MINSLEEP                    | Disables minimum sleep time. If “on” the driver may “wait active” (no call to pthread_yield()).
| SPM_PLUGIN                     | Enables support for SPI devices (spidev framework).
| TIME                           | Enables toolkit function, setting the device time during device start-up.
| VIRTETH                        | Enables support for the netX based virtual Ethernet interface. Note: This feature requires dedicated hardware and firmware.
| SHARED                         | Switch between shared and static library.
| VFIO                           | Enable support for VFIO devices (DMA support if IOMMU is enabled with translation).
| VFIO_FORCE_LEGACY              | Enable in case iommufd (cdev interface) is not supported by the target kernel (<6.2.).

### Build and install the library

1. create a build folder and enter it
```
mkdir drv_build; cd drv_build
```
2. Prepare the build environment via cmake call and pass the path to the driver's lists file (CMakelists.txt within libcifx folder).
Run the preparation with your required options e.g. enable debug messages:
```
cmake ../ -DDEBUG=ON
```

3. build and install the driver
```
make; sudo make install
```

<br>

# <a id="System-and-hardware-setup"></a>System and hardware setup

## <a id="System-configuration"></a>System configuration

### <a id="PCI-host-interface"></a>PCI host interface
A PCI device can be accessed via the kernel module uio_netx or vfio-pci. Both drivers provide interrupt handling and mapping the device's memory to userspace which can then be accessed by the user space library libcifx. In contrast to the uio_netx module the vfio-pci provides IOMMU support.

This means if DMA is to be used and IOMMU is enabled (mode="translated") vfio-pci is required. Setting the IOMMU mode to "passthrough" allows further use of the uio_netx driver. The userspace driver can handle both at the same time for information refer to [uio_netx & vfio-pci parallel](#Running-uio_netx-and-vfio-pci-in-parallel).</b>

The following table gives an overview of performance impact and DMA support according to the system's abiltiy and configuration.

| hardware IOMMU (BIOS)   | IOMMU mode            | kernel module     | DMA  | access protection | max. performance in case of virtualization     |
|------------------------:|:---------------------:|:-----------------:|:----:|:-----------------:|:-----------------------------------------------|
| enabled                 | disabled              | uio / vfio-pci    | yes  | no                | -
| enabled                 | translated            | uio               | no   | -                 | -
| enabled                 | translated            | vfio-pci          | yes  | yes               | no
| enabled                 | passthrough           | uio               | yes  | yes               | yes
| enabled                 | passthrough           | vfio-pci          | yes  | yes               | yes

The IOMMU mode can be switched by kernel [commandline parameter](https://docs.kernel.org/admin-guide/kernel-parameters.html) ('iommu', 'intel_iommu', ...).

On kernel >=5.11 the IOMMU mode can be changed per IOMMU group during runtime via "/sys/kernel/iommu_groups/{group}/type" ([more info here](https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-kernel-iommu_groups)).

For kernel version <6.2. iommufd (cdev interface) is not supported. In this case set the libcifx compile option 'VFIO_FORCE_LEGACY'.

<br>

#### <a id="VFIO-Driver"></a>VFIO Driver / IOMMU support
The vfio-pci driver is a generic driver for PCI devices. To motivate the driver to take control over a specific PCI device, the device need to be bind to the driver.
For background information about VFIO framework refer to the [kernel's VFIO API documentation](https://docs.kernel.org/driver-api/vfio.html).

<br>

<b>NOTE: Depending on your hardware it might be necessary to assign more than only the device you are interrested in. This depends on the device's IOMMU group created by the system. All devices within one IOMMU group need to be controlled by the vfio-pci driver.
For more information about the relation of devices and groups and how to access unprivileged refer to [kernel's VFIO API documentation](https://docs.kernel.org/driver-api/vfio.html).</b>

<br>

If case the module is not already loaded you can load it and pass an array of ids
```
sudo modprobe vfio-pci:[pci-id]
```
If the module is already loaded you can (de-)register devices PCI device id, e.g.:

```
echo 15cf 0000 | sudo tee /sys/bus/pci/drivers/vfio-pci/new_id
echo 15cf 0000 | sudo tee /sys/bus/pci/drivers/vfio-pci/remove_id
```

If the binding was successful and the libcifx is compiled with 'VFIO' option, the device will be automatically detected by the user space driver libcifx.

<br>To bind/unbind a particular device run
```
echo 0000:04:00.0 |sudo tee /sys/bus/pci/drivers/vfio-pci/bind
echo 0000:04:00.0 |sudo tee /sys/bus/pci/drivers/vfio-pci/unbind
```

<br>

#### <a id="UIO-Driver"></a>UIO Driver (no IOMMU support)
The uio_netx kernel module provides access to PCI, ISA or other memory mapped devices. In case vfio-pci is used as well it is recommended to disable PCI support via DISABLE_PCI_SUPPORT (see [Running uio_netx and vfio-pci in parallel](#Running-uio_netx-and-vfio-pci-in-parallel)).

To be able to access uio_netx based PCI devices as an unprivileged user use the provided [udev rule](templates/udev/80-udev-netx.rules).

##### Compile options uio_netx

| parameter                      | description                                                                                         |
|:------------------------------:|:---------------------------------------------------------------------------------------------------:|
| DISABLE_PCI_SUPPORT            | Disables PCI support (should be used if PCI devices need to be accessed via vfio-pci kernel module) |
| DISABLE_DMA                    | Disables DMA (need to be set if DISABLE_PCI_SUPPORT)                                                |

##### Build / install the uio_netx module

1. Enter the module's source folder uio_netx and run make.

```
# default DMA enabled and take control over all found Hilscher PCI devices
make
# or for exmaple DMA support disabled
make DISABLE_DMA=1
```

2. Then install the module according to your system's setup and update the module dependencies e.g:

```
# installs the module
make modules install
# or
sudo cp uio_netx.ko /lib/modules/$(uname -r)/kernel/drivers/uio/
sudo depmod
```

3. (un-)load the module

After successfully loading the module the device will be automatically detected by the user space driver libcifx.

```
# loads the module
sudo modprobe uio_netx
# unloads the module
sudo modprobe -r uio_netx
```

<br>

#### <a id="Running-uio_netx-and-vfio-pci-in-parallel"></a>Running uio_netx and vfio-pci in parallel
<b>NOTE: In generell it is not recommended to run both in parallel. Except the uio_netx need to be active to provide access to other devices, like ISA or other memory mapped devices. In this case the uio_netx and vfio-pci need to be used in parallel.

The device assignment noted under [VFIO Driver](#VFIO-Driver) will only work if the device is not already under uio_netx control. The same applies to the uio_netx module. The device access will only work if not already under vfio-pci control.

To avoid these device access conflicts between the drivers it is recommended to disable PCI support ('DISABLE_PCI_SUPPORT') of the uio_netx driver.

<br>

### SPI host interface
To be able to access a SPI device make sure to allow read/write access to the SPI interface (e.g. /dev/spidev0.0).
The SPI plugin default configuration is stored under "/opt/cifx/plugins/netx-spm/config0". Make sure to update the configuration as required.

| parameter      | description   |
| -------------- |:-------------:|
| Device         | Name of the SPI device to open (e.g. /dev/spidev0.0 -> Device=spidev0.0)
| Speed          | Maximum speed to configure the driver (e.g. 25Mhz -> Speed=25000000)
| Mode           | SPI mode 0 to 3

<br>

## Hardware configuration
Depending on the hardware we need to prepare the host system to be able to access the hardware. Flash based devices may not require any system setup to run a simple demo application.
In contrast to that a RAM based device like the common cifX PCIe hardware, the driver need to download at least a second stage loader to run a simple test application. 
For more advanced API tests a firmware and dedicated firmware configuration file is required.

The driver uses a simple folder structure to store the bootloader and firmware for each hardware. The driver supports four different methods of how to identify the devices and handling their firmware and configuration assignments.
In the following the most simple (at the expense of the support of multiple devices at a time) called "single directory" is explained.

![](doc/drv_config.png)

This folder structure can be created by using the provided script. The script also installs the required boot loader.
Enter the script folder and install the boot loader and create the required folder structure.
```
./install_firmware install
./install_firmware create_single_dir
```

The script should have now created the base directory "/opt/cifx/" with the subfolders "deviceconfig", "deviceconfig/FW" and "deviceconfig/FW/channel0". Further it should have installed the boot loader within the base directory.

When running now an example application a small set of tests will work. For an advanced test copy the correct firmware and it's configuration into the 'channel0' folder within the configuration directory.

<br>

# Build of the provided example applications
1. create a build folder and enter it
```
mkdir demo_build; cd demo_build
```
2. Prepare the build environment via cmake call and pass the path to the examples lists (CMakelists.txt within examples folder) file.
Run the preparation with your required options e.g.:
```
cmake ../examples/
```
3. Build the examples. And run a demo application. Note that you may need root rights. This depends on your system setup. For more information see "System and hardware setup".
```
make
./cifx_api
#or
./cifx_tcpserver
```
