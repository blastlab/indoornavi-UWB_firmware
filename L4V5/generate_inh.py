import os
import re
from datetime import datetime
import generate_bin_packet

namePrefix = datetime.now().strftime('%Y-%m-%d')+'-L4V5'
os.chdir('./Debug')

keyvalmap = {"A":"1;", "B":"2;"}

for sufix in ["A", "B"]:
  generate_bin_packet.replece_after('..', '.ld', 'COMPILE ', keyvalmap[sufix])
  os.system('python ../updateHash.py')
  #os.system('make clean')
  os.system('make -j8 pre-build main-build ')
  name = namePrefix+sufix+'.bin'
  if os.path.isfile(name):
      os.remove(name)
  os.rename('L4V5.bin', name)

generate_bin_packet.replece_after('..', '.ld', 'COMPILE ', '0;')

trek_path = r"D:\UWB\RTLS_DotNET\TREK1000_Api\bin\Release\TREK1000_Api.exe"
cmd = " -i "+namePrefix+'A.bin'+" -i "+namePrefix+'B.bin'+ " -o "+namePrefix+'.inh'+' --offset 2048'
os.system(trek_path + cmd)