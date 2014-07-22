#include <lib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>


int inodewalker(int * r){
    message m;
    m.m1_i1=r;
    printf("avant syscall\n");
    int x=_syscall(VFS_PROC_NR,108,&m);
    printf("apres syscall\n");
    return x;
}


int zonemapwalker(int * r){
    message m;
    m.m1_i1=r;
    m.m1_i2=0;
    printf("avant syscall\n");
    int x=_syscall(VFS_PROC_NR,109,&m);
    printf("apres syscall\n");
    return x;
}


int inodefixer(int n){
    message m;
    m.m1_i1=NULL;
    int x=_syscall(VFS_PROC_NR,108,&m);

    message m2;
    m2.m1_i1=NULL;
    m2.m1_i2=n;
    int x=_syscall(VFS_PROC_NR,109,&m2)
    return x;
}





