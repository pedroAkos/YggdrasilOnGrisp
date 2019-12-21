# YggdrasilOnGrisp

This repository contains the code of [Yggdrasil](https://github.com/LightKone/Yggdrasil) modified to work with RTEMS within the [GRiSP](https://www.grisp.org) platform.

The libyggdrasil folder contains the source code of Yggdrasil. To compile, the Makefile should be changed to reflect the your directory structure that contains the RTEMS tool chain.

The myapp folder contains a simple Erlang application for GRiSP that uses Erlang NIFs to use the functionalities provided by Yggdrasil directly in Erlang.
