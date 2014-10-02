
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
#include <sys/fcntl.h>
#include <sys/namei.h>
#include <sys/vnode.h>

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_extern.h>

#include "pmigrate_kerni.h"
#include "imgact_elfcore.h"

#include <amd64/include/pcb.h>
#include <amd64/include/fpu.h>
#include <amd64/include/tss.h>
#include <amd64/include/segments.h>
#include <amd64/include/frame.h>

#define DEBUG 1

/* fill the memory with the contents of the core file */
void createmap(pid_t pid, char* execfile, char* corefile)
{
#ifdef DEBUG
  printf("Adding memory map zone to pid %d \n", (int) pid);
#endif
  vm_map_t vmmap;
  struct proc *p;
  
  sx_xlock(&allproc_lock);
  FOREACH_PROC_IN_SYSTEM(p)
  {
    PROC_LOCK(p);
    if (p->p_pid == pid)
      {
	struct thread *td;
	td = FIRST_THREAD_IN_PROC(p);
	
	{
	  /* Clear the vm map */
	  vmmap = &p->p_vmspace->vm_map;
	  vm_map_lock(vmmap);
	  vm_map_remove(vmmap, vm_map_min(vmmap), vm_map_max(vmmap));
	  vm_map_unlock(vmmap);

	  u_long addr = (u_long) NULL;
	  u_long entry = (u_long) NULL;

#ifdef DEBUG
	  printf("corefile: %s \n", corefile);
#endif

	  int res;
	  res = elf64_load_file_core(p, execfile, &addr, &entry, 4096, 1, 0);

#ifdef DEBUG
	  printf("load_file_core returned: %d \n", res );
#endif

	  res = elf64_load_file_core(p, corefile, &addr, &entry, 4096, 0, 1);

#ifdef DEBUG
	  printf("load_file_core returned: %d \n", res );
#endif
	}
      } 
    PROC_UNLOCK(p);
  }
  sx_unlock(&allproc_lock);
};

/* load the registers */
void loadpcb(struct pm_data_ref *ppmdata)
{
  Elf_Ehdr *hdr;
  Elf_Phdr *phdr;
  Elf_Note *pnote;
  struct prstatus status;

  int pid = ppmdata->pid;

  /* go through the start of the ELF header */

  char *data = ppmdata->elfaddr;
  hdr = (Elf_Ehdr *) data;
  phdr = (Elf_Phdr*) (data + hdr->e_phoff);
  pnote = (Elf_Note *) (data + phdr->p_offset);
  char* pnote_start = (char*) pnote;
  
  /* parse the notes */

  while (((char*) pnote - pnote_start) <  phdr->p_filesz)
    {
      int len = sizeof(Elf_Note) + pnote->n_namesz + pnote->n_descsz;
      char* desc = ((char*) pnote) + sizeof(Elf_Note) + pnote->n_namesz;

      if (pnote->n_type == NT_PRSTATUS)
	{
#ifdef DEBUG
	  printf("found NT_PRSTATUS \n");
#endif
	  memcpy(&status, desc, sizeof(struct prstatus));
#ifdef DEBUG
	  printf("status->pr_reg->r_rip %lx \n", status.pr_reg.r_rip);
#endif
	}

      pnote = (Elf_Note*)((char*) pnote + len); /* go to the next note */ 
    }  

#ifdef DEBUG
  uprintf("pid: %d \n", pid);
  printf("pid: %d \n", pid);
#endif

  /* find the process and do the copy */

  struct proc *p;

  sx_xlock(&allproc_lock);
  FOREACH_PROC_IN_SYSTEM(p)
  {
    PROC_LOCK(p);
    if (p->p_pid == pid)
      {
	struct thread *td;
	td = FIRST_THREAD_IN_PROC(p);
	{
	  set_regs(td, &status.pr_reg);
	}
      } 
    PROC_UNLOCK(p);
  }
  sx_unlock(&allproc_lock);
}
