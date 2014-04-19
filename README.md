OS Project 1
=====

## System Call Interfaces
* gettime(time_t* s, long* ns);<br>
s-->time in second, ns-->time in nanosecond.

* printkk(char* s);
call printk(s) in user mode.

## Misc
* sigusr1 are used to set nice to -19.
* sigusr2 are used to set nice to 20.
