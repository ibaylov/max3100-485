# max3100-485
MAX 3100 driver class for Arduino,  geared towards usage in RS-485 Modbus  applications

# Your feedback matters

I provide this code to you in hope you'll find it useful without seeking any profit or other benefits. So if you find it useful please drop a comment or hit the star button above.

# Implementation notes

You can by all means use the code to control the UART in an implementation that has nothing to do with RS-485. My implementation was driven by some practical issues I encountered when implementing RS-485 for Modbus, below are share these with you.

## RTS Control 
In my implementation I use MAX3100 RTS signal to control MAX487CPA RS-485 transciever Data Out Enable (DE).
It is not necessary to implement DE control this way. You can use any GPIO to enable the data transfer, but using MAX100 RTS may greatly simplify the hardware implementation if you decide to use multiple MAX3100 with a single CPU. 

## Block Transfer
What I found to be a deficiency in other implementations is the lack of block transfer methods, so I have implemented these. Block transfer greatly simplifies line control and timing, and is a nantural way to approach a block-oriented protocol, Modbus included.


## Handling Interrupts
I use a very simplistic approach for the interrupt handling. Inside the ISR nothing is actually done besides setting the incoming data flag, which in turn can be polled from the application.
The ISR is using a singleton pointer that is assigned in MAX3100485::begin. If you plan to use more than one MAX3100 on a single board this part should be refactored. I may refactor this to more general form in the future.  
