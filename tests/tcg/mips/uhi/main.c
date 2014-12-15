#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

char buf[10 * 1024];
int bufsize = sizeof(buf);

const char * arg0 = "program_e";
const char * arg1 = "a";
const char * arg2 = "b b";
const char * arg3 = "ccc";
const char * arg4 = "dd\" d \"d";


static void test_remove(void)
{
    int tmp;
    int fd;
    const char *fname = "tmptestprog_remove1";
    const char *fname2 = "tmptestprog_remove2";

    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(close(fd) != -1);

    fd = open(fname2, O_WRONLY | O_CREAT, 0777);
    assert(fd != -1);
    assert(close(fd) != -1);

    assert(remove(fname) == 0);
    assert(remove(fname) == -1);
    assert(errno == ENOENT);

    assert(remove(fname2) == 0);
    assert(remove(fname2) == -1);
    assert(errno == ENOENT);

    printf("%s: pass\n", __func__);
}

static void test_rename(void)
{
    int tmp;
    int fd;
    const char *fname = "tmptestprog_rename1";
    const char *fname2 = "tmptestprog_rename2";

    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(close(fd) != -1);

    assert(rename(fname, fname) == -1);
    assert(errno == EEXIST);

    assert(rename(fname, fname2) == 0);

    assert(rename(fname, fname2) == -1);
    assert(errno == ENOENT);

    assert(rename(fname2, fname) == 0);

#ifdef DO_CLEANUP
    assert(remove(fname) == 0);
#endif

    printf("%s: pass\n", __func__);
}

static void test_rw(void)
{
    int fd;
    const char *fname = "tmptestprog_rw";
    const char *fname2 = "tmptestprog_rw/cause_ENOTDIR_error";
    const char * str1 = "qwertyuiopASDFGHJKL!£$%^&*()][;'.,/><#~2@?";

    fd = open(fname, O_RDWR | O_TRUNC, 0644);
    assert(fd == -1);
    assert(errno == ENOENT);

    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(write(fd, str1, 32) == 32);
    assert(close(fd) != -1);

    fd = open(fname2, O_RDWR | O_TRUNC, 0644);
    assert(fd == -1);
    assert(errno == ENOTDIR);

    fd = open(fname, O_RDONLY);
    assert(fd != -1);
    assert(write(fd, "xyz", 1) == -1);
    assert(errno == EBADF);
    assert(read(fd, buf, 32) == 32);
    assert(close(fd) != -1);

    assert(strncmp(str1, buf, 32) == 0);

    fd = open("_File_Doesnt_Exist_", O_RDONLY);
    assert(fd == -1);
    assert(errno == ENOENT);

#ifdef DO_CLEANUP
    assert(remove(fname) == 0);
#endif

    printf("%s: pass\n", __func__);
}

static void test_lseek(void)
{
    int fd;
    const char *fname = "tmptestprog_lseek";

    int offset = 2;
    const char *str1 = "123456789";
    int str1_len = strlen(str1);
    const char *str2 = "<.,>";
    int str2_len = strlen(str2);
    const char *result = "12<.,>789";

    fd = 0x98761234;
    assert(write(fd, str1, str1_len) == -1);
    assert(errno == EBADF);

    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(write(fd, str1, str1_len) == str1_len);
    assert(lseek(fd, offset, SEEK_SET) == offset);
    assert(write(fd, str2, str2_len) == str2_len);
    assert(lseek(fd, 0, SEEK_SET) == 0);
    assert(read(fd, buf, str1_len) == str1_len);

    assert(lseek(fd, 1, 0xfff) == -1);
    assert(errno == EINVAL);

    assert(close(fd) != -1);

    buf[str1_len] = '\0';
    assert(strcmp(buf, result) == 0);

#ifdef DO_CLEANUP
    assert(remove(fname) == 0);
#endif

    printf("%s: pass\n", __func__);
}


static void test_prw(void)
{
    int fd;
    const char * str1 = "12345678";
    int str1_size = strlen(str1) + 1;
    const char * str2 = "ABCD";
    int str2_size = strlen(str2) + 1;
    int offset = 3;
    const char * result = "123ABCD";

    const char *fname = "tmptestprog_prw";

    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(write(fd, str1, str1_size) == str1_size);
    assert(pwrite(fd, str2, str2_size, offset) == str2_size);
    assert(close(fd) != -1);

    fd = open(fname, O_RDONLY);
    assert(fd != -1);
    assert(pread(fd, buf, str2_size, offset) == str2_size);
    assert(close(fd) != -1);
    assert(strcmp(buf, str2) == 0);

    fd = open(fname, O_RDONLY);
    assert(fd != -1);
    assert(read(fd, buf, str1_size) == str1_size);
    assert(close(fd) != -1);
    assert(strcmp(buf, result) == 0);

#ifdef DO_CLEANUP
    assert(remove(fname) == 0);
#endif

    printf("%s: pass\n", __func__);
}

