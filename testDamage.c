#include <stdio.h>
#include <stdlib.h>

#include <repair.h>

#include <damage.h>

int main(){

    int d;
    printf("Inode to damage: ");
    scanf("%d",&d);

	int * r=calloc(9,4);
	int x=inodedamage(r,d);

	printf("x: %d\n",x);
	printf("r: %d  %d  %d\n",r[0],r[1],r[2]);

	FILE * file=fopen("Dblocks.txt","w");

	int i=0;
	while(r[i]!=0&&i<9){
        i++;
	}
    fprintf(file,"number of block references erased: %d\n",i);
    i=0;
	while(r[i]!=0&&i<9){
        fprintf(file,"%d\n",r[i]);
        i++;
	}

	fclose(file);
	free(r);

	printf("Block numbers of damaged file saved in Dblocks.txt\n");

	return 0;

}
