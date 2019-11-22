#!/usr/bin/env python

import sys

print("""\
<?xml version="1.0" encoding="UTF-8"?>
  <!-- Generated file! Do not edit. -->
  <gresources>
    <gresource prefix="/com/inform7/IDE">""")

for line in sys.stdin:
    line = line.rstrip()
    if line.endswith('/'):
        continue

    if line.startswith('Documentation/'):
        line = line.replace('Documentation/', 'inform/', 1)
    if line.startswith('Resources/'):
        line = line.replace('Resources/', 'inform/', 1)

    string = '    <file'

    if line.endswith('.html'):
        string += ' compressed="true"'

    string += '>' + line + '</file>'
    print(string)

print("""\
  </gresource>
</gresources>""")
