import os
import re
import time
from datetime import datetime

def replece_after(wd, fileends, fieldname, value, replaceFieldnameWithValue=False):
    ''' w folderze wd lub podfolderach poszukaj pliku o nazwie konczacej sie na fielends,
        znajdz w nim wiersz rozpoczynajacy sie na fieldname, a wartosc za znakiem '=' w tej
        linii zamien na value'''
    file_fond = False
    for root, dirs, files in os.walk(wd):
        files_ = [f for f in files if f.endswith(fileends)]
        if len(files_) > 0:
            file_fond = True
            fin, fout = root + '/' + files_[0],  root + '/' + files_[0] + '_'
            with open(fin) as src, open(fout, 'w') as dst:
                for l in src.readlines():
                    if l.strip().startswith(fieldname):
                        if replaceFieldnameWithValue:
                            dst.write(l.replace(fieldname, value, 1))
                        else:
                            dst.write(l[:l.index('=')+1] + ' ' + str(value) + '\n')
                    else:
                        dst.write(l)
            os.remove(fin)
            os.rename(fout, fin)
            break
    if not file_fond:
        print('Brak plikow z koncowka ' + fileends + ' w ' + os.getcwd() + wd)

namePrefix = datetime.now().strftime('%Y-%m-%d')+'-L4V5'
os.chdir('./Debug')
replece_after('..', '.ld', 'KEEP(*(.bootloader));', '/*KEEP(*(.bootloader));*/', True)

keyvalmap = {"A":"1;", "B":"2;"}

for sufix in ["A", "B"]:
  replece_after('..', '.ld', 'COMPILE ', keyvalmap[sufix])
  os.system('python ../updateHash.py')
  os.system('make -j8 pre-build main-build ')
  name = namePrefix+sufix+'.bin'
  if os.path.isfile(name):
      os.remove(name)
  # following line is important, because make checks file changes by comparing files timestamp,
  # so when build will take less than 1s then new files will be seen as an old and make won't start compilation
  time.sleep(1)
  if not os.path.isfile('L4V5.bin'):
      print('Compilation failed')
      break
  os.rename('L4V5.bin', name)

replece_after('..', '.ld', 'COMPILE ', '0;')
replece_after('..', '.ld', '/*KEEP(*(.bootloader));*/', 'KEEP(*(.bootloader));', True)

trek_path = r"D:\UWB\RTLS_DotNET\TREK1000_Api\bin\Debug\TREK1000_Api.exe"
cmd = " -i "+namePrefix+'A.bin'+" -i "+namePrefix+'B.bin'+ " -o "+namePrefix+'.inh'+' --offset 2048'
os.system(trek_path + cmd)