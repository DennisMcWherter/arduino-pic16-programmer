Serial PIC Programmer
=====================

This repo contains the code for my Arduino-based serial PIC programmer. The architecture is rather simple:

  1. The PC takes PIC `.hex` file and streams this to the Arduino (line by line)
  1. The Arduino processes the stream and programs the PIC16 chip with each line
      * The Arduino does various amounts of error checking and validation for each word written 
  1. The Arduino reports back when it has finished

I will link to the coming blog post for more information soon!
