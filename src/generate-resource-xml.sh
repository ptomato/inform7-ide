#!/bin/bash -e

informtarball=$1
gresource=$2
script_dir=$(dirname $0)
mkdir -p inform
if test "$informtarball" -nt "$gresource"; then
    xz -F lzma -dc "$informtarball" | \
        tar -xv -C inform --strip-components=1 Documentation Resources | \
        "$script_dir/generate-resource-xml.py" > \
        "$gresource"
else
    echo "No changes in Inform data archive, skipping GResource generation"
fi
