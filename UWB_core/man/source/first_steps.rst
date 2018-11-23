.. _First steps:


================
First steps
================

Usage
================

IndoorNavi is a system designed to track position of machines and peoples. There may by a lot of purposes to do that, for example:

* Alert when somebody working in dangerous environment doesn't move for a long while
* Detect some logistic bottle-neck in magazines or factory
* Check statistics how many times peoples or things spend in some area


Configuration
================

After you receive set of sink, few anchors and tags then you can start configure it.

#. Install Anchors on target places, connect power supply or battery
#. Turn on devices
#. Connect Sink to computer
#. Download TREK software
#. Connect to Sink via serial from TREK software
#. Configure measure lists


How to configure measure lists?
After devices boot, then sink will receive :ref:`INF_DEVICE_TURN_ON` and unused devices will send periodically :ref:`INF_BEACON`.
It's a good mechanism to discover devices identification numbers (*did*).
Then you can set measure list with :ref:`setanchors` and :ref:`settags` commands.
For example when you discovered that in your set is:

* **Sink** with did *8001*
* **Tags** with did *10* and *A2*
* **Anchors** with did *9001* *A001* *9003*

Then you should put::

  setanchors 8001 9001 A001 9003
  settags 10 A2 8001 9001 A001 9003

Now anchors should be auto-located on a screen and tag moves should be visible in TREK.
In this mode each device is measured to each other - it is a good mode for demo of system.
In target system there shouldn't be measures between anchors to save measure time for tags.
To do that just reset measure with :ref:`clear` command and create new measure table without measures between anchors::

  clear -m
  setanchors 8001 9001 A001 9003
  settags 10 A2


Remote server
================

If you want to connect to remote server then:

#. In TREK folder rename 'setup_template.xml' to 'setup.xml'
#. Edit server settings (see TREK manual)
