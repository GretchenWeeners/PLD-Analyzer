 ----------------------------------------------------------------------------
 PAL device reader, Rev. 4
 (C) 2007-2011 Charles MacDonald
 WWW: http://cgfm2.emuviews.com
 ----------------------------------------------------------------------------

 ----------------------------------------------------------------------------
 Overview
 ----------------------------------------------------------------------------

 This is a design a for a 68000-based single board computer that
 can dump SPLDs such as the 16V8, 22V10, etc, in DIP and PLCC packages.

 Given the cost and complexity involved I would not seriously recommend
 building this project just to dump PALs, it is a prototype design I came
 up with to test a number of different ideas, mainly to design a 68K SBC
 and then to try multiple approaches to PAL dumping.

 Warning

 The files here are sort of a rough snapshot of my development files,
 there are parts missing from other projects that need to be included.
 I won't guarantee you can build the complete, functioning project from
 the material here alone.

 ----------------------------------------------------------------------------
 License
 ----------------------------------------------------------------------------

 - You can't sell any part of this project for profit.
 - You can't distribute any part of this project for free or for profit.
   This includes distributing on physical media or electronically.
 - You can't make derivative work based off this project.

 In other words, it is for you to use exclusively for yourself and
 nobody else.

 ----------------------------------------------------------------------------
 Support
 ----------------------------------------------------------------------------

 I will not provide any support.
 Don't ask for a PCB layout in other formats.
 Don't ask about when features will be completed.

 ----------------------------------------------------------------------------
 How it works
 ----------------------------------------------------------------------------

 The 68000 executes a startup program in boot ROM after the system has
 a power-on reset event. It then polls the USB module and processes
 commands that the Windows client software sends. Software (utility programs)
 are sent over USB and loaded into RAM for the 68000 to execute. These
 programs control the various hardware associated with the PALs:

 - I/O circuit to read and control the PAL pin levels.
 - Power supply circuit to provide power and control PAL power-up events.
 - Power monitor circuit to monitor voltage levels of the PAL power supply.
 - ISP serial interface for programming Lattice ispGAL22V10 parts.

 A design goal was to support registered PAL dumping. The hardware does
 this, but the 68K software is quite complex and is still under development.

 ----------------------------------------------------------------------------
 Configuration
 ----------------------------------------------------------------------------

 My board is set up as follows:

 X1  = 40 MHz OSC
 
   J_TVR jumper: Shorted
     ISP header: Jumper to short SDO to GND
    URST jumper: Pins 1-2 shorted
   J_PSW jumper: Pins 2-3 shorted
   P_RST jumper: Shorted
  J_4151 jumper: Pins 2-3 shorted
   P_MON jumper: Shorted
   P_LED jumper: Shorted
 P_22960 jumper: Pins 2-3 shorted (either one, it doesn't matter)

 EXT socket/header : Not connected to anything

 This configuration uses the TPS22960 to supply power to the PLD sockets
 exclusively, and the utility programs are written to support this setup.
 You could omit the TPS2051 and/or ST890B ICs then.

 ----------------------------------------------------------------------------
 Files
 ----------------------------------------------------------------------------

 analyze\pd.exe

 Use for analyzing 20 pin dumps only.
 The 'dump' command is only valid for the earlier board revisions.

 bootrom\boot.bin

 Write this to a AM29F040 flash chip

 client\main.exe

 Windows (command-line) software to control the board, mainly uploading
 software and downloading data.

 gal\*

 JED files for the three 22V10s (WSG, CKG, STB)

 pcb\*

 PCB layout

 util\dump20

 Program to dump 20-pin PALs such as 16V8
 Dumps are 256K.

 util\dump24

 Program to dump 24-pin PALs such as 22V10
 Dumps are 8MB.

 ----------------------------------------------------------------------------
 End
 ----------------------------------------------------------------------------
