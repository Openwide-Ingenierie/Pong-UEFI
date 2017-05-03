# Simple Pong! for UEFI

## Purpose of this project
The purpose of this project is to learn how to program for UEFI and to show it is not that hard. Here is a screenshot from the game running:

![Screenshot](screenshot.jpg)

## Compiling this project

To compile this project, you need `EDK II`, follow the instructions given on their official documentation page which can be find here: https://github.com/tianocore/tianocore.github.io/wiki/Getting-Started-with-EDK-II

## Launching the game

After compiling, rename the generated `Pong.efi` to `BOOTX64.EFI` (or `BOOTIA64.EFI` or `BOOTARM.EFI`, depending on the target) and put it on a FAT16 or FAT32 USB key in the path:

```
/EFI/BOOT/BOOTX64.EFI
```

## Controls

s / x : Control left bat
Up / Down : Control right bat
