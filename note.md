<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
#ext2文件系统学习笔记
##文件系统建立(blocksize=1k)
* 新建一个ext2文件系统
* dd if=/dev/sda4 of=extfs bs=1k count=64000
* losetup /dev/loop0 ext2fs
> loop 设备是一种伪设备（pseudo-device），或者也可以说是仿真设备。它能使我们像块设备一样访问一个文件
* mount -t ext2 /dev/loop0 /mnt/extfs
* cd /mnt/extfs;mount

##超级块
* 超级块的backup 保存在第0 , 3^n^,  5^n^ , 7^n^ 个块组之中
* 超级块信息
>struct ext2_super_block {
      __u32   s_inodes_count;
      __u32   s_blocks_count;
      __u32   s_r_blocks_count;
      __u32   s_free_blocks_count;
      __u32   s_free_inodes_count;
      __u32   s_first_data_block;
      __u32   s_log_block_size;
      __u32   s_dummy3[7];
      unsigned char s_magic[2];
      __u16   s_state;
    ...
    }

(用工具获得超级块信息`dumpe2fs /dev/loop0 `)
   第一个块是启动块，从0x400-0x800是超级块。

##组描述符
####*所有组的组描述符以数组形式存放在k个块当中，这k个块存放了所有组的组描述符*
>
struct ext2_group_desc
    {
      __u32 bg_block_bitmap; /* Blocks bitmap block */
      __u32 bg_inode_bitmap; /* Inodes bitmap block */
      __u32 bg_inode_table; /* Inodes table block */
      __u16 bg_free_blocks_count; /* Free blocks count */
      __u16 bg_free_inodes_count; /* Free inodes count */
      __u16 bg_used_dirs_count; /* Directories count */
      __u16 bg_flags;
      __u32 bg_exclude_bitmap_lo;/* Exclude bitmap for snapshots */
      __u16 bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap)LSB */
      __u16 bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap)LSB */
      __u16 bg_itable_unused; /* Unused inodes count */
      __u16 bg_checksum; /* crc16(s_uuid+grouo_num+group_desc)*/
    };//total size=0x20

##Block size
>每一个块组用一个块存放该块组的bitmap，所以最多块数=8*blocksize

块大小|最多块数|块组大小
-|-|-
1k|8k|8M
2k|16k|32M
4k|32k|128M

>Inode中有15个指针指向块12个为直接指针，13为间接寻址1的间接指针，14位间接寻址2。。。

块大小|直接寻址|间接寻址1|间接寻址2|间接寻址3
-|-|-|-|-
1k|12k|0x100k|0x10000k|0x1000000k
2k|24k|4*|8*|16*
4k|48k|4*|8*|16*

###Bytes per Inode
期望的每个文件的大小（一个块组可用仅当Inode和block都没有用完的时候）

##分配Inode
* 对于挂载点下第一级目录：
> 1 存在空闲inode
2 目录数要少于上一个块组备选项,
3 块组freeinode数不小于average
4 块组free block数不小于average

* 对于非第一级目录：
> 1 尽量和父目录在同一个块组下
2 debt max_debt = 255

##解析目录
命令查看文件所在块
>ll -ai;
  debugfs /dev/loop0;
  stat <num>

###手工解析目录
####查找所在块
* 确定所在group
* 从group_desc中定位inode table
* inode_table大小为128byte，(inode-first inode of this block)*128 + inode_table
* i_block偏移量为40
>    struct ext2_dir_entry_2 {
        __le32    inode;            /* Inode number */
        __le16    rec_len;        /* Directory entry length */
        __u8    name_len;        /* Name length */
        __u8    file_type;
        char    name[];            /* File name, up to EXT2_NAME_LEN */
    };
>     enum {
        EXT2_FT_UNKNOWN        = 0,
        EXT2_FT_REG_FILE    = 1,
        EXT2_FT_DIR        = 2,
        EXT2_FT_CHRDEV        = 3,
        EXT2_FT_BLKDEV        = 4,
        EXT2_FT_FIFO        = 5,
        EXT2_FT_SOCK        = 6,
        EXT2_FT_SYMLINK        = 7,
        EXT2_FT_MAX
    };

![](http://blog.chinaunix.net/attachment/201208/11/24774106_13446861649HG4.jpg) 

注意：
1 文件名按4个字节对齐，不足的补0.
2 从当前条目的rec_len的末尾到下一个**合法（未删除）**条目rec_len末尾的偏移量

##文件删除
文件删除时将文件inode改成0，last rec_len=last rec_len+this rec_len（即上文中“合法”）

##文件恢复
命令恢复
>1 通过debugfs的lsdel命令找到你删掉文件的inode。
2 通过debugfs 找到inode对应的block位置
3 dd 读取对应block的内容到临时文件。

***
参考：bean 《深入ext2 文件系统》












