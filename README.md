# ArduinoMixer
This program is a driver for an "external machine", a board with slider potentiometers and a arduino uno board.
ArduinoMixer gets all of the active audio sessions in windows, formats them and sends it to the arduino board.

# Features
### Reservation:
You can reserv specific spots on the board.
If you do that spot will always be allocated for the specific program.
For example: if I reserv spot 1 for Chrome, that spot cannot be anything else than Chrome. 
If Chrome is not open it will be called "RESERV"

### Exclusion:
You can exclude programs.
For example: if you exclude the "system" audio session, it will not show up on the screen.
This always take priority!

### Rename:
Since ArduinoMixer gets the process name of all sessions it might be a weird name.
For example: windows music "Groove"s process name is "MUSIC.UI", then you can rename it to "GROOVE". 

# VERSION 4
### New Features
Version 4 has some new hardware features. The volume wheels are now rotary encoders, instead of potentiometer. Which means better customisation and accuracy.
<p align="center">
<img src="doc/ArduinoMixer_v_4.jpg" align="center"/> 
 </p>

# VERSION 3
<p align="center">
<img src="doc/ArduinoMixer_v_3.jpg" align="center"/> 
 </p>

# VERSION 2
<p align="center">
<img src="doc/ArduinoMixer_v_2.jpg" align="center"/> 
 </p>
