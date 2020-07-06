# PC client software for Sega USB Link V1.0
# (c) 2004  Charles MacDonald

CC	=	gcc
LDFLAGS	=	-s
FLAGS	=	-I.
LIBS	=	ftd2xx.lib

# Emulation core
OBJ	=       obj/main.o	\
		obj/usb.o

EXE	=	main.exe

all	:	$(EXE)

$(EXE)	:	$(OBJ)
		$(CC) -o $(EXE) $(OBJ) $(LIBS) $(LDFLAGS)
	        
obj/%.o : 	%.c %.h
		$(CC) -c $< -o $@ $(FLAGS)
	        
        
pack	:
		strip $(EXE)
		upx -1 $(EXE)	        

clean	:	        
		rm -f obj/*.o
		rm -f *.bak
		rm -f *.exe
		rm -f *.log
		rm -f *.wav
		rm -f *.zip

makedir :
		mkdir obj
	        
archive:	        
		pk -dir -add -max \
		-excl=rom -excl=src -excl=test -excl=zip \
		-excl=obj -excl=doc -excl=bak -excl=out \
		mdsrc.zip *.*
	        
#
# end of makefile
#
