#include <sys/stat.h>
#include "cpu.h"
#include "exec/helper-proto.h"
#include "exec/softmmu-semi.h"

typedef enum UHI_Op {
    UHI_exit = 1,
    UHI_open = 2,
    UHI_close = 3,
    UHI_read = 4,
    UHI_write = 5,
    UHI_lseek = 6,
    UHI_unlink = 7,
    UHI_fstat = 8,
    UHI_argc = 9,
    UHI_argnlen = 10,
    UHI_argn = 11,
    UHI_heapinfo = 12,
    UHI_plog = 13,
    UHI_assert = 14,
    UHI_exception = 15,
    UHI_findfirst = 16,
    UHI_findnext = 17,
    UHI_findclose = 18,
    UHI_pread = 19,
    UHI_pwrite = 20,
    UHI_yield = 21,
    UHI_link = 22
} UHI_Op;

typedef struct UHI_stat {
    short uhi_st_dev;
    unsigned short uhi_st_ino;
    unsigned int uhi_st_mode;
    unsigned short uhi_st_nlink;
    unsigned short uhi_st_uid;
    unsigned short uhi_st_gid;
    short uhi_st_rdev;
    unsigned long long uhi_st_size;
    unsigned long long uhi_st_atime;
    unsigned long long uhi_st_spare1;
    unsigned long long uhi_st_mtime;
    unsigned long long uhi_st_spare2;
    unsigned long long uhi_st_ctime;
    unsigned long long uhi_st_spare3;
    unsigned long long uhi_st_blksize;
    unsigned long long uhi_st_blocks;
    unsigned long long uhi_st_spare4[2];
} UHI_stat;

enum UHIOpen_flags {
    UHIOpen_RDONLY = 0x0,
    UHIOpen_WRONLY = 0x1,
    UHIOpen_RDWR   = 0x2,
    UHIOpen_APPEND = 0x8,
    UHIOpen_CREAT  = 0x200,
    UHIOpen_TRUNC  = 0x400,
    UHIOpen_EXCL   = 0x800
};

#ifdef CONFIG_USER_ONLY
static const char **semihosting_argv;
static int semihosting_argc;
static const char *semihosting_root;
#else
extern const char **semihosting_argv;
extern int semihosting_argc;
extern const char *semihosting_root;
#endif

static int put_root_in_front(const char *jr, char *buf)
{
    const char *separator = "/";
    bool is_jr_separator = (jr[strlen(jr) - 1] == separator[0]);
    bool is_path_separator = (buf[0] == separator[0]);
    char *path = g_malloc(strlen(buf) + 1);

    if (!path) {
        return -1;
    }

    strcpy(path, buf);
    strcpy(buf, jr);

    if (is_jr_separator && is_path_separator) {
        /* skip redundant separator */
        strcat(buf, path + 1);
    } else {
        if (!is_jr_separator && !is_path_separator) {
            /* insert missing separator */
            strcat(buf, separator);
        }
        strcat(buf, path);
    }

    g_free(path);
    return 0;
}

static int apply_root(char *buf)
{
    if (!semihosting_root) {
        return 0; /* nothing to do */
    } else if (semihosting_root[0] == '\0') {
        return 0; /* nothing to do */
    } else {
        return put_root_in_front(semihosting_root, buf);
    }
}

static void copy_stat_to_target(CPUMIPSState *env, const struct stat *src,
                                target_ulong vaddr)
{
    hwaddr len = sizeof(struct UHI_stat);
    UHI_stat *dst = lock_user(VERIFY_WRITE, vaddr, len, 0);

    if (dst) {
        dst->uhi_st_dev = tswap16(src->st_dev);
        dst->uhi_st_ino = tswap16(src->st_ino);
        dst->uhi_st_mode = tswap32(src->st_mode);
        dst->uhi_st_nlink = tswap16(src->st_nlink);
        dst->uhi_st_uid = tswap16(src->st_uid);
        dst->uhi_st_gid = tswap16(src->st_gid);
        dst->uhi_st_rdev = tswap16(src->st_rdev);
        dst->uhi_st_size = tswap64(src->st_size);
        dst->uhi_st_atime = tswap64(src->st_atime);
        dst->uhi_st_mtime = tswap64(src->st_mtime);
        dst->uhi_st_ctime = tswap64(src->st_ctime);
        dst->uhi_st_blksize = tswap64(src->st_blksize);
        dst->uhi_st_blocks = tswap64(src->st_blocks);

        unlock_user(dst, vaddr, len);
    }
}

