import subprocess
import os
import re

targetDir = "source/version.rst"
sourceVersionPath = "../src/gitversion.h"
F_MAJOR_DEF = "__F_MAJOR__"
F_MINOR_DEF = "__F_MINOR__"
F_HASH_DEF = "__F_HASH__"

# read current version from sourceVersionPath
with open(sourceVersionPath) as f:
    for l in f.readlines():
        if F_MAJOR_DEF in l:
            l = l[len(F_MAJOR_DEF) + l.find(F_MAJOR_DEF):].strip()
            firmwareMajor = int(l)
        elif F_MINOR_DEF in l:
            l = l[len(F_MINOR_DEF) + l.find(F_MINOR_DEF):].strip()
            firmwareMinor = int(l)
        elif F_HASH_DEF in l:
            l = l[len(F_HASH_DEF) + l.find(F_HASH_DEF):].strip()
            hash = int(l, 16)

print("Version: {}.{}".format(firmwareMajor, firmwareMinor))
print("Hash:    {}".format(hex(hash)))

str = '''
.. _firmware version:

================
Firmware version
================


This manual describe firmare in version::

  Major:   {}
  Minor:   {} / {}
  Hash:    {}

Check version
=================

To check version on target device see :ref:`version`

  this file is generated automatically from script
'''.format(firmwareMajor, 2*firmwareMinor, 2*firmwareMinor+1, hex(hash % 2**64))
strspl = str.split('\n')

print("target dir: " + os.getcwd() + targetDir)
with open(targetDir, 'a+') as fr:
    content = fr.readlines()
    if len(content) == 0:
        content = ["fake value, to show that file needs to be updated"]
    for i, l in enumerate(content):
        if i > len(strspl) or strspl[i] != l:
            fr.close()
            with open(targetDir, "w+") as f:
                f.write('\n')
                f.write(str)
                print(targetDir + ' updated')
                exit(0)

print(targetDir + ' is already up to date')


def getDefineValue(path, define):
    result = ""
    with open(path) as f:
        for l in f.readlines():
            if define in l:
                result = l[len(define) + l.find(define):].strip()
                break
    return result
