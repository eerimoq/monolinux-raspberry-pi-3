Monolinux Raspberry Pi 3
========================

A Monolinux distro for Raspberry Pi 3!

Monolinux: https://github.com/eerimoq/monolinux

Features
========

- Fast development cylce (a matter of seconds from source code change
  to entering user space).

- Everything the Linux kernel provides (networking, filesystems,
  drivers, etc).

- Various libraries provided by `Monolinux`_.

Build and run
=============

One time setup
--------------

Build a Raspberry Pi 3 image using Buildroot, insert an SD card into
the PC and write the image to it.

.. code-block:: shell

   $ cd ..
   $ git clone https://github.com/buildroot/buildroot.git
   $ cd buildroot
   $ make raspberrypi3_defconfig
   $ make
   $ sudo dd if=output/images/sdcard.img of=/dev/sdX
   $ cd monolinux-raspberry-pi-3

Open ``/media/X/config.txt`` and replace ``#initramfs rootfs.cpio.gz``
to ``initramfs initramfs.cpio``.

Daily development
-----------------

Build this project and copy the kernel and ramdisk to the SD card,
overwriting the existing files with the same name.

.. code-block:: shell

   $ ./rundocker.sh
   $ make -s -j8
   $ exit
   $ cp app/build/initramfs.cpio /media/X
   $ cp app/build/linux/arch/arm/boot/zImage /media/X
   $ sync

Now insert the SD card into your Raspberry Pi 3 and power it up. The
console is on the serial port on the P1 header.

.. _Monolinux: https://github.com/eerimoq/monolinux
