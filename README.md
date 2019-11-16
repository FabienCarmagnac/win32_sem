# win32_sem

interproc_rw_lock class is designed to execute code with a inter-process sync mecanism for all sharing the same 'key'.
Read mode : in parallel with other readers sharing the 'key' but no writer
Write mode : alone. No other reader or writer process sharing the 'key' can execute this code on the system.

The mail (Source.cpp) is simple code to understand well the mutex win32 api sync object.
The goal is to illustrate the zoology of win32 c++ mutexes but with smart management of mutex process-owner crash.

  - no dependancy. No boost. lol.
  - 100% native c++ windows api based. No boost or other heavy 

Build with visual >= 2017.
