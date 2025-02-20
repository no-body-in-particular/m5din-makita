# m5din-makita
Makita lockout flag reset tool based on https://github.com/mnh-jansson/open-battery-information .
Pinout and how to connect things can be found in the pictures, a further explanation can be found in martin's repository.
For 14.4v batteries please connect gnd to the nearest pin on the 8 pin header.
For very old 18v batteries you may need 5v logic, since an ESP uses 3.3v logic you will need a level converter when talking to old batteries that are completely dead.

OpenBatteryInformation is Copyright (c) 2024 Martin Jansson - MIT licensed software. I've simply adapted it to run standalone on a m5din meter.
Please use the included OneWire library as makita has funny timings.

As you can also see on the connection pictures, only one set of wires from the m5din meter is used, both data lines hooked up to the 5v power out on that channel of the din meter with a 4k7 pullup resistor. Ground is connected to battery ground - and power for the m5din is pulled from the battery positive.

I've included a custom makita BMS that i used to debug/test the protocol, you can use it as you please.
