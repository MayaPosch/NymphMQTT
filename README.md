# NymphMQTT #

This project aims to support both MQTT 3.x and 5, targeting C++ and Ada. It uses components from the [NymphRPC](https://github.com/MayaPosch/NymphRPC "NymphRPC") Remote Procedure Call library for the networking side.

The C++ version implements the MQTT 3.x client features in 1,412 lines of code:

	-------------------------------------------------------------------------------
	Language                     files          blank        comment           code
	-------------------------------------------------------------------------------
	C++                             10            412            449            964
	C/C++ Header                    12            209            141            448
	-------------------------------------------------------------------------------
	SUM:                            22            621            590           1412
	-------------------------------------------------------------------------------

## Goals ##

* MQTT 3.1.1 & MQTT 5 support.
* Easy integration with C++ and Ada client applications.
* Integrated MQTT broker.
* Light-weight and versatile.
* Minimal dependencies.
* Cross-platform (Windows, Linux/BSD, MacOS, etc.).
* Multi-broker (multiple active brokers per client).

## Building ##

**C++:**

Dependencies are:

* LibPOCO
* [ByteBauble](https://github.com/MayaPosch/ByteBauble)

Navigate to the `src/` folder and run the `make` command there. Alternatively use either of these options:

	$ make test

This will build the library and the test applications, equivalent to running make without options.

	$ make lib

This will only build the library. It can be found in the 'src/lib' folder afterwards. 

## Status ##

The project status, for each port.

### C++ ###

* All MQTT 3.1.1 (v4) client features have been implemented.
* Connecting to multiple brokers should work.
* MQTT 3.1.1 server (broker) features are being implemented.
* MQTT 5 support is being integrated.


### Ada ###

The Ada port at this point is being planned. Development will likely commence after the C++ port has stabilised sufficiently.

## Tests ##

A number of unit/integration tests can be found in each port's folder, compilable using the provided Makefile.