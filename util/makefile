# PAL dumping & analysis utility
# (C) 2007-2008 Charles MacDonald

CC	=	gcc
LDFLAGS	=	ftd2xx.lib
FLAGS	=	-O1 -Wall -Werror
LIBS	=

OBJ	=       obj/main.o	\
		obj/intf.o	\
		obj/mcu.o	\
		obj/usb.o

EXE	=	pd.exe

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
		rm -f *.zip

makedir :
		mkdir obj
	        	        
#
# end of makefile
#
