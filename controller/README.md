Serial PIC Programmer Arduino Controller
========================================

This is the PC controller for my Arduino-based PIC programmer. It is intended to be used with an Arduino running my associated sketch for programming PIC chips with ICSP. A blog post is to follow soon and I will link there for more information.

Building
========

```
$ make
```

Running
=======

```
$ ./programmer /path/to/your/arduinoSerialDevice /path/to/your/PIC/hex/file.hex
```

