# Install prerequisites

The following works for Debian Buster. Steps would be extremely close
on recent Ubuntus but exact package names may vary and will
definitely vary on distros that aren’t Debian derivatives.

```
$ sudo apt-get update && apt-get upgrade -y && apt-get install -y \
    libcanberra-gtk3-module dbus-x11 ninja-build virtualenv \
    libxml2-dev pkg-config libglib2.0-dev libgoocanvas-2.0-dev \
    libwebkit2gtk-4.0-dev libgtksourceview-4-dev libgspell-1-dev \
    libplist-dev desktop-file-utils libgstreamer1.0-dev \
    gstreamer1.0-plugins-bad gstreamer1.0-plugins-good \
    gstreamer1.0-tools git build-essential
```
## Meson

Inform IDE requires meson >= 0.55.0. Debian Buster's is too old,
but if you're or Debian Bullseye (or Sid) or Ubuntu 20.10 (or 21.04)
could just apt install meson.

```
$ git clone -b gtk3 https://github.com/ptomato/gnome-inform7.git
$ cd gnome-inform7
$ virtualenv --python=python3 meson
$ source meson/bin/activate
$ pip install meson==0.55
```
At this point you have to manually copy the appropriate ni binary
for your architecture to gnome-inform7/src/ni/ni -– if you already
have Inform 7 installed, you already have a copy, probably in
/usr/local/libexec/ni , otherwise find it in

http://inform7.com/apps/6M62/I7_6M62_Linux_all.tar.gz

Once that’s done (still in the same gnome-inform7 directory as above):

```
$ meson setup build
$ cd build
$ meson compile
$ sudo meson install # WARNING! will overwrite any existing inform7-ide under /usr/local

# now you can run it with
$ LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu inform7-ide
# or if you don't do meson install, run with:
$ LD_LIBRARY_PATH=gnome-inform7/build/subprojects/ratify:gnome-inform7/build/subprojects/chimara gnome-inform7/build/src/inform7-ide
```

Or for whichever of those is appropriate, you could put the following in a
file called, say, i7ide and put it in /usr/local/bin or elsewhere in your PATH
(doing a ``chmod +x i7ide`` to ensure it's executable).

```
#!/bin/bash
LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu inform7-ide "$@" &
```

To re-compile you can just go to gnome-inform7/build and run meson compile again.
Sourcing meson/bin/activate just puts meson/bin in your PATH. So you can do that,
or just explicitly run gnome-inform7/meson/bin/meson, e.g.,

$ cd gnome-inform7/build
$ ../meson/bin/meson compile

Unless you've created a user dedicated to building inform7-ide (not a terrible idea),
I recommend against sourcing meson/bin/activate in your bashrc. When you use a virtualenv
, for the duration of your session it's the python instance that takes priority, and
it's a python that sees only the libraries you've installed there and not ones you may
have installed system-wide.


