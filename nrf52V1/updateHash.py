import subprocess
import os
import re


# find 'COMPILE' in linker script
# to set right version of software
numer_of_flash_loc = 0
print('work directory: ' + os.getcwd())
for root, dirs, files in os.walk('../'):
    linkerfile = [f for f in files if f.endswith('.ld')]
    if len(linkerfile) >= 1:
        linkerfile = linkerfile[0]
        print(linkerfile)
        with open('uwb_nrf_gcc_nrf52.ld') as f:
            for l in f.readlines():
                if l.strip().startswith('COMPILE'):
                    numer_str = re.search(r'\d+', l).group()
                    print('COMPILATION: ' + numer_str)
                    numer_of_flash_loc = int(numer_str) - 1
                    if numer_of_flash_loc < 0:
                        numer_of_flash_loc = 0
                    break


hash = str(subprocess.check_output(['git', 'rev-parse', 'HEAD']))[2:-3]
hash = int(hash[0:8], 16)
print("GIT hash:   \"" + hex(hash) + "\"")
branch = str(subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']))[2:-3]
print("GIT branch: \"" + branch + "\";")

firmwareMajor = 0
firmwareMinor = 2
firmwareMinorMax = 256*256

str = '''// file generated automatically from updateHash.py, do not edit

# ifndef __GITVERSION_H__
# define __GITVERSION_H__

# define __F_MAJOR__\t{}
# define __F_MINOR__\t{}
# define __F_HASH__ \t{}
# endif
'''.format(firmwareMajor, 2*firmwareMinor+numer_of_flash_loc, hex(hash % 2**64))
strspl = str.split('\n')

targetDir = "../../UWB_core/src/gitversion.h"
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
                print('gitversion.h updated')
                exit(0)

print('gitversion.h is already up to date')
