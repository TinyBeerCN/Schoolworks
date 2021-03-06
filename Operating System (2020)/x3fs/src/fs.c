#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "fs.h"

int fd = -1;

sb_t *sb = NULL;
bid_t *fat = NULL;
dir_t *cur_dir = NULL;
dir_t *tmp_dir = NULL;

of_t ofs[MAX_FD] = {0};

int fs_loadfrom(const char *filename)
{
    int retval = -1;

    fd = open(filename, O_RDWR);
    struct stat stat_buf;
    if (stat(filename, &stat_buf) < 0)
    {
        fprintf(stderr, "error when stat: %s\n", strerror(-errno));
        retval = -errno;
        goto out;
    }
    if ((int)stat_buf.st_size < sizeof(blk_t))
    {
        report_error("short superblock block!");
    }

    blk_t buf;
    retval = read(fd, &buf, sizeof(blk_t));

    sb_t *disk_sb = (sb_t *)&buf;
    if (!sb_check_magic(disk_sb))
    {
        report_error("Magic number of superblock didn't match");
    }

    // load superblock to memory
    sb = malloc(sizeof(sb_t));
    memcpy(sb, disk_sb, sizeof(sb_t));

    if ((int)stat_buf.st_size < (sb->total_size - sizeof(blk_t)))
    {
        report_error("file too small");
    }

    // load fat1 & fat2 to memory
    fat = malloc(sizeof(blk_t) * sb->fat_block_num);
    read(fd, fat, sizeof(blk_t) * sb->fat_block_num);
    read(fd, fat, sizeof(blk_t) * sb->fat_block_num);

    // load root dir to current dir
    cur_dir = malloc(sizeof(blk_t));
    read(fd, cur_dir, sizeof(blk_t));
    if (!dir_check_magic(cur_dir))
    {
        report_error("Magic number of directory didn't match");
    }

    // create tmp buffer
    tmp_dir = malloc(sizeof(blk_t));

out:
    return retval;
}

int fs_writeto(const char *filename)
{
    // close opened fd
    for (int i = 0; i < MAX_FD; ++i)
    {
        if (ofs[i].not_empty)
            fs_close(i);
    }

    // set fd offset to zero
    lseek(fd, 0, SEEK_SET);

    blk_t *blk = calloc(1, sizeof(blk_t));
    // superblock
    memcpy(blk, sb, sizeof(sb_t));
    write(fd, blk, sizeof(blk_t));

    blk = realloc(blk, sizeof(blk_t) * sb->fat_block_num);
    memcpy(blk, fat, sizeof(blk_t) * sb->fat_block_num);
    // fat1
    write(fd, blk, sizeof(blk_t) * sb->fat_block_num);
    // fat2
    write(fd, blk, sizeof(blk_t) * sb->fat_block_num);

    // release
    close(fd);
    free(blk);
    free(sb);
    free(fat);
    free(cur_dir);
    free(tmp_dir);

    return 0;
}

int fs_exit()
{
    // save data to file before exit
    fs_writeto(NULL);
    exit(0);
}
