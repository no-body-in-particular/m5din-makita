# m5din-makita
Makita lockout flag reset tool based on https://github.com/mnh-jansson/open-battery-information .
Pinout and how to connect things can be found in the pictures, a further explanation can be found in martin's repository.
For 14.4v batteries please connect gnd to the nearest pin on the 8 pin header.
For very old 18v batteries you may need 5v logic, since an ESP uses 3.3v logic you will need a level converter when talking to old batteries that are completely dead.

OpenBatteryInformation is Copyright (c) 2024 Martin Jansson - MIT licensed software. I've taken it as inspiration and modified it heavily - and added options to extract more information from batteries.
Please use the included OneWire library as makita has funny timings.

As you can also see on the connection pictures, only one set of wires from the m5din meter is used, both data lines hooked up to the 5v power out on that channel of the din meter with a 4k7 pullup resistor. Ground is connected to battery ground - and power for the m5din is pulled from the battery positive.

I've included a custom makita BMS that i used to debug/test the protocol, you can use it as you please.


As for the "why makita": their 5ah 18v batteries can take an absolute *ton* of abuse, and outside of 18v 12ah milwaukee batteries - protection mosfets wise their current capacity is really unmatched.
I like the batteries, and i like being able to repair and maintain my own :)

That said, there are chinese PCB clones with roughly the same rDSOn mosfets wise - https://aliexpress.com/item/1005007278381624.html being an example, or the PCBs found here https://aliexpress.com/item/1005007589522529.html .
If RdsON is no concern, and casing size is a problem https://aliexpress.com/item/1005008036314483.html is a good example of a pcb with undervoltage protection.
Downside of the later is that the mosfet CAN go into hysteresis when hitting the low voltage cutoff, and this can kill the mosfet.
Another downside of the above PCBs is that they do consume more current than the original makita PCBs, hence the batteries need charging more often to stay alive.
Above PCBs are still MUCH better than the older makita PCBs which do not have any of those protections however, and should be preferred for those batteries.
