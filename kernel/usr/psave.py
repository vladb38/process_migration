#! /usr/local/bin/python
# Copyright (c) 2010 Vlad Balan

 
#    Copyright (c) 2010, Vlad Balan
#    All rights reserved.

#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#    * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#    * Neither the name of the Vlad Balan nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Vlad Balan BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



import sys;
import os;

TMPDIR="/tmp/"

pid = sys.argv[1]

if not (os.path.exists(TMPDIR+pid)):
    os.makedirs(TMPDIR+pid);

# stop it
os.system('kill -STOP '+pid);

# save the kernel structures
os.system('./psave '+pid+' '+TMPDIR+pid+'/kerndata.d')

# optional, make a copy of the process map for later reference
os.system('dd if=/proc/'+pid+'/map of='+TMPDIR+pid+'/map')

# get the executable name
f = os.popen("ls -l /proc/"+pid+"/file | awk '{print $11}'")
filename = f.read()
f.close()
filename = filename.rstrip()
os.system("ls -l /proc/"+pid+"/file | awk '{print $11}' | tr -d '\n' > "+TMPDIR+pid+"/filename")


#save the open file descriptors
os.system('lsof -p '+pid+ '> '+TMPDIR+pid+'/lsof.dat');

#save the open sockets
os.system("sockstat | awk '{if ($3 == "+pid+" ) print $0}' > "+TMPDIR+pid+'/sockets.dat');   

# create a core dump
os.system('kill -SIGSEGV '+pid);
os.system('kill -CONT '+pid);

os.system('./parse_fd.py '+pid+' > '+TMPDIR+pid+'/lsfd.dat')

# move the core dump to the destination
os.system('mv '+filename+'.core '+TMPDIR+pid+'/data.core');
os.system('chmod +x '+TMPDIR+pid+'/data.core');

#compress all
os.system('tar -cjPf '+pid+'.tar.bz2 ' +TMPDIR+pid);

os.system('sync');

# we are no longer saving the memory through this script
# os.system('./psave_mem.py ' +pid);
