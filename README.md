mbpfan
====================

This is an enhanced version of [Allan McRae mbpfan](http://allanmcrae.com/2010/05/simple-macbook-pro-fan-daemon/)

mbpfan is a daemon that uses input from coretemp module and sets the fan speed using the applesmc module.
This enhanced version assumes any number of processors and fans (max. 10).

*  It only uses the temperatures from the processors as input.
*  It requires coretemp and applesmc kernel modules to be loaded.
*  It requires root use
*  Users can configure it using the file /etc/mbpfan.conf

**Table Of Contents**

- [Supported GNU/Linux Distributions](#supported-gnulinux-distributions)
- [Tested Macbook Models](#tested-macbook-models)
- [Tested iMac/Mac Mini Models](#tested-imacmac-mini-models)
- [Requirements](#requirements)
- [Installation](#installation)
- [Run Instructions](#run-instructions)
- [Starting at boot](#starting-at-boot)
- [Usage](#usage)
- [License](#license)
- [Credits](#credits)

## Supported GNU/Linux Distributions

We provide scripts to to load mbpfan daemon at system boot for many distros.
Please note that the support is provided by volunteers. mbpfan needs tests and bug reports.

Supported distributions:

- Alpine
- Archlinux
- CentOS
- Debian
- Fedora
- Gentoo
- RedHat
- Solus
- Trisquel
- Ubuntu

## Tested Macbook Models

This section reports those models where mbpfan was tested successfully. It does not necessarily mean that the daemon does not work on non-listed models.

- MacBook Pro 12,1 13"  (Intel i5 - Linux 4.20)
- MacBook Pro 11,4 15"  (Intel i7 - Linux 4.9.41)
- MacBook Pro 11,1 13"  (Intel i5 - Linux 3.14, Linux 3.15)
- MacBook Pro 9,2 13"  (Intel i5/i7 - Linux 3.10)
- MacBook Pro 8,2 15"  (Intel i7 - Linux 3.6.2)
- MacBook Pro 8,1 13"  (Intel i7 - Linux 3.2.0)
- MacBook Pro 7,1 15"  (Intel Core 2 Duo - Linux 3.13)
- MacBook Pro 6,2 15"  (Intel i7 - Linux 3.5.0)
- MacBook Pro 6,2 15"  (Intel i7 - Linux 3.2.0)
- MacBook Pro 2,2 15"  (Intel Core 2 Duo - Linux 3.4.4)
- MacBook Air 5,2 13"  (Intel i5 - Linux 3.16)
- MacBook Air 1,1 13"  (Intel Core Duo - Linux 4.4, Linux 4.8)
- MacBook Air 7,2 13"  (Intel Core Duo - Linux 4.10)
- MacBook 1,1 (Intel Core Duo - Linux 3.16)

## Tested iMac/Mac Mini Models

This section reports the iMac/Mac-mini models where mbpfan was tested successfully.

- iMac Retina 16.2 21.5" (Intel i5 - Linux 4.4.0 Ubuntu 16.04)  
- iMac 12,1 21.5" (Intel i7 - Linux 4.6.4)
- iMac 5,1 17" (Intel T7400 (Core 2 Duo) - Linux 14.04 Ubuntu)
- Mac Mini 2,1 (Core 2 Duo - Linux 4.4.0)
- Mac Mini 5,3 (Core i7 2.0 - Linux 4.4.0 elementary/Ubuntu)
- Mac Mini 6,1 (Core i7 2.3 - Linux 4.7.3-4-ck Archlinux)

## Requirements

Be sure to load the kernel modules **applesmc** and **coretemp**.

These modules are often automatically loaded when booting up GNU/Linux on a MacBook. If that is not the case, you should make sure to load them at system startup.

**How do I know if applesmc and coretemp are loaded?**

In most distributions, you can run the following command:

`lsmod | grep coretemp && lsmod | grep applesmc`

If you see `coretemp` and `applesmc` listed, you are all set.

**If you do not see `coretemp` and `applesmc` listed, you must load them.**

This is _usually_ achieved by inserting the following two lines in the file `/etc/modules`

```
coretemp
applesmc
```

Please check the relevant documentation of your GNU/Linux distribution.

## Installation

Compile with

    make

Install with

    sudo make install

It copies mbpfan to /usr/sbin, mbpfan.conf to /etc (and overwrites existing files),
README.md to /usr/share/doc/mbpfan, and mbpfan.8.gz to /usr/share/man/man8

Run the tests now, see two sections below.

If you would like to compile with Clang instead of GCC, simply set your system's
default compiler to be Clang. Tested with Clang 3.8 and 3.9. Tested with Clang
4.0 along with llvm-lld (The LLVM Linker).


Run The Tests (Recommended)
---------------------------
It is recommended to run the tests after installing the program. Please run the following command _from within the source directory_.

    sudo ./bin/mbpfan -t

or

    sudo make tests


## Run Instructions

If not installed, run with

    sudo bin/mbpfan

If installed, manually run with

    sudo mbpfan

If installed and using the init file, run with (Ubuntu example)

    sudo service mbpfan start


## Starting at boot

For systemd based init systems, the file mbpfan.service
has been provided.
For using it, execute:

    sudo cp mbpfan.service /etc/systemd/system/
    sudo systemctl daemon-reload
    sudo systemctl enable mbpfan.service
    sudo systemctl start mbpfan.service


## Usage

    Usage: ./mbpfan OPTION(S)

    -h Show the help screen
    -t Run the tests


## License

GNU General Public License version 3


**This Project Is Based On:**

* http://allanmcrae.com/2010/05/simple-macbook-pro-fan-daemon/
* http://allanmcrae.com/2011/08/mbp-fan-daemon-update/
* https://launchpad.net/macfanctld
* http://paste2.org/p/862259
* http://www.lobotomo.com/products/FanControl/


## Credits

**This Project uses following library:**

* [ANSI C Application Settings Managment](http://pokristensson.com/settings.html) by Per Ola Kristensson.
