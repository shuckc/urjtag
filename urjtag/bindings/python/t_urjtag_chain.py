#!/usr/bin/python

#
# A general test and demonstration of several urjtag operations from python.
#

# works in both python 2 and 3
def printf(format, *args):
     """Format args with the first argument as format string, and print.
     If the format is not a string, it is converted to one with str.
     You must use printf('%s', x) instead of printf(x) if x might
     contain % or backslash characters."""
     sys.stdout.write(str(format) % args)

import sys
sys.path.append( "." )

import urjtag

#urjtag.loglevel(0) # ALL

urc = urjtag.chain()
printf("%s\n", urc);

urc.cable("JTAGKey")
printf("urc.cable done %s\n", urc)

urc.test_cable()
printf("urc.test_cable done\n")

f = urc.get_frequency()
printf("frequency was %d Hz\n", f)
urc.set_frequency(10000)
f = urc.get_frequency()
printf("frequency readback = %d Hz\n", f)

trst = urc.get_trst()
printf("TRST=%d\n", trst)
urc.set_trst(0)
trst = urc.get_trst()
printf( "TRST set 0 -> %d\n", trst)
urc.set_trst(1)
trst = urc.get_trst()
printf("TRST set 1 -> %d\n", trst)

urc.reset();

urc.tap_detect()
printf("urc.detect done\n")
printf("chainlength=%d\n", urc.len())

printf("id[0]=%08x\n", urc.partid(0) );

srstbit = 0x10
srstval = urc.get_pod_signal(srstbit)
printf( "srstval -> %s\n", srstval);

urc.set_pod_signal(srstbit, 0)
srstval = urc.get_pod_signal(srstbit)
printf("srstval set 0 -> %s\n", srstval)
urc.set_pod_signal(srstbit, 1)
srstval = urc.get_pod_signal(srstbit)
printf("srstval set 1 -> %s\n", srstval)

urc.set_instruction("SAMPLE/PRELOAD")
urc.shift_ir()
drval = urc.get_dr_in()
printf("BSR dr_in result: %s\n", drval)
urc.shift_dr()
drval = urc.get_dr_out()
printf("BSR dr_out result: %s\n", drval)

urc.set_instruction("IDCODE")
urc.shift_ir()
urc.shift_dr()
drval = urc.get_dr_out()
printf("IDREG dr result: %s\n", drval)

urc.set_instruction("BYPASS")
urc.shift_ir()
