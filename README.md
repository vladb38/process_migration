process_migration
=================

Process Migration in FreeBSD 5.2

See the YouTube [video] (https://www.youtube.com/watch?v=0am-5noTrWk) for more details.

Process migration implemented in the FreeBSD operating system. This allows you to take a small static executable, stop it on one machine and resume it on another machine from the exact point where it has been stopped.

To be able to compile the kernel modules you will need to create four symlinks in this directory:

@ -> /usr/src/sys
machine -> /usr/src/sys/amd64/include
usr/machine -> /usr/src/sys/amd64/include
usr/sys -> /usr/src/sys

Then run "make" in the current directory.

In order to test the kernel module, compile the "hellosimple.c" code in the "example" directory
and compile it as a static executable using:

"gcc -static -o hellosimple hellosimple.c"

You can then follow the instructions in the youtube video and see whether you are able to move
this simple process from one host to another:

"http://www.youtube.com/watch?v=0am-5noTrWk"

Let me know if you have questions, doubts, suggestions.

Vlad
