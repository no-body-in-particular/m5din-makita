# m5din-makita
Makita lockout flag reset tool based on https://github.com/mnh-jansson/open-battery-information .

OpenBatteryInformation is Copyright (c) 2024 Martin Jansson - MIT licensed software. I've simply adapted it to run standalone on a m5din meter.
Please use the included OneWire library as makita has funny timings.

As you can also see on the connection pictures, only one set of wires from the m5din meter is used, both data lines hooked up to the 5v power out on that channel of the din meter with a 4k7 pullup resistor. Ground is connected to battery ground - and power for the m5din is pulled from the battery positive.
