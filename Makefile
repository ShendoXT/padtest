CDLIC_FILE = /usr/local/psxsdk/share/licenses/infoeur.dat
NOPS_CMD = mono ~/Tools/nops.exe
COM_PORT = /dev/cu.usbserial-10
BIN2H = python3 ~/Tools/bin2header.py

all:
	psx-gcc text.c graphics.c controllers.c padtest.c -o padtest.elf
	elf2exe padtest.elf padtest.exe -mark_eur
	#upx -9 padtest.exe --force

image:
	mkdir -p cd_root
	psx-gcc text.c graphics.c controllers.c padtest.c -o padtest.elf
	elf2exe padtest.elf padtest.exe -mark_eur
	cp padtest.exe cd_root
	systemcnf padtest.exe > cd_root/system.cnf
	mkisofs -o padtest.hsf -V padtest -sysid PLAYSTATION cd_root
	mkpsxiso padtest.hsf padtest.bin $(CDLIC_FILE)
	
res:
	bmp2tim images/Buttons.bmp images/Buttons.tim 8 -org=448,0 -clut=320,241 -mpink
	bmp2tim images/Mouse.bmp images/Mouse.tim 8 -org=480,0 -clut=320,242 -mpink
	$(BIN2H) -o images/buttons.h images/Buttons.tim
	$(BIN2H) -o images/mouse.h images/Mouse.tim
	
clean:
	rm padtest.elf
	rm padtest.exe
	rm padtest.hsf
	rm padtest.cue
	rm padtest.bin

run:
	$(NOPS_CMD) /exe padtest.exe $(COM_PORT) /m
	