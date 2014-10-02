
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

#include "pmigrate.h"
#include "pmigrate_kerni.h"
#include "pmigrate_kerno.h"

/* The system call's arguments. */ 
struct sc_save_args 
{ 
  struct pm_data_ref* ppmdata; 
}; 

struct sc_load_args 
{ 
  struct pm_data_ref* ppmdata; 
}; 

// static int coredump(struct thread* td);

/* The system call function. */ 
static int sc_save(struct thread *td, void *syscall_args) 
{ 
  printf("sc_save called \n");

  struct sc_save_args *uap; 
  uap = (struct sc_save_args *)syscall_args; 

  cpprocdata(uap->ppmdata->pid, uap->ppmdata->addr);

  /* force a core dump */
  //  coredump(td);
  forcecoredump(uap->ppmdata->pid);

  return 0;
}

static int sc_load(struct thread *td, void *syscall_args) 
{ 
  printf("sc_load called \n");

  struct sc_load_args *uap; 
  uap = (struct sc_load_args *)syscall_args; 

  struct pm_data_ref ppmdata;

  copyin(uap->ppmdata, &ppmdata,  sizeof(struct pm_data_ref));

  uprintf("uap->ppmdata->pid %d \n", uap->ppmdata->pid);
  uprintf("uap->ppmdata->execfile %s \n", uap->ppmdata->execfile);
  uprintf("uap->ppmdata->corefile %s \n", uap->ppmdata->corefile);

  loadpcb(uap->ppmdata);
  createmap(uap->ppmdata->pid, uap->ppmdata->execfile, uap->ppmdata->corefile);

  return 0;
}


/* The sysent for the new system call. */ 
static struct sysent sc_save_sysent = { 
  1, /* number of arguments */ 
  sc_save /* implementing function */ 
}; 

static struct sysent sc_load_sysent = { 
  1, /* number of arguments */ 
  sc_load /* implementing function */ 
}; 

/* The offset in sysent[] where the system call is to be allocated. */ 
static int offset_save = NO_SYSCALL;
static int offset_load = NO_SYSCALL;

/* The function called at load/unload. */ 
static int module_save_load(struct module *module, int cmd, void *arg) 
{ 
  int error = 0;
  switch (cmd) 
    { 
    case MOD_LOAD: 
      uprintf("System call p_save loaded at offset %d. \n", offset_save); 
      break; 
    case MOD_UNLOAD: 
      uprintf("System call p_save unloaded from offset %d. \n", offset_save); 
      break; 
    default: 
      error = EOPNOTSUPP; 
      break; 
    } 
  return(error); 
}

/* The function called at load/unload. */ 
static int module_load_load(struct module *module, int cmd, void *arg) 
{ 
  int error = 0;
  switch (cmd) 
    { 
    case MOD_LOAD: 
      uprintf("System call p_load loaded at offset %d. \n", offset_load); 
      break; 
    case MOD_UNLOAD: 
      uprintf("System call p_load unloaded from offset %d. \n", offset_load); 
      break; 
    default: 
      error = EOPNOTSUPP; 
      break; 
    } 
  return(error); 
}

SYSCALL_MODULE(sc_save, &offset_save, &sc_save_sysent, module_save_load, NULL);
SYSCALL_MODULE(sc_load, &offset_load, &sc_load_sysent, module_load_load, NULL);
