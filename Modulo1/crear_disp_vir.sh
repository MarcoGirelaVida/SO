#!/bin/bash
sudo mknod /dev/loop0 b 7 0
sudo mknod /dev/loop1 b 7 1
sudo dd if=/dev/zero of=/root/archivo_SA20 bs=2k count=10000
sudo dd if=/dev/zero of=/root/archivo_SA30 bs=3k count=10000
sudo losetup /dev/loop0 /root/archivo_SA20
sudo losetup /dev/loop1 /root/archivo_SA30