static int get_open_flags(target_ulong target_flags)
{
    int open_flags = 0;

    if (target_flags & UHIOpen_RDWR) {
        open_flags |= O_RDWR;
    } else if (target_flags & UHIOpen_WRONLY) {
        open_flags |= O_WRONLY;
    } else {
        open_flags |= O_RDONLY;
    }

    open_flags |= (target_flags & UHIOpen_APPEND) ? O_APPEND : 0;
    open_flags |= (target_flags & UHIOpen_CREAT)  ? O_CREAT  : 0;
    open_flags |= (target_flags & UHIOpen_TRUNC)  ? O_TRUNC  : 0;
    open_flags |= (target_flags & UHIOpen_EXCL)   ? O_EXCL   : 0;

    return open_flags;
}

static int write_to_file(CPUMIPSState *env, target_ulong fd, target_ulong vaddr,
                         target_ulong len, target_ulong offset)
{
    void *dst = lock_user(VERIFY_READ, vaddr, len, 1);

    if (!dst) {
        return 0;
    } else {
        int num_of_bytes = offset ? pwrite(fd, dst, len, offset)
                                  : write(fd, dst, len);
        unlock_user(dst, vaddr, 0);
        return num_of_bytes;
    }
}

static int read_from_file(CPUMIPSState *env, target_ulong fd,
                          target_ulong vaddr, target_ulong len,
                          target_ulong offset)
{
    void *dst = lock_user(VERIFY_WRITE, vaddr, len, 0);

    if (!dst) {
        return 0;
    } else {
        int num_of_bytes = offset ? pread(fd, dst, len, offset)
                                  : read(fd, dst, len);
        unlock_user(dst, vaddr, len);
        return num_of_bytes;
    }
}

static int copy_argn_to_target(CPUMIPSState *env, int arg_num,
                               target_ulong vaddr)
{
    int strsize;
    char *dst;

    strsize = strlen(semihosting_argv[arg_num]) + 1;
    dst = lock_user(VERIFY_WRITE, vaddr, strsize, 0);

    if (!dst) {
        return -1;
    } else {
        strcpy(dst, semihosting_argv[arg_num]);
        unlock_user(dst, vaddr, strsize);
        return 0;
    }
}

