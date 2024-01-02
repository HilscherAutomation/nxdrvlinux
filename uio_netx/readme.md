
## Overview uio_netx kernel module

The module provides the automatic mapping of Hilscher netX PCI device to user space, which then will be handled by the user space library libcifx.
It provides as well mapping of non-PCI devices by passing the device memory information.

Example for an ISA device at physical address 0xD0000 and DPM length of 0x4000 and irq line 5.

```
modprobe uio_netx custom_dpm_addr=0xD0000 custom_dpm_len=0x4000 custom_irq=5
```

Set to custom_irq to "0" if polling not connected or polling is to be used.
