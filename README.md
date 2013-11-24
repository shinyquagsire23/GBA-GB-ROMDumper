GBA/GB ROM Dumper
----------------

As a GBA ASM hacker one of the things that was always so mystical about the GBA was it's cartridges. For the longest time I wondered how all those little pins could amount to readable hexadecimal that can be run by the ARM7TDMI chip within the GBA. However, I have finally unlocked this mystery and I can finally say I know how the GBA works both from a software and hardware perspective.

Currently this dumper is still a WIP. However, by the end of development I plan to have a fully functional GBA ROM dumper that can not only dump ROMs, but can also write and read save files as well as dump the ROMs and saves from the original GameBoy. I also plan on implementing an SD card into the design, but I am still awaiting the parts to do so.

However, since this dumper is functional, I decided to provide the breakout PCB I used (based on insidergadgets' PCB - thanks!) as well as the wiring diagram shown below:

![An Arduino Mega 2560 with a rainbow of wires connecting to another circuit board with a GBA cartridge sticking out of it](http://i569.photobucket.com/albums/ss135/mtinc2/GBAROMDumper_zps70c396cc.png)

License
-------

<i>GBA/GB ROM Dumper</i>

<i>Copyright (C) 2013  Max Thomas</i>

<i>This program is free software: you can redistribute it and/or modify</i>
<i>it under the terms of the GNU General Public License as published by</i>
<i>the Free Software Foundation, either version 3 of the License, or</i>
<i>any later version.</i>

<i>This program is distributed in the hope that it will be useful,</i>
<i>but WITHOUT ANY WARRANTY; without even the implied warranty of</i>
<i>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the</i>
<i>GNU General Public License for more details.</i>

<i>You should have received a copy of the GNU General Public License</i>
<i>along with this program.  If not, see http://www.gnu.org/licenses/</i>


*Special Thanks to drhelius and insidergadgets for their GameBoy ROM dumpers. They really inspired me to continue from their work in addition to giving me an idea of how to interface with the GameBoy cartridges properly and efficiently. Also a shoutout to Fritzing.org for their circuit diagram which helped a lot in showing how this is hooked up!*


