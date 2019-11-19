# Interprocess Slim Reader/Writer (SRW) Locks Windows Native C++11

interproc_rw_lock class is designed to execute code with a inter-process sync mecanism for all sharing the same 'key'.
It reproduces the Slim Reader/Writer (SRW) Locks on native c++ windows but in an interprocess way.

So behaviour:
  - read mode : in parallel with other readers sharing the 'key' but no writer will own the lock,
  - write mode : alone. No reader or writer process sharing the 'key' will own the lock.
  
The main (Source.cpp) is simple code to understand well the mutex win32 api sync object.
The goal is to illustrate the zoology of win32 c++ mutexes but with smart management of mutex process-owner crash.

But best of all, no boost needed !

Build with visual >= 2017.
