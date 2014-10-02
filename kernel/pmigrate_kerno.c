
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

/* Copy the process data to userspace */

#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/proc.h> 
#include <sys/module.h> 
#include <sys/sysproto.h>
#include <sys/sysent.h> 
#include <sys/kernel.h> 
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>
#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include "pmigrate.h"
#include "pmigrate_kerno.h"

#include <amd64/include/pcb.h>
#include <amd64/include/fpu.h>
#include <amd64/include/tss.h>
#include <amd64/include/segments.h>
#include <amd64/include/frame.h>

#define DEBUG 1

/* 
Copy the process data to userspace address addr. Return size of data copied. 
*/
size_t cpprocdata(int pid, char *addr)
{
  size_t offset = 0;
  size_t len;

  struct pm_header header;

  /* we skip sizeof(struct pm_header) bytes for now, but we'll write the header in the end */
  offset = sizeof(struct pm_header); 

#ifdef DEBUG
  printf("copying process %d data to userspace \n", pid); 
#endif

  len = cpproc(pid, addr+offset);
  header.procoffset = offset;
  offset += len;

  len = cptds(pid, addr+offset, &(header.ntds));
  header.tdsoffset = offset;
  offset += len;

  header.mmapsoffset = 0;

#ifdef DEBUG
  printf("writing header \n");
#endif

  header.pid = pid;
  header.len = offset;

  copyout(&header, addr, sizeof(header));

#ifdef DEBUG
  printf("copied %d bytes to userspace \n", (int) offset);
#endif

  return offset;
}

/*
  Copy the struct proc associated with the process to userspace address addr.
*/
size_t cpproc(int pid, char *addr)
{
#ifdef DEBUG
  printf("copying process %d pcb to userspace address %p \n", pid, addr); 
#endif

  size_t offset = 0;

  struct proc *p;

  sx_xlock(&allproc_lock);

  FOREACH_PROC_IN_SYSTEM(p)
  {
    PROC_LOCK(p);
    if (p->p_pid == pid)
      {
	copyout(p, addr, sizeof(struct proc));

	offset = sizeof(struct proc);	  
      } 
    PROC_UNLOCK(p);
  }
  sx_unlock(&allproc_lock);

  return offset;
}

/*
  Copy the multiple struct td associated with the process to userspace address addr. Return the size of the data copied and the number of threads copied.
*/
size_t cptds(int pid, char *addr, int *ntds)
{

#ifdef DEBUG
  printf("copying threads to userspace address %p \n", addr); 
  printf(" thread");
#endif

  size_t offset = 0;
  int n = 0; /* number of threads */

  struct proc *p;
  struct thread *td;

  sx_xlock(&allproc_lock);

  FOREACH_PROC_IN_SYSTEM(p)
  {
    PROC_LOCK(p);
    if (p->p_pid == pid)
      {
	FOREACH_THREAD_IN_PROC(p, td)
	  {
	    copyout(td, addr+offset, sizeof(struct thread));
	    offset += sizeof(struct thread);	  

	    copyout(td->td_pcb, addr+offset, sizeof(struct pcb));
	    offset += sizeof(struct pcb);	  

	    n++;

#ifdef DEBUG
	    printf(" %d", n);
#endif
	  }
      } 
    PROC_UNLOCK(p);
  }
  sx_unlock(&allproc_lock);

  *ntds = n;

#ifdef DEBUG
  printf(" \n");
#endif

  return offset;
}

void forcecoredump(int pid)
{
  struct proc *p;
  struct thread *td;

  sx_xlock(&allproc_lock);

  FOREACH_PROC_IN_SYSTEM(p)
  {
    PROC_LOCK(p);
    if (p->p_pid == pid)
      {
	td = FIRST_THREAD_IN_PROC(p);
	  {
	    //	    sigexit(td, SIGKILL);
	  }
      } 
    PROC_UNLOCK(p);
  }
  sx_unlock(&allproc_lock);
}
