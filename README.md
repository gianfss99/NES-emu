# NES-emu
Object-Oriented NES Emulator written in C++

After how much I learned and enjoyed developing my first emulator, a CHIP-8 virtual machine, 
I decided to delve further into the world of emulation. The NES seemed like a substantial step 
up from CHIP-8, while keeping it relatively simple hardware-wise.

The emulator consists of a virtual 6502 CPU and a 2C02 PPU (Picture Processing Unit) to render
graphics. The CPU includes all legal opcodes in the instruction set with per instruction accuracy.
The PPU has single-cycle accuracy, and keeps up with the CPU to maintain correct synchronization.
The graphical output and controller input are achieved using SDL2.

This project was meant to familiarize myslef with hardware virtualization, going through lots of 
documentation, and design a system that performs accurately and efficiently.

![](https://github.com/gianfss99/NES-emu/blob/main/DK_gameplay.gif)
