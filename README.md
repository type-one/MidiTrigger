# MidiTrigger
Bind shell commands to MIDI keyboard (i.e. Korg nanoKONTROL2)


Fast written Simple MIDI to commands framework

Allow to bind MIDI triggers/buttons to shell script commands


The json file contains an example of bindings that were tuned for Korg Nano KONTROL2

Replace the "echo" commands with your own scripts or command line executables


The framework is totally agnostic to the commands and the json file injects the dependencies


Can be compiled and Linux and Windows.

On Linux, just use cmake .
On Windows, just use cmake-gui to generate a Visual Studio solution
Should be easily adapted to MacOSX as well

Tested on Windows 10, Ubuntu Linux,  and Raspberry Pi 3 / Ubuntu Linux

Launch in a shell using "miditrigger miditrigger.json"  (json file as 1st argument)



Enjoy

Laurent Lardinois / Type One (TFL-TDV)

https://be.linkedin.com/in/laurentlardinois

https://demozoo.org/sceners/19691/
