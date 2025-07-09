### Description
This is dummy linux kernel character device driver that only shows how much this device has been opened.

### Build

```
$ git clone https://github.com/plvhx/dummy-chardev.git
$ cd dummy-chardev
$ cd module
$ make
$ cd ..
$ sudo cp udev/rules.d/99-chrdev.rules /etc/udev/rules.d
$ sudo insmod modules/dummy.ko
```

### Check with insmod and udevadm

```
$ lsmod | grep dummy
dummy                  12288  0
$ udevadm info -a -p /sys/class/dummy/dummy
Udevadm info starts with the device specified by the devpath and then
walks up the chain of parent devices. It prints for every device
found, all possible attributes in the udev rules key format.
A rule to match, can be composed by the attributes of the device
and the attributes from one single parent device.

  looking at device '/devices/virtual/dummy/dummy':
    KERNEL=="dummy"
    SUBSYSTEM=="dummy"
    DRIVER==""
    ATTR{power/async}=="disabled"
    ATTR{power/control}=="auto"
    ATTR{power/runtime_active_kids}=="0"
    ATTR{power/runtime_active_time}=="0"
    ATTR{power/runtime_enabled}=="disabled"
    ATTR{power/runtime_status}=="unsupported"
    ATTR{power/runtime_suspended_time}=="0"
    ATTR{power/runtime_usage}=="0"
$ ls -l /dev/dummy
crw-rw-rw- 1 root root 511, 0 Jul  9 13:03 /dev/dummy
```

### Test

Try to run this command: ```cat /dev/dummy```
