#!/usr/bin/python

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

# trst = urc.get_trst()
# printf("TRST=%d\n", trst)
# urc.set_trst(0)
# trst = urc.get_trst()
# printf( "TRST set 0 -> %d\n", trst)

# urc.set_trst(1)
# trst = urc.get_trst()
# printf("TRST set 1 -> %d\n", trst)

srstbit = 0x10
srstval = urc.get_pod_signal(srstbit)
printf( "srstval -> %s\n", srstval);

urc.set_pod_signal(srstbit, 0)
srstval = urc.get_pod_signal(srstbit)
printf("srstval set 0 -> %s\n", srstval)
urc.set_pod_signal(srstbit, 1)
srstval = urc.get_pod_signal(srstbit)
printf("srstval set 1 -> %s\n", srstval)
