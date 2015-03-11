CDLIC_FILE = /usr/local/psxsdk/share/licenses/infoeur.dat

all:
	mkdir -p cd_root
	psx-gcc padtest.c -o padtest.elf
	elf2exe padtest.elf padtest.exe -mark_eur
	cp padtest.exe cd_root
	systemcnf padtest.exe > cd_root/system.cnf
	mkisofs -o padtest.hsf -V padtest -sysid PLAYSTATION cd_root
	mkpsxiso padtest.hsf padtest.bin $(CDLIC_FILE)
	
res:
	bmp2tim images/Buttons.bmp images/Buttons.tim 8 -org=448,0 -clut=320,241 -mpink
	bin2h images/Buttons.tim images/buttons.h ButtonsTimData -nosize
	
clean:
	rm padtest.elf
	rm padtest.exe
	rm padtest.hsf
	rm padtest.cue
	rm padtest.bin
	
debug:
	send padtest.exe com6