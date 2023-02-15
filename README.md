### CreateProcessStressTest

This is a simple tool designed to stress-test the operating system's process
table. It supports three modes:

* Default: create and exit
* <count>: create that many child processes and exit
* --fork <count>: divide count into a batch per core and run them in a child

If you have any anti-virus software enabled it will likely make this much
slower than it might otherwise be. You can disable realtime-protection for
the duration of your test to make it run faster.

