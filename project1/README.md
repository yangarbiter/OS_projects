NTU CSIE Operating System Project 1
=====
project detail: http://rswiki.csie.org/dokuwiki/courses:102_2:project_1

## System Call Interfaces
* gettime(time_t* s, long* ns);<br>
s-->time in second, ns-->time in nanosecond.

* printkk(char* s);<br>
call printk(s) in user mode.

## Misc
* sigusr1 are used to set nice to -19.
* sigusr2 are used to set nice to 20.
