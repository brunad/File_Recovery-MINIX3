#include "fs.h"
#include <assert.h>
#include <minix/vfsif.h>
#include <minix/bdev.h>
#include "inode.h"
#include "clean.h"
#include <stdlib.h>



int * block_numbers;
int * lost_blocks;
int broken_inodeNumber;


/*===========================================================================*
 *				fs_sync					     *
 *===========================================================================*/
int fs_sync()
{
/* Perform the sync() system call.  Flush all the tables.
 * The order in which the various tables are flushed is critical.  The
 * blocks must be flushed last, since rw_inode() leaves its results in
 * the block cache.
 */
  struct inode *rip;

  assert(lmfs_nr_bufs() > 0);

  /* Write all the dirty inodes to the disk. */
  for(rip = &inode[0]; rip < &inode[NR_INODES]; rip++)
	  if(rip->i_count > 0 && IN_ISDIRTY(rip)) rw_inode(rip, WRITING);

  /* Write all the dirty blocks to the disk. */
  lmfs_flushall();

  return(OK);		/* sync() can't fail */
}


/*===========================================================================*
 *				fs_flush				     *
 *===========================================================================*/
int fs_flush()
{
/* Flush the blocks of a device from the cache after writing any dirty blocks
 * to disk.
 */
  dev_t dev = (dev_t) fs_m_in.REQ_DEV;
  if(dev == fs_dev) return(EBUSY);

  lmfs_flushall();
  lmfs_invalidate(dev);

  return(OK);
}


/*===========================================================================*
 *				fs_new_driver				     *
 *===========================================================================*/
int fs_new_driver(void)
{
/* Set a new driver endpoint for this device. */
  dev_t dev;
  cp_grant_id_t label_gid;
  size_t label_len;
  char label[sizeof(fs_dev_label)];
  int r;

  dev = (dev_t) fs_m_in.REQ_DEV;
  label_gid = (cp_grant_id_t) fs_m_in.REQ_GRANT;
  label_len = (size_t) fs_m_in.REQ_PATH_LEN;

  if (label_len > sizeof(label))
	return(EINVAL);

  r = sys_safecopyfrom(fs_m_in.m_source, label_gid, (vir_bytes) 0,
	(vir_bytes) label, label_len);

  if (r != OK) {
	printf("MFS: fs_new_driver safecopyfrom failed (%d)\n", r);
	return(EINVAL);
  }

  bdev_driver(dev, label);

  return(OK);
}



int fs_zonemapwalker(){
	fprintf(stderr, "fs_zonemapwalker\n");

	struct super_block* sp = get_super(fs_m_in.REQ_DEV);

	fprintf(stderr, "usable inodes on the minor device: %d\n", sp->s_ninodes);
	fprintf(stderr, "total device size: %d\n", sp->s_nzones);
	fprintf(stderr, "number of blocks in inode map: %d\n", sp->s_imap_blocks);
	fprintf(stderr, "number of blocks in zone map: %d\n", sp->s_zmap_blocks);
	fprintf(stderr, "number of zones: %d\n", sp->s_zones);
	fprintf(stderr, "block size: %d\n", sp->s_block_size);
	fprintf(stderr, "number of first data zone: %d\n", sp->s_firstdatazone);
	fprintf(stderr, "direct zones in an inode: %d\n", sp->s_ndzones);
	fprintf(stderr, "indirect zones per indirect block: %d\n", sp->s_nindirs);
	fprintf(stderr, "inodes below this bit number are in use: %d\n", sp->s_isearch);
	fprintf(stderr, "zones below this bit number are in use: %d\n", sp->s_zsearch);

    	lost_blocks=calloc(sp->s_zones,4);
	int index=0;
	int t=0;
	int found=0;

	int k;
	for(k=0;k<sp->s_zmap_blocks;k++){
		struct buf* buffer = get_block(fs_m_in.REQ_DEV, sp->s_imap_blocks+2+k, 0);
		char * tmp=(char*)buffer->data;
		int i;
		for(i=1;i<32768;i++){
			if((tmp[i/8] & (1 << (i%8) )) != 0 ){
				index=0;
				found=0;
				while(block_numbers[index]!=0){
					if(block_numbers[index]==(32768*k+i+(sp->s_firstdatazone)-1))found=1;
					index++;
				}
				if(found==0){
					lost_blocks[t]=(32768*k+i+(sp->s_firstdatazone)-1);
					t++;
				}
			}
		}
		put_block(buffer,0);
	}



	printf("test: %d, %d, %d\n",block_numbers[0],block_numbers[1],block_numbers[2]);
	printf("index: %d	t: %d\n",index,t);
	printf("lost: %ld %d %d %d\n",(long)lost_blocks,lost_blocks[0],lost_blocks[1],lost_blocks[2]);

	fs_m_out.RES_DEV=(long)lost_blocks;
	fs_m_out.RES_NBYTES=t*4;


	return 0;
}


