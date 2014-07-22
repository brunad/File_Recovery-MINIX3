#include "fs.h"
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <minix/vfsif.h>
#include <minix/const.h>
#include <minix/endpoint.h>
#include <stdlib.h>
#include <stdio.h>
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/vm.h>
#include <string.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include <string.h>

#include "fproc.h"
#include "vmnt.h"
#include "vnode.h"
#include "path.h"
#include "param.h"

#include <damage.h>

int RC_CODE;

int do_inodedamage(){
    printf("successfully called vfs inodedamage...\n");

    struct vmnt *vmp;

    char * dest=(char *)m_in.m1_i1;
    endpoint_t w=m_in.m_source;
    printf("dest, entering vfs: %d\n",(int)dest);

    for (vmp = &vmnt[0]; vmp < &vmnt[NR_MNTS]; ++vmp) {
    	if ( strcmp("/home", vmp->m_mount_path) == 0 ) {
            printf("found home directory");
            printf(" number of devices: %d", vmp->m_dev);
            message m;
            m.m_type = REQ_INODEDAMAGE;
            m.REQ_DEV = vmp->m_dev;
            m.REQ_INODE_NR = m_in.m1_i2;

            printf("entering mfs");
            RC_CODE = fs_sendrec(vmp->m_fs_e, &m);

            int size=m.RES_NBYTES;

            int * blocks=malloc(size);
            if(sys_datacopy(m.m_source, (vir_bytes)m.RES_DEV, SELF, (vir_bytes)blocks, size)==OK)printf("Copy1 ok\n");
            printf("test copy1: %ld %d  %d  %d\n",m.RES_DEV,blocks[0],blocks[1],blocks[2]);

            if(sys_datacopy(SELF, (vir_bytes)blocks, w , (vir_bytes)dest, size)==OK)printf("copy2 OK\n");

        }
    }


    return 0;
}


