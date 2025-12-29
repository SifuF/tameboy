rgbasm.exe -o main.o main.asm
rgblink.exe -o ../../../roms/balls.gb main.o
del main.o