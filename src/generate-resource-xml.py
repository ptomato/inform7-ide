#!/usr/bin/env python3

import argparse
import os

parser = argparse.ArgumentParser(
    description='Generate GResource XML from Inform data archive')
parser.add_argument('current_source_dir')
parser.add_argument('output', type=argparse.FileType('w'))
args = parser.parse_args()

inform_dir = os.path.join(args.current_source_dir, 'inform')

print("""\
<?xml version="1.0" encoding="UTF-8"?>
  <!-- Generated file! Do not edit. -->
  <gresources>
    <gresource prefix="/com/inform7/IDE">""", file=args.output)

for (root, dirs, files) in os.walk(inform_dir):
    # 'licenses' dir is separate from Inform data archive, handled in
    # src/com.inform7.IDE.gresource.xml
    if 'licenses' in dirs:
        dirs.remove('licenses')

    dirs.sort()
    files.sort()

    for name in files:
        string = '    <file'

        if name.endswith('.html'):
            string += ' compressed="true"'

        relpath = os.path.relpath(root, args.current_source_dir)
        string += f'>{relpath}/{name}</file>'
        print(string, file=args.output)

print("""\
  </gresource>
</gresources>""", file=args.output)
