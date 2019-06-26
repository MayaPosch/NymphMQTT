# NymphMQTT #

This project aims to support both MQTT 3.x and 5, targeting C++ and Ada. It uses components from the [NymphRPC](https://github.com/MayaPosch/NymphRPC "NymphRPC") Remote Procedure Call library for the networking side.

## Goals ##

* MQTT 3.1.1 & MQTT 5-compatibility.
* Easy integration with C++ and Ada client applications.
* Integrated MQTT broker.
* Light-weight and versatile.
* Minimal dependencies.
* Cross-platform (Windows, Linux/BSD, MacOS, etc.).
* Multi-broker (multiple brokers per client).

## Status ##

The project status, for each port.

### C++ ###

* Basic client connect/subscribe/publish/receive support works.
* Connecting to multiple brokers should work.
* Basic MQTT 5 properties support is being integrated.
* LibPOCO as only dependency at this point.

### Ada ###

The Ada port at this point is being planned. Development will likely commence after the C++ port has stabilised sufficiently.

## Tests ##

A number of unit/integration tests can be found in each port's folder, compilable using the provided Makefile.