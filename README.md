# Arduino MIDI Controller

**A Program to use an arduino uno board as a MIDI controller.**    
This mixer can be used along with other audio editing/mixing software like FL studio, Cubase or Traktor.

Since this program is developed to send MIDI signals through the serial port of the arduino, it is needed to use another software
(not delivered here) like "hairless MIDI <-> Serial Bridge"  to parse the serial signal to a MIDI signal, then you can use a driver
llamado Midiyoke, to read that signal as a physical MIDI input connected to the computer.