int fs_inodewalker(){
	fprintf(stderr, "fs_inodewalker\n");

	struct super_block* sp = get_super(fs_m_in.REQ_DEV);

	fprintf(stderr, "usable inodes on the minor device: %d\n", sp->s_ninodes);
	fprintf(stderr, "total device size: %d\n", sp->s_nzones);
	fprintf(stderr, "number of blocks in inode map: %d\n", sp->s_imap_blocks);
	fprintf(stderr, "number of blocks in zone map: %d\n", sp->s_zmap_blocks);
	fprintf(stderr, "number of zones: %d\n", sp->s_zones);
	fprintf(stderr, "block size: %d\n", sp->s_block_size);
	fprintf(stderr, "number of first data zone: %d\n", sp->s_firstdatazone);
	fprintf(stderr, "direct zones in an inode: %d\n", sp->s_ndzones);
	fprintf(stderr, "indirect zones per indirect block: %d\n", sp->s_nindirs);
	fprintf(stderr, "inodes below this bit number are in use: %d\n", sp->s_isearch);
	fprintf(stderr, "zones below this bit number are in use: %d\n", sp->s_zsearch);

	block_numbers=calloc(sp->s_zones*4,1);
	int index=0;

	int k;
	for(k=0;k<sp->s_imap_blocks;k++){
		struct buf* buffer = get_block(fs_m_in.REQ_DEV, 2+k, 0);
		char * tmp=(char*)buffer->data;
		int i;
		for(i=1;i<32768;i++){
			if((tmp[i/8] & (1 << (i%8) )) != 0 ){
			    struct inode * ino = get_inode(fs_m_in.REQ_DEV,32768*k+i);
                int j;
                for(j=0;j<9;j++){
                    if(ino->i_zone[j]!=0){
                        block_numbers[index] = ino->i_zone[j];
                        index++;
                    }
                }
                if(ino->i_zone[7]!=0){
                    struct buf* b2=getblock(fs_m_in.REQ_DEV, ino->i_zone[7], 0);
                    int * tmp2=(int*)b2->data;
                    j=0;
                    while(tmp2[j]!=0){
                        block_numbers[index] = tmp2[j];
                        index++;
                        j++;
                    }
                }
                put_block(b2,0);
			    put_inode(ino);
			}
		}
		put_block(buffer,0);
	}



	printf("test: %d, %d, %d\n",block_numbers[0],block_numbers[1],block_numbers[2]);
	printf("index: %d\n",index);

	fs_m_out.RES_DEV=(int)block_numbers;
	fs_m_out.RES_NBYTES=index*4;

	return 0;
}




int fs_inodedamage(){
	fprintf(stderr, "fs_inodedamage\n");

	//struct super_block * sp=get_super(fs_m_in.REQ_DEV);

	block_numbers=calloc(9,4);
	int index=0;

    broken_inodeNumber = fs_m_in.REQ_INODE_NR;

    struct inode * ino = get_inode(fs_m_in.REQ_DEV,broken_inodeNumber);

    int j;
    for(j=0;j<9;j++){
        if(ino->i_zone[j]!=0){
            block_numbers[index] = ino->i_zone[j];
            ino->i_zone[j] = 0;
            index++;
        }
    }
    put_inode(ino);

	printf("test: %d, %d, %d\n",block_numbers[0],block_numbers[1],block_numbers[2]);
	printf("index: %d\n",index);

	fs_m_out.RES_DEV=(int)block_numbers;
	fs_m_out.RES_NBYTES=index*4;


	return 0;
}




int fs_inodefixer(){
	fprintf(stderr, "fs_inodeFixer\n");

	int index=0;

	printf("test:  %d  %d  \n",block_numbers[0],lost_blocks[0]);
	broken_inodeNumber = fs_m_in.REQ_INODE_NR;
	if(broken_inodeNumber==0)return 0;

    struct inode * ino = get_inode(fs_m_in.REQ_DEV,broken_inodeNumber);

    int j;
    for(j=0;j<7;j++){
        if(ino->i_zone[j]==0){
            ino->i_zone[j]= lost_blocks[index] ;
            index++;
        }
    }
    if(lost_blocks[8]!=0) ino->i_zone[7]=lost_blocks[8];
    if(lost_blocks[1033]!=0) ino->i_zone[8]=lost_blocks[1033];
    put_inode(ino);

	printf("done\n");
	free(lost_blocks);
	free(block_numbers);

	return 0;
}

