
def generate_messages_description(in_file, out_file):
  for l in in_file.readlines():
    code, name, description = 0, "", ""
    ml = l[l.find("(")+1:].strip()[:-1]  # zawartosc pomiedzy ( i )
    s = ml.split(",")
    if "ADD_ITEM_M(" in l:
      code, name = int(s[0]), s[1].strip()
      start, stop = ml.find("\""), ml.rfind("\"")
      description = ml[start+1:stop]
    elif "ADD_ITEM(" in l:
      code, name = int(s[0]), s[1].strip()
      description = name
    elif "COMMENT(" in l:
      start, stop = ml.find("\""), ml.rfind("\"")
      comment = ml[start+1:stop]
      out_file.write("*comment*: " + comment + "\n\n")
      continue
    else:
      continue
    out_file.write(
"""  .. _{1}:

**{1}**


*code:* {0}

*descriptor:* "{2}"

""".format(code, name, description))



input_prefix = "../src/logger/"
out = open("source/messages.rst", "w")

out.write(
""".. _messages:

========
Messages
========

.. _information messages:

Informations
============

""")

generate_messages_description(open(input_prefix + "logs_inf.h"), out)

out.write("""
.. _warning messages:

Warnings
============

""")

generate_messages_description(open(input_prefix + "logs_wrn.h"), out)

out.write("""
.. _error messages:

Errors
============

""")

generate_messages_description(open(input_prefix + "logs_wrn.h"), out)

out.write("""
.. critical messages:

Critical
============

""")

generate_messages_description(open(input_prefix + "logs_crit.h"), out)

out.write("""
.. _test messages:

Test
============

""")

generate_messages_description(open(input_prefix + "logs_test.h"), out)