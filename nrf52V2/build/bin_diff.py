#!/usr/bin/python3

def cmp(a, b, start):
  for i in range(start+1, min(len(a), len(b))):
    if a[i] != b[i]:
      return i
  return -1

def print_bin(table, ind):
  diff_len = 5
  start = max(0, ind - diff_len)
  stop = min(len(table), ind + diff_len + 1)
  msg = "\t" + ' '.join('{:02x}'.format(x) for x in table[start:ind])
  msg = msg + ' |{:02x}| '.format(table[ind]) + ' '.join('{:02x}'.format(x) for x in table[ind+1:stop])
  print(msg)

def main():
    fg_path = 'nrf52832_xxaaA.bin'
    fh_path = 'readed_mem.bin'
    fg = open(fg_path, 'rb')
    fh = open(fh_path, 'rb')
    try:
        fg_content = fg.read()
        fh_content = fh.read()
        max_len = min(len(fg_content), len(fg_content))

        print("fg len: " + str(len(fg_content)))
        print("fh len: " + str(len(fh_content)))

        ind = cmp(fg_content, fh_content, 0)
        while ind > 0:
          print("diff at " + hex(ind))
          print_bin(fg_content, ind)
          print_bin(fh_content, ind)
          ind = cmp(fg_content, fh_content, ind)
    finally:
        fg.close()
        fh.close()
    print("No more diffs")

def print_file_hex():
  # fg_path = 'nrf52832_xxaaA.bin'
  # fw_path = 'nrf52832_xxaaA.txt'
  fg_path = 'readed_mem.bin'
  fw_path = 'readed_mem.txt'
  fg = open(fg_path, 'rb')
  fw = open(fw_path, 'w+')
  try:
      fg_content = fg.read()
      max_len = len(fg_content)
      num_rows = 32
      ind = 0
      i = 1
      while ind < 0x1000:
        stop = min(max_len, ind + num_rows)
        fw.write("" + ' '.join('{:02X}'.format(x) for x in fg_content[ind:stop]) + "\n")
        ind = ind + num_rows
        i = i + 1
  finally:
      fg.close()
      fw.close()

if __name__ == "__main__":
  #print_file_hex()
  main()