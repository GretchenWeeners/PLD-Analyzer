@echo off
snasm68k -p main.asm,boot.rom,,main.lst
checkrom\checkrom boot.rom

