import os

def get_value(value_name, path="."):
  for root, dirs, files in os.walk(path, topdown=False):
    for name in files:
        with open(root+'/'+name) as f:
          for cnt, l in enumerate(f):
            if value_name in l:
              val = l[len(value_name)+l.find(value_name)+1:].strip()
              print(l[:-1])  # do not print \n
              return val
  raise("Not found " + value_name)

def insert_const(file, name, value, comment=""):
  file.write(
"""
.. _{0}:

{0} = {1}

{2}

""".format(name, value, comment)
  )

input_prefix = "../src/logger/"
out = open("source/constants.rst", "w")

out.write(
""".. _constants:

================
Constants
================


""")

v = get_value("#define MEASURE_TRACE_MEMORY_DEPTH", "../src/")
insert_const(out, "MAX_MEASURE_TRACES", v)

v = get_value("#define TOA_MAX_DEV_IN_POLL", "../src/")
insert_const(out, "TOA_MAX_DEV_IN_POLL", v)

v = get_value("#define RANGING_TEMP_ANC_LIST_DEPTH", "../src/")
insert_const(out, "SETANCHORS_MAX_NUMBER", v)

v = get_value("#define CARRY_MAX_TARGETS", "../src/")
insert_const(out, "MAX_ANCHORS_PER_SINK", v)

v = get_value("#define CARRY_MAX_TAGS", "../src/")
insert_const(out, "MAX_TAGS_PER_SINK", v)