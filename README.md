# Arduino MIDI Controller :musical_note:

This project was based on a "MIDI Fighter" project from Instructables.com, so thanks to the original author of this:
https://www.instructables.com/Arcade-Button-MIDI-Controller/

**So, to begin with this project I have used an Arduino UNO board.**    
The controller has this mixer controller style layout but it can be customized of course, and it can work with any DAW software that accepts generic MIDI mappings. 

Note: The arduino program sends these MIDI messages through the serial port of the arduino board, thus to make it visible to the host software it is needed to virtually emulate the serial signal as a true MIDI input signal to be recognized by the DAW's.

To achieve this I used an emulator (not delivered here) called "hairless MIDI <-> Serial Bridge" first to parse the Serial into MIDI, and then, I used a driver
called "Midiyoke", to read the parsed signal as a physical MIDI input connected to the computer.

Another option (instructions not included here) would be to add the MIDI coverter circuit interface or buy a compatible pre-made physical interface converter.

![](https://i.ibb.co/zxtFNwQ/emulador.png)

**Initially I used a paper box for the body**  
![](https://i.ibb.co/yB9dyPR/IMG-1118.jpg)

**Then I decided to move it to a plastic box that I adapted**
![](https://i.ibb.co/p2FkyDw/IMG-1122.jpg)
![](https://i.ibb.co/64h5RW7/IMG-1123.jpg)

**I tested it with a couple of softwares like Virtual dj, Traktor and FL studio and it worked well, and had lots of fun using it**
![](https://i.ibb.co/sQk05Ns/IMG-1124.jpg)

**You can find some videos of me testing the midi controller using virtual DJ**
Please check them manually at the root of the repository :)

Thank you for your visit here.

