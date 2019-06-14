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
For example: if you exclude the "system" audio session, it will not show up on the screen 

### Rename:
Since ArduinoMixer gets the process name of all sessions it might be a weird name.
For example: windows music "Groove"s process name is "MUSIC.UI", then you can rename it to "GROOVE". 

#VERSION 2
![img_20190220_165407__01](https://user-images.githubusercontent.com/17643866/53105077-c3220600-3530-11e9-8349-2fe2258260be.jpg)

#VERSION 3
![ArduinoMixer_Version_3_Mark_2](https://user-images.githubusercontent.com/17643866/59508867-ab8bd600-8eaf-11e9-8e17-ca0de0841388.PNG)
