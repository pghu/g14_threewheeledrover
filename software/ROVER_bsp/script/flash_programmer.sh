#!/bin/sh
#
# This file was automatically generated.
#
# It can be overwritten by nios2-flash-programmer-generate or nios2-flash-programmer-gui.
#

#
# Converting SOF File: C:\Users\Peter\Desktop\DE0\Demonstration\ROVER_NOFLASH\DE0_Nano.sof to: "..\flash/DE0_Nano_epcs.flash"
#
sof2flash --input="C:/Users/Peter/Desktop/DE0/Demonstration/ROVER_NOFLASH/DE0_Nano.sof" --output="../flash/DE0_Nano_epcs.flash" --epcs --verbose 

#
# Programming File: "..\flash/DE0_Nano_epcs.flash" To Device: epcs
#
nios2-flash-programmer "../flash/DE0_Nano_epcs.flash" --base=0x1800 --epcs --sidp=0x6000060 --id=0x0 --accept-bad-sysid --device=1 --instance=0 '--cable=USB-Blaster on localhost [USB-0]' --program --verbose 

#
# Converting ELF File: C:\Users\Peter\Desktop\DE0\Demonstration\ROVER_NOFLASH\software\ROVER_NOFLASH\ROVER_NOFLASH.elf to: "..\flash/ROVER_NOFLASH_epcs.flash"
#
elf2flash --input="C:/Users/Peter/Desktop/DE0/Demonstration/ROVER_NOFLASH/software/ROVER_NOFLASH/ROVER_NOFLASH.elf" --output="../flash/ROVER_NOFLASH_epcs.flash" --epcs --after="../flash/DE0_Nano_epcs.flash" --verbose 

#
# Programming File: "..\flash/ROVER_NOFLASH_epcs.flash" To Device: epcs
#
nios2-flash-programmer "../flash/ROVER_NOFLASH_epcs.flash" --base=0x1800 --epcs --sidp=0x6000060 --id=0x0 --accept-bad-sysid --device=1 --instance=0 '--cable=USB-Blaster on localhost [USB-0]' --program --verbose 

