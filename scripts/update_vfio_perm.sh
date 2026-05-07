#!/bin/bash

if [ $# -lt 1 ]; then
        # check if forced
        script=$(basename $0)
        if [ "${script}" != "update_vfio_perm_force.sh" ]; then
                echo "Confirm script iteration over all found Hilscher PCI device (y)es/(n)o:"
                echo "Abort script to pass particular device/list."
                read r && [ "${r}" != "y" ] && exit
        fi

        echo "Trying to update IOMMU groups of all found Hilscher (vfio-pci) PCI devices!"
        IFS=$'\n'
        for dev in $(lspci -d 15cf::ff00 | cut -d " " -f 1); do
                lspci -s 0000:$dev -k | grep -oq "Kernel driver in use: vfio-pci" && dev_list="0000:${dev} ${dev_list}"
        done
        unset IFS
else
        dev_list=${*/#/0000:}
fi

for dev in ${dev_list}; do
        echo "Trying to retrieve IOMMU group of PCI device '${dev}'..."

        perm_update_path=$(basename /sys/bus/pci/devices/${dev}/vfio-dev/*/dev)
        # check if it's legacy or cdev
        if [ -e "${perm_update_path}" ]; then
                # cdev interface
                echo "PCI device '${dev}' uses cdev vfio interface"
                perm_update_path=$(basename /sys/bus/pci/devices/${dev}/vfio-dev/*)
                perm_update_path="/dev/vfio/devices/${perm_update_path}"
        else
                echo "PCI device '${dev}' uses legacy vfio interface"
                vfio_group_path=$(readlink "/sys/bus/pci/devices/${dev}/iommu_group")
                vfio_group_path=$(basename $vfio_group_path)
                perm_update_path="/dev/vfio/${vfio_group_path}"
        fi
        echo "Updating permissions of '${perm_update_path}'..."

        echo -e "\nTODO: update script to change permissions of '${perm_update_path}'!\n"
        #TODO
        # change permission of /dev/vfio/${vfio_group_path}
done

echo "NOTE: CDEV INTERFACE REQUIRES ALSO ADAPTION OF '/dev/iommu'"
echo ""
echo "$0 finished..."
