# ArduinoMixer
This program is a driver for an "external machine", a board with slider potentiometers and a arduino uno board.
ArduinoMixer gets all of the active audio sessions in windows, formats them and sends it to the arduino board.

# Features

### Reservation:
You can reserv specific spots on the board.
If you do that spot will always be allocated for the specific program.
For example: if I reserv spot 1 for Chrome, that spot will always be chrome, if it is open, 
otherwise it will say "RESERV"

### Exclusion:
You can exclude programs.
For example: if you exclude the "system" audio session, it will not show up on the screen 

### Rename:
Since ArduinoMixer gets the process name of all sessions it might be a weird name.
For example: windows music "Groove"s process name is "MUSIC.UI", then you can rename it to "GROOVE". 
