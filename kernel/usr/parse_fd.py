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
import re;

pid = sys.argv[1]

if not (os.path.exists(pid)):
    exit(-1)

f = os.popen("cat "+pid+"/lsof.dat")
text = f.readlines()
f.close()

c = 0
for line in text:
    cline = line.split()
    if (c > 1):
        fd = cline[3]
        fdtype = cline[4]
        if (fdtype != 'VBAD'):
            off = cline[6]
            node = cline[7]
            name = cline[8]
            if ((fd != "cwd") and (fd != "rtd") and (fd != 'txt')):
                fdnum = fd
                fdacc = fd
                fdnum = re.sub("\D","",fdnum)
                fdacc = re.sub("[0-9]","",fdacc)
                # process a FD
                if (node != 'UDP'):                
                    fdtype = 'FD'
                    print(fdtype + " " + fdnum + " "+ fdacc + "" + " " + off + " " + name)
                # process a UDP socket
                if (node == "UDP"):
                    fdtype = "UDP"
                    f = os.popen("cat "+pid+"/sockets.dat | awk '{if ($4 == "+fdnum+") print $0}'")
                    ttext = f.readlines()
                    f.close()                    
                    ttext = ttext[0]
                    cudp = re.split("[: ]+", ttext)
                    address = cudp[7]
                    port = cudp[8]
                    print(fdtype + " " + address + " " + port)
                    
    c = c + 1

