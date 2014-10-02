
/* 
   Copyright (c) 2010, Vlad Balan
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   * Neither the name of the Vlad Balan nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Vlad Balan BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <sys/syscall.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

#include "../pmigrate.h"

#define SYSCALL_NUM 211

#define DATASIZE 80000

char buf[DATASIZE];
char elfbuf[DATASIZE];
char cwd[MAXPATHLEN];
char execfile[MAXPATHLEN];
char execstrfile[MAXPATHLEN];
char kerndatafile[MAXPATHLEN];
char corefile[MAXPATHLEN];
char* path;

int main(char argn, char **argv)
{
  long int pid;

  if (argn > 1)
    {
      pid = strtol(argv[1], (char **) NULL, 10);
      if (pid == 0) return;
      path = argv[2];
      if (path == NULL) return;
    }
  else
    printf("usage: pload pid_to_hack path_to_data\n");

  /* getcwd(cwd, MAXPATHLEN); */

  sprintf(execstrfile, "%s/%s/filename", cwd, path);
  sprintf(kerndatafile, "%s/%s/kerndata.d", cwd, path);
  sprintf(corefile, "%s/%s/data.core", cwd, path);

  printf("pid: %d \n", (int) pid);
  printf("kerndata: %s \n", kerndatafile);
  printf("filename: %s \n", execstrfile);
  printf("corefile: %s \n", corefile);

  int fd = open(kerndatafile, O_RDONLY);
  read(fd, buf, sizeof(buf));
  close(fd);

  fd = open(corefile, O_RDONLY);
  read(fd, elfbuf, 8192);
  close(fd);

  fd = open(execstrfile, O_RDONLY);
  read(fd, execfile, 8192);
  close(fd);

  printf("execfile: %s \n", execfile);

  struct pm_data_ref pm_data;
  pm_data.pid = (int) pid; /* hello world's PID */
  pm_data.addr = (char*) buf;
  pm_data.elfaddr = (char*) elfbuf;
  pm_data.execfile = execfile;
  pm_data.corefile = corefile;

  printf("pm_data address %p \n", &pm_data);

  syscall(SYSCALL_NUM, &pm_data);

  struct pm_header *pm_hdr = (struct pm_header *) buf;

  printf("returned pid: %d \n", pm_hdr->pid);
  printf("returned len: %ud \n", pm_hdr->len);

  printf("\n ");

  while (2>1);
  
  return 0;
}
