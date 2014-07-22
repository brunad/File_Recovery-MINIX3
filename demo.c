#include <stdio.h>
#include <stdlib.h>

#include <repair.h>

int main(){
    int * r=calloc(1017088,4);

	int x=inodewalker(r);

	//printf("x: %d\n",x);
	//printf("r: %d  %d  %d\n",r[0],r[1],r[2]);

	FILE * file=fopen("blocks.txt","w");

	int i=0;
	while(r[i]!=0){
		i++;
	}
    printf("number of blocks referenced by inodes: %d",i);
    i=0;
	while(r[i]!=0){
        fprintf(file,"%d",r[i]);
		i++;
	}

	fclose(file);
	free(r);
	printf("Blocks referenced by inodes saved in blocks.txt\n");

    int * e=calloc(1017088,4);
	x=zonemapwalker(e);
	//printf("x: %d\n",x);
	//printf("e: %d  %d  %d\n",e[0],e[1],e[2]);

	FILE * file2=fopen("blocks2.txt","w");

	int j=0;
	while(e[j]!=0){
		j++;
	}
    printf("number of blocks referenced by zonemap: %d",i-j);
    printf("difference with inodewalker:");
    j=0;
	while(e[j]!=0){
        fprintf(file,"%d",e[j]);
		j++;
	}

	fclose(file2);
	free(e);
	printf("Blocks referenced by zonemap saved in blocks2.txt\n");

}