void helper_do_semihosting(CPUMIPSState *env)
{
    const char *opname = "invalid";
    target_ulong *gpr = env->active_tc.gpr;
    const UHI_Op op = gpr[25];
    char *p, *p2;

    qemu_log("UHI(%d): gpr4:(0x%x), gpr5(0x%x), gpr6(0x%x)\n",
             op, (int)gpr[4], (int)gpr[5], (int)gpr[6]);

    switch (op) {
    case UHI_exit:
        qemu_log("UHI(%d): exit(%d)\n", op, (int)gpr[4]);
        exit(gpr[4]);
        break;
    case UHI_open:
        p = lock_user_string(gpr[4]);
        if (!p) {
            gpr[2] = -1;
            gpr[3] = ENAMETOOLONG;
        } else {
            if (!strcmp("/dev/stdin", p)) {
                gpr[2] = 0;
            } else if (!strcmp("/dev/stdout", p)) {
                gpr[2] = 1;
            } else if (!strcmp("/dev/stderr", p)) {
                gpr[2] = 2;
            } else {
                apply_root(p);
                gpr[2] = open(p, get_open_flags(gpr[5]), gpr[6]);
                gpr[3] = errno;
            }
            unlock_user(p, gpr[4], 0);
        }
        break;
    case UHI_close:
        opname = "close";
        if (gpr[4] >= 3) {
            gpr[2] = close(gpr[4]);
        } else {
            gpr[2] = 0; /* ignore closing stdin/stdout/stderr */
        }
        gpr[3] = errno;
        break;
    case UHI_read:
        opname = "read";
        gpr[2] = read_from_file(env, gpr[4], gpr[5], gpr[6], 0);
        gpr[3] = errno;
        break;
    case UHI_write:
        opname = "write";
        gpr[2] = write_to_file(env, gpr[4], gpr[5], gpr[6], 0);
        gpr[3] = errno;
        break;
    case UHI_lseek:
        opname = "lseek";
        gpr[2] = lseek(gpr[4], gpr[5], gpr[6]);
        gpr[3] = errno;
        break;
    case UHI_unlink:
        opname = "unlink";
        p = lock_user_string(gpr[4]);
        if (!p) {
            gpr[2] = -1;
            gpr[3] = ENAMETOOLONG;
        } else {
            apply_root(p);
            gpr[2] = remove(p);
            gpr[3] = errno;
            unlock_user(p, gpr[4], 0);
        }
        break;
    case UHI_fstat:
        opname = "fstat";
        {
            struct stat sbuf;
            memset(&sbuf, 0, sizeof(sbuf));
            gpr[2] = fstat(gpr[4], &sbuf);
            gpr[3] = errno;
            copy_stat_to_target(env, &sbuf, gpr[5]);
        }
        break;
    case UHI_argc:
        opname = "argc";
        gpr[2] = semihosting_argc;
        break;
    case UHI_argnlen:
        opname = "argnlen";
        if (gpr[4] < semihosting_argc) {
            gpr[2] = strlen(semihosting_argv[gpr[4]]);
        } else {
            gpr[2] = 0;
        }
        break;
    case UHI_argn:
        opname = "argn";
        if (gpr[4] < semihosting_argc) {
            gpr[2] = copy_argn_to_target(env, gpr[4], gpr[5]);
        } else {
            gpr[2] = -1;
        }
        break;
    case UHI_plog:
        opname = "plog";
        p = lock_user_string(gpr[4]);
        if (!p) {
            gpr[2] = -1;
            gpr[3] = ENAMETOOLONG;
        } else {
            char *percentd_pos = strstr(p, "%d");
            if (percentd_pos) {
                int char_num = percentd_pos - p;
                char *buf = g_malloc(char_num + 1);
                strncpy(buf, p, char_num);
                buf[char_num] = '\0';
                gpr[2] = printf("%s%d%s", buf, (int)gpr[5], percentd_pos + 2);
                g_free(buf);
            } else {
                gpr[2] = printf("%s", p);
            }
            unlock_user(p, gpr[4], 0);
        }
        break;
    case UHI_assert:
        opname = "assert";
        p = lock_user_string(gpr[4]);
        p2 = lock_user_string(gpr[5]);
        if (!p || !p2) {
            gpr[2] = -1;
            gpr[3] = ENAMETOOLONG;
        } else {
            printf("assertion '");
            printf("\"%s\"", p);
            printf("': file \"%s\", line %d\n", p2, (int)gpr[6]);
            unlock_user(p2, gpr[4], 0);
            unlock_user(p, gpr[4], 0);
        }
        abort();
        break;
    case UHI_exception:
        opname = "exception";
        /* TODO */
        break;
    case UHI_pread:
        opname = "pread";
        gpr[2] = read_from_file(env, gpr[4], gpr[5], gpr[6], gpr[7]);
        gpr[3] = errno;
        break;
    case UHI_pwrite:
        opname = "pwrite";
        gpr[2] = write_to_file(env, gpr[4], gpr[5], gpr[6], gpr[7]);
        gpr[3] = errno;
        break;
    case UHI_yield:
        /* gpr4 not used */
        gpr[2] = sched_yield();
        gpr[3] = errno;
        break;
    case UHI_link:
        opname = "link";
        p = lock_user_string(gpr[4]);
        p2 = lock_user_string(gpr[5]);
        if (!p || !p2) {
            gpr[2] = -1;
            gpr[3] = ENAMETOOLONG;
        } else {
            apply_root(p);
            apply_root(p2);
            gpr[2] = link(p, p2);
            gpr[3] = errno;
            unlock_user(p2, gpr[5], 0);
            unlock_user(p, gpr[4], 0);
        }
        break;
    default:
        gpr[2] = -1;
        gpr[3] = 0;
        break;
    }

    qemu_log("UHI(%s): gpr2:(0x%x), gpr3(0x%x)\n",
             opname, (int)gpr[2], (int)gpr[3]);
}
