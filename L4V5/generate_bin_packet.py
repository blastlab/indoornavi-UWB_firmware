import os
import re

def replece_after(wd, fileends, fieldname, value):
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
                        dst.write(l[:l.index('=')+1] + ' ' + str(value) + '\n')
                    else:
                        dst.write(l)
            os.remove(fin)
            os.rename(fout, fin)
            break
    if not file_fond:
        print('Brak plikow z koncowka ' + fileends + ' w ' + os.getcwd() + wd)


# lista opcji - łączymy wszystko z wszystkim
role = [['RTLS_SINK,', 'S'], ['RTLS_ANCHOR,', 'A'], ['RTLS_TAG,', 'T']]
fminors = [['1', '1'], ['2', '2']]
compilations = [['1;', 'A'], ['2;', 'B']]

for r in role:
    for m in fminors:
        for c in compilations:
            replece_after('.', '.ld', 'COMPILE ', c[0])
            replece_after('.', 'updateHash.py', 'firmwareMinor ', m[0])
            replece_after('../UWB', 'settings.c', '.role ', r[0])

            os.chdir('./Debug')
            os.system('python ../updateHash.py')
            os.system('make all')
            name = r[1]+c[1]+'G'+m[1]+'.bin'
            if os.path.isfile(name):
                os.remove(name)
            os.rename('L4v1.bin', name)
            os.chdir('../')

# przywroc ustawienia domyslne
replece_after('.', '.ld', 'COMPILE ', c[0][0])
replece_after('.', 'updateHash.py', 'firmwareMinor ', m[0][0])
replece_after('../UWB', 'settings.c', '.role ', r[0][0])

print('  _____ ')
print('/@@@@@@@\\')
print('| *   * |')
print('    |   ')
print('  \___/ ')
print('\   ||   /')
print(' `------` ')