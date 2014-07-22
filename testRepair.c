#include <stdio.h>
#include <stdlib.h>

#include <repair.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>


int main(){

    printf("Inode to be repaired: ");
    int n;
    scanf("%d",&n);
    int x=inodefixer(n);

    printf("Inode fixed...\n");

	return 0;

}