static void test_rw_big(void)
{
    char byte_in_word[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    const char *fname = "tmptestprog_rw_big";
    int fd, i;

    for (i = 0; i < bufsize; i++) {
        buf[i] = byte_in_word[i%4];
    }
    
    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(write(fd, buf, bufsize) == bufsize);
    assert(close(fd) != -1);

    fd = open(fname, O_RDONLY);
    assert(fd != -1);
    assert(read(fd, buf, bufsize) == bufsize);
    assert(close(fd) != -1);

#ifdef DO_CLEANUP
    assert(remove(fname) == 0);
#endif

    printf("%s: pass\n", __func__);
}

static void test_fstat(void)
{
    const char * fname = "tmptestprog";
    int fd;
    struct stat st;
    memset(&st, 0, sizeof(st));

    fd = 0x98765432;
    assert(fstat(fd, &st) == -1);
    assert(errno == EBADF);    

    fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    assert(write(fd, "abcd", 4) == 4);
    assert(fstat(fd, &st) == 0);
    assert(close(fd) != -1);

    assert(st.st_size == 4);
/*
    printf("st_dev = 0x%08x\n", st.st_dev);
    printf("st_ino = 0x%08x\n", st.st_ino);
    printf("st_mode = 0x%08x\n", st.st_mode);
    printf("st_nlink = 0x%08x\n", st.st_nlink);
    printf("st_uid = 0x%08x\n", st.st_uid);
    printf("st_gid = 0x%08x\n", st.st_gid);
    printf("st_rdev = 0x%08x\n", st.st_rdev);
    printf("st_size = 0x%08x\n", st.st_size);
    printf("st_blksize = 0x%08x\n", st.st_blksize);
    printf("st_blocks = 0x%08x\n", st.st_blocks);
    printf("st_atime = 0x%08x\n", st.st_atime);
    printf("st_mtime = 0x%08x\n", st.st_mtime);
    printf("st_ctime = 0x%08x\n", st.st_ctime);
*/

#ifdef DO_CLEANUP
    assert(remove(fname) == 0);
#endif

    printf("%s: pass\n", __func__);
}

int uhi_mips_plog(char *msg, int num)
{
    void *gpr4 = msg;
    int gpr5 = num;
    int gpr2;

    __asm__ ( "ori    $25, $0, 13\n"
              "or     $5, $0, %1\n"
              "or     $4, $0, %2\n"
              "sdbbp  1\n"
              "or     %0, $0, $2"
              : "=r" (gpr2)
              :  "r" (gpr5), "r" (gpr4));

    return gpr2;
}

static void test_plog_1234_1234(void)
{
    /* expecting 'test_plog_1234_1234' to be printed */
    uhi_mips_plog("test_plog_%d_1234: pass\n", 1234);
}

int uhi_mips_argc()
{
    int gpr2;

    __asm__ ( "ori    $25, $0, 9\n"
              "sdbbp  1\n"
              "or     %0, $0, $2"
              : "=r" (gpr2));

    return gpr2;
}

static void test_argc(void)
{
    assert(uhi_mips_argc() == 5);
    printf("%s: pass\n", __func__);
}

int uhi_mips_argnlen(int arg_num)
{
    int gpr4 = arg_num;
    int gpr2;

    __asm__ ( "ori    $25, $0, 0xa\n"
              "or     $4, $0, %1\n"
              "sdbbp  1\n"
              "or     %0, $0, $2"
              : "=r" (gpr2)
              : "r" (gpr4));

    return gpr2;
}

static void test_argnlen(void)
{
    assert(uhi_mips_argnlen(1) == 1);
    assert(uhi_mips_argnlen(2) == 3);
    assert(uhi_mips_argnlen(3) == 3);
    assert(uhi_mips_argnlen(4) == 8);
    assert(uhi_mips_argnlen(99) == -1);
    printf("%s: pass\n", __func__);
}

int uhi_mips_argn(int arg_num, void *argbuf)
{
    void *gpr5 = argbuf;
    int gpr4 = arg_num;
    int gpr2;

    __asm__ ( "ori    $25, $0, 0xb\n"
              "sdbbp  1\n"
              "or     %0, $0, $2"
              : "=r" (gpr2)
              : "r" (gpr4), "r" (gpr5));

    return gpr2;
}

static void test_argn(void)
{
    assert(uhi_mips_argn(1, buf) == 0);
    assert(strcmp(buf, arg1) == 0);

    assert(uhi_mips_argn(0, buf) == 0);
    assert(strncmp(buf, arg0, strlen(arg0)) == 0);

    assert(uhi_mips_argn(2, buf) == 0);
    assert(strcmp(buf, arg2) == 0);

    assert(uhi_mips_argn(3, buf) == 0);
    assert(strcmp(buf, arg3) == 0);

    assert(uhi_mips_argn(4, buf) == 0);
    assert(strcmp(buf, arg4) == 0);

    assert(uhi_mips_argn(5, buf) == -1);

    printf("%s: pass\n", __func__);
}


int main(int argc, char *argv[])
{
    assert(argc == 5);
    assert(strncmp(argv[0], arg0, strlen(arg0)) == 0);
    assert(strcmp(argv[1], arg1) == 0);
    assert(strcmp(argv[2], arg2) == 0);
    assert(strcmp(argv[3], arg3) == 0);
    assert(strcmp(argv[4], arg4) == 0);

    test_remove();
    test_rename();
    test_rw();
    test_rw_big();
    test_prw();
    test_lseek();
    test_fstat();
    test_plog_1234_1234();
    test_argc();
    test_argnlen();
    test_argn();

    exit(88);
    return 0;
}
