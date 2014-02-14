all: linux_display.c
	gcc -Wall --pedantic -O2 linux_display.c -lXll -o displayImage