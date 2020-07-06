
 PLD analyzer
 (C) 2007-2008 Charles MacDonald
 WWW: http://cgfm2.emuviews.com

 Table of contents

 1.) Features
 2.) Updates
 3.) Driver information
 4.) Overview of files
 5.) Bill of materials
 6.) Power supply
 7.) Assembly information
 8.) Notes
 9.) Disclaimer

 ----------------------------------------------------------------------------
 Features
 ----------------------------------------------------------------------------

 - Supports 20-pin PALs and GALs in a DIP package with at most 18 inputs
   and 10 outputs such as the 16L4/6/8, 16V8, etc.

 - Automatic detection of pin functions and device configuration.

 - CUPL output can be modified to fit the original design in a different
   replacement device.

 ----------------------------------------------------------------------------
 Updates
 ----------------------------------------------------------------------------

 (01/12/08)
 - Initial release

 ----------------------------------------------------------------------------
 Driver information
 ----------------------------------------------------------------------------

 You can get the latest FTDI device drivers for your operating
 system from this webpage:

 http://www.ftdichip.com/Drivers/D2XX.htm

 ----------------------------------------------------------------------------
 Overview of files
 ----------------------------------------------------------------------------

 Included in the archive is:

 /pcb   ExpressPCB board layout
 /sch   Circuit schematic
 /util  Source code to utility program
 /bin   Utility program (pal.exe, Windows binary)

 The PCB file (paldump-final.pcb) can only be used by the ExpressPCB software:

 http://www.expresspcb.com/ExpressPCBHtm/Download.htm

 It meets the requirements of a MiniBoard, which is their most economical
 PCB service.

 The schematic file can be viewed with BSch3V:

 http://www.suigyodo.com/online/e/index.htm

 ----------------------------------------------------------------------------
 Bill of materials
 ----------------------------------------------------------------------------

 Loc.  Description                          What I used

 Integrated circuits

   U1 - DLP-2232M module                     DLP-2232M
   U2 - GAL22V10      (24-pin DIP)           Lattice GAL22V10B
   U3 - 74HC245       (20-pin DIP)           TI SN74HC245N
   U4 - 74HC574       (20-pin DIP)           TI SN74HC574N
   U5 - 74HC245       (20-pin DIP)           TI SN74HC245N
   U6 - 74HC574       (20-pin DIP)           TI SN74HC574N
   U7 - 74HC574       (20-pin DIP)           TI SN74HC574N
   U8 - 74HC273       (20-pin SOIC)          TI SN74HC273DW
   U9 - 74HC273       (20-pin SOIC)          TI SN74HC273DW
  U10 - 74HC126       (14-pin SOIC)          TI SN74HC126D
  U11 - 74HC126       (14-pin SOIC)          TI SN74HC126D
  U12 - DS1233        (TO-220)               Maxim DS1233
 REG1 - 7805          (TO-220)               TI uA7805C

 Capacitors

   C1 - 0.1uF ceramic
   C2 - 0.1uF ceramic (1206)
   C3 - 0.1uF tantalum or metal foil
   C4 - 220uF radial electrolytic (25V)

 Resistors

  RN1 - 4.7K ohm resistor network, isolated, 16-pin DIP
  R1  - 330 ohm resistor

 Diodes

   D1 - 1N4007 (1A)
 LED1 - High efficiency red T-1 3/4 lamp    Fairchild HLMP-3301

 Connectors

   J1 - 2 pin header (Optional: GPIOH0,1 pins)
   J2 - 2 pin header (Optional: power supply)

 Sockets

 - 40-pin machine tooled socket U1
 - 24-pin soldertail socket for U2
 - 20-pin soldertail socket for U3,U4,U5,U6,U7
 - 16-pin machine tooled socket for the PAL16V8 

 * Mouser carries the DLP-2232M module.

   http://www.mouser.com

 * Jameco carries everything else.

   http://www.jameco.com

 ----------------------------------------------------------------------------
 Power supply
 ----------------------------------------------------------------------------

 Use a +9V, 800mA to 850mA power supply. The connector is 2.1mm, negative
 tip, positive ring.

 I would strongly recommend adding a heatsink to the 7805 if you can
 squeeze one in.

 ----------------------------------------------------------------------------
 Assembly information
 ----------------------------------------------------------------------------

 Solder the SMT capacitors and SOIC parts first, be careful about the
 limited space between U8 and U9.

 Solder all remaining components.

 ----------------------------------------------------------------------------
 Notes
 ----------------------------------------------------------------------------

 You can omit U8, U9, U10, U11 and U12. This section of the circuit
 provides programmable output drivers to control the bidirectional pins
 of the device you are dumping. In practice, I found the pull-up/pull-down
 resistors implemented with RN1 and U4 were enough to drive the
 bidirectional pins to valid logical levels when they were configured
 as inputs.

 J1 and J2 do not need to be populated.

 U2 has a lot of extra connections for future expansion, if you can't
 program PLD devices I'm sure some simple logic could replace the
 address decoding. See the CUPL source code (u2.pld) to see the address
 decoding logic.

 ----------------------------------------------------------------------------
 Disclaimer
 ----------------------------------------------------------------------------

 - Use at your own risk. Anything involving the assembly of electronics
   or running a circuit off a power supply is inherently dangerous.

 - I cannot provide this device in a kit or assembled form. It is a
   do-it-yourself project.

 - The software cannot be used in whole or in part in any other program.
   Contact me if you want to port it to another operating system or make
   any derivative versions.

 ----------------------------------------------------------------------------
 End
 ----------------------------------------------------------------------------
