
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
#include <sys/param.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define FDREPLICATE
char* pid;
char buf[MAXPATHLEN];
char fdfile[4096];

int main(int argn, char** argv)
{

  if (argn > 0)
    {
      pid = argv[1];
    }

#ifdef FDREPLICATE
  sprintf(buf, "%s/lsfd.dat", pid);
  printf("path to lsfd file: %s \n", buf);
  
  /* create the fds */
  if (pid != NULL)
    {
      int fdtemp = open(buf, O_RDONLY);
      read(fdtemp, fdfile, sizeof(fdfile));
      close(fdtemp);

      char *line;
      for (line = strtok(fdfile, "\n"); line; line = strtok(NULL, "\n"))
	{
	  char ftype[20];
	  
	  sscanf(line, "%s", ftype);
	  
	  printf("line: %s \n", line);
	  
	  printf(" type: %s \n", ftype);
	  
	  switch (ftype[0])
	    {
	    case 'F':
	      {
		int fd;
		char access;
		char offset_val[20];
		char filename[MAXPATHLEN];
		long int offset;

		sscanf(line, "%s %d %c %s %s", ftype, &fd, &access, offset_val, filename);
		
		offset = strtol(offset_val, NULL, 10);
		printf(" fd: %d \n", fd);
		printf(" access: %c \n", access);
		printf(" offset: %ld \n", offset);
		printf(" file: %s \n", filename);
		
		int flags = 0;
		switch (access)
		  {
		  case 'u': flags = O_RDWR; break;
		  case 'r': flags = O_RDONLY; break;
		  case 'w': flags = O_WRONLY; break;
		  }
		fd = open(filename, flags);
		break;
	      } /* switch (access)   */
	    case 'U':
	      {
		char address[40];
		int port;
		
		sscanf(line, "%s %s %d", ftype, address, &port);
		int fd = socket(PF_INET, SOCK_DGRAM, 0);

		struct sockaddr_in name;
		name.sin_family = AF_INET;
		name.sin_port = htons(port);
		inet_pton(AF_INET, address, &name.sin_addr);

		printf(" fd: %d \n", fd);
		printf(" address: %s \n", address);
		printf(" pr: %d \n", port);

		connect(fd, (struct sockaddr*) &name, sizeof(name));	       		

		break;
	      }
	    }   /* switch (ftype[0]) */	    
	}       /* for (line =       */
    }           /* if (pid != NULL)  */
#endif
  
  printf("loaded \n");

  while (1<2);
}
