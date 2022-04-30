# Installation instructions

These are installation instructions for Debian Linux distributions,
particularly Debian Buster. The steps will be fairly close on other
versions of Debian, as well as recent versions of Ubuntu.

## Install prerequisites

Exact package names may vary and will definitely vary on distros that
aren’t Debian derivatives.

```
$ sudo apt-get update
$ sudo apt-get upgrade -y
$ sudo apt-get install -y \
    libcanberra-gtk3-module dbus-x11 ninja-build virtualenv \
    libxml2-dev pkg-config libglib2.0-dev libgoocanvas-2.0-dev \
    libwebkit2gtk-4.0-dev libgtksourceview-4-dev libgspell-1-dev \
    libplist-dev desktop-file-utils libgstreamer1.0-dev \
    gstreamer1.0-plugins-bad gstreamer1.0-plugins-good \
    gstreamer1.0-tools git build-essential
```

## Meson

Inform IDE requires meson >= 0.55.0. Debian Buster's is too old,
but if you're on Debian Bullseye (or later) or Ubuntu 20.10 (or later)
you could just `apt install meson` instead of the below.

```
$ git clone https://github.com/ptomato/inform7-ide.git
$ cd inform7-ide
$ virtualenv --python=python3 meson
$ source meson/bin/activate
$ pip install meson==0.55
```

At this point you have to manually copy the appropriate `ni` binary
for your architecture to `inform7-ide/src/ni/ni` -– if you already
have Inform 7 installed, you already have a copy, probably in
`/usr/local/libexec/ni`, otherwise find it in
[the compiler package](http://inform7.com/apps/6M62/I7_6M62_Linux_all.tar.gz).

## Build and run for testing

Once that’s done (still in the same `inform7-ide` directory as above):

```
$ meson _build
```

Now you can compile and run it, for testing, with

```
$ ./build-aux/run_uninstalled.sh
```

## System-wide installation

To install it in your system, do the following.
Note that this will **overwrite** any existing copy of inform7-ide under
`/usr/local`.

```
$ meson _build -Dprefix=/usr/local
$ ninja -C _build
$ sudo ninja -C _build install
```

Now you can run it with

```
$ LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu inform7-ide
```

Or you could put the following in a file called, say, `i7ide` and put it in
`/usr/local/bin` or `~/.local/bin` or elsewhere in your `PATH` (doing a
`chmod +x i7ide` to ensure it's executable).

```
#!/bin/bash
LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu inform7-ide "$@" &
```

## Notes

* If you are upgrading from version 5J39 or earlier, and you are having
  problems with extensions, you may have to delete your `~/Inform`
  directory and reinstall your extensions.
* If you have installed a pre-release package, you may need to first
  remove it entirely before you install an official release.
* If you were using the virtualenv / `source meson/bin/activate` method
  of installing Meson, you will have to run `source meson/bin/activate`
  every time you go to build Inform 7 again in a new terminal window.

I recommend against sourcing meson/bin/activate in your bashrc. When you use a virtualenv
, for the duration of your session it's the python instance that takes priority, and
it's a python that sees only the libraries you've installed there and not ones you may
have installed system-wide.
