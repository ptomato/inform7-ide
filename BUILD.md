# Build instructions

These are the instructions for building the app from its source code,
which you might want to do if you are contributing to its development,
or trying out a pre-release.

These instructions assume that you have a Linux distribution that is not
too old; the most recent long-term-support (LTS) version of Ubuntu Linux
(at the time of writing, 24.04) or stable version of Debian Linux (at
the time of writing, Bookworm) will do.
The instructions should be fairly similar for any other Debian-based
distributions, as well.

For Fedora Linux, most instructions will be similar but the package
names in the next step will be different.
You'll need to work out yourself which packages to install, but the
[RPM spec file](./inform7-ide.spec) might be helpful.

## Install prerequisites

```bash
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install -y \
    libcanberra-gtk3-module dbus-x11 ninja-build meson \
    libxml2-dev pkg-config libglib2.0-dev libgoocanvas-2.0-dev \
    libwebkit2gtk-4.1-dev libgtksourceview-4-dev libgspell-1-dev \
    libhandy-1-dev libplist-dev desktop-file-utils libgstreamer1.0-dev \
    gstreamer1.0-plugins-bad gstreamer1.0-plugins-good \
    gstreamer1.0-tools git build-essential
```

## Get the source code

This step will get a copy of the source code of the Inform compiler and
the app, and several other tools needed to build them.

```bash
git clone -b v7.2.0 https://github.com/ganelson/inweb.git
git clone -b v2.1.0 https://github.com/ganelson/intest.git
git clone -b v10.1.2 https://github.com/ganelson/inform.git
git clone https://github.com/ptomato/inform7-ide.git
```

Note, this gets specific versions of the Inform compiler and its tools.
If that's not what you want — for example, if you want the very newest
features of Inform — then you might need to build a different branch of
the app.
(If you are doing that, we'll assume you know how to work with Git
branches and tags.)
In that case, leave off the tags from inweb, intest, and inform, and
get the `inform-next` branch of inform7-ide.
If there is no `inform-next` branch, then that's probably because it's
not needed; use the `main` branch in that case.

## Build the Inform compiler

There are instructions in the `README.md` files of the inweb, intest,
and inform repositories, but they are summarized below for convenience:

```bash
bash inweb/scripts/first.sh linux
bash intest/scripts/first.sh
cd inform
bash scripts/first.sh
```

(That's just for the first build.
If you already built Inform once, and are just updating it after pulling
from GitHub, then instead do this:)

```bash
cd inform
make -C .. -f inweb/inweb.mk
make -C .. -f intest/intest.mk
make
```

Regardless of whether this was a first build or a subsequent build, you
now need to copy a file that tells where to install the Inform compiler
and its materials, to a place where the Inform build process can find
it, one level above the `inweb`, `intest`, and `inform` folders:

```bash
cp ../inform7-ide/build-aux/make-integration-settings.mk ..
```

Now do the following to install the files into the inform7-ide folder:

```bash
make forceintegration
make retrospective
cp -R retrospective ../inform7-ide/
```

(On subsequent builds, you don't need to keep copying
`make-integration-settings.mk` unless it has changed.
If you're not sure, then copy it every time.)

## Build the app and run for testing

Once that’s done, go to the `inform7-ide` folder:

```bash
cd ../inform7-ide
```

Then you can prepare the build.
If the previous steps succeeded, the following command should not
complain about being unable to find files:

```bash
meson setup _build
```

Now you can compile and run the app, with

```bash
./build-aux/run_uninstalled.sh
```

## System-wide installation

To install it in your system, do the following.
Note that this will **overwrite** any existing installation of the app
if you installed a DEB or RPM package.

```bash
meson _build -Dprefix=/usr
ninja -C _build
sudo ninja -C _build install
```

After doing that, you'll find it in the apps grid or menu or wherever
else you start your apps from.

To install it _alongside_ an existing version, you can also do it
slightly differently; replace `-Dprefix=/usr` with `-Dprefix=/usr/local`
in the above command.
That will overwrite any other copy you installed in `/usr/local` but not
the package in `/usr`.
If you do it this way, you have to start it from the terminal; run it
with

```bash
LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu inform7-ide
```

Or you could put the following in a file called, say, `i7ide` and put it
in `~/.local/bin` or elsewhere in your `PATH` (doing a `chmod +x i7ide`
to ensure it's executable.)

```bash
#!/bin/bash
LD_LIBRARY_PATH=/usr/local/lib/x86_64-linux-gnu inform7-ide "$@" &
```
