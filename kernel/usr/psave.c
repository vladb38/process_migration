
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
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

#include "../pmigrate.h"

#define SYSCALL_NUM 210 /* this might change */

#define DATASIZE 80000

char buf[DATASIZE];

int main(char argn, char **argv)
{
  long int pid;

  char* filename;
  FILE* fd;

  if (argn > 0)
    {
      pid = strtol(argv[1], (char **) NULL, 10);
      if (pid == 0) return;
    }

  if (argn > 1)
    {
      filename = argv[2];
    }
  else return;

  struct pm_data_ref pm_data;
  pm_data.pid = (int) pid; /* saved program PID */
  pm_data.addr = (char*) buf;

  syscall(SYSCALL_NUM, &pm_data);

  struct pm_header *pm_hdr = (struct pm_header *) buf;

  printf("returned pid: %d \n", pm_hdr->pid);
  printf("returned len: %d \n", pm_hdr->len);

  fd = fopen(filename, "w");
  fwrite(buf, (size_t) pm_hdr->len, 1, fd); 
  fflush(fd);
  fclose(fd);
  
  return 0;
}
