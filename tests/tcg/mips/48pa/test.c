/* 
   Mapping (done in TLB FILL exception handler -> tlbfill.s):

   VIRTUAL ADDRESS: 0x6abc d000

   is translated to

   PHYSICAL ADDRESS: 0x0000 abcd 0000 0000

   If d > 0, then accessing phys addr <= 36
   If c > 0, then accessing phys addr <= 40
   If b > 0, then accessing phys addr <= 44
   If a > 0, then accessing phys addr <= 48

   The test uses only 1 tlb entry (index 0), so there is always TLB miss. Therefore
   there is no need to clean TLB after enabling/disabling ELPA flag.
 */

static void OUT_serial(const char * str)
{
    while (*str) {
        *(volatile char *)0xb80003f8 = *str++;
    }
}

#define MAX_B_STR "0x48b"
#define LOWEST_ADDR "0000000200000000"

#define TEST_READ(VA, PA_STR, B_STR)                        \
    OUT_serial("TEST:READ," PA_STR "," B_STR ":");          \
    LDW(VA, k);

#define TEST_WRITE(VA, PA_STR, B_STR, VAL)                  \
    OUT_serial("TEST:WRITE," PA_STR "," B_STR ":");         \
    STW(VA, VAL);

#define TEST_READ_ELPA(VA, PA_STR, B_STR)                           \
    if (elpa) {                                                     \
        OUT_serial("TEST:READ," PA_STR "," B_STR ":");              \
    } else {                                                        \
        OUT_serial("TEST:READ," LOWEST_ADDR "," MAX_B_STR ":");     \
    }                                                               \
    LDW(VA, k);

#define TEST_WRITE_ELPA(VA, PA_STR, B_STR, VAL)                     \
    if (elpa) {                                                     \
        OUT_serial("TEST:WRITE," PA_STR "," B_STR ":");             \
    } else {                                                        \
        OUT_serial("TEST:WRITE," LOWEST_ADDR "," B_STR ":");        \
    }                                                               \
    STW(VA, VAL);



#define STW(addr, val) *(volatile unsigned int *) addr = val
#define LDW(addr, var) var = *(volatile unsigned int *)addr

static inline void run_read_test(int elpa)
{
    unsigned int k;
    TEST_READ(0x6000A000, "0000000a00000000", "0x36b")
    TEST_READ_ELPA(0x60012000, "0000001200000000", "0x37b")
    TEST_READ_ELPA(0x60022000, "0000002200000000", "0x38b")
    TEST_READ_ELPA(0x60042000, "0000004200000000", "0x39b")
    TEST_READ_ELPA(0x60082000, "0000008200000000", "0x40b")
    TEST_READ_ELPA(0x60102000, "0000010200000000", "0x41b")
    TEST_READ_ELPA(0x60202000, "0000020200000000", "0x42b")
    TEST_READ_ELPA(0x60402000, "0000040200000000", "0x43b")
    TEST_READ_ELPA(0x60802000, "0000080200000000", "0x44b")
    TEST_READ_ELPA(0x61002000, "0000100200000000", "0x45b")
    TEST_READ_ELPA(0x62002000, "0000200200000000", "0x46b")
    TEST_READ_ELPA(0x64002000, "0000400200000000", "0x47b")
    TEST_READ_ELPA(0x68002000, "0000800200000000", "0x48b")
}

static inline void run_write_test(int elpa)
{
    TEST_WRITE(0x6000A000, "0000000a00000000", "0x36b", 0x36b)
    TEST_WRITE_ELPA(0x60012000, "0000001200000000", "0x37b", 0x37b)
    TEST_WRITE_ELPA(0x60022000, "0000002200000000", "0x38b", 0x38b)
    TEST_WRITE_ELPA(0x60042000, "0000004200000000", "0x39b", 0x39b)
    TEST_WRITE_ELPA(0x60082000, "0000008200000000", "0x40b", 0x40b)
    TEST_WRITE_ELPA(0x60102000, "0000010200000000", "0x41b", 0x41b)
    TEST_WRITE_ELPA(0x60202000, "0000020200000000", "0x42b", 0x42b)
    TEST_WRITE_ELPA(0x60402000, "0000040200000000", "0x43b", 0x43b)
    TEST_WRITE_ELPA(0x60802000, "0000080200000000", "0x44b", 0x44b)
    TEST_WRITE_ELPA(0x61002000, "0000100200000000", "0x45b", 0x45b)
    TEST_WRITE_ELPA(0x62002000, "0000200200000000", "0x46b", 0x46b)
    TEST_WRITE_ELPA(0x64002000, "0000400200000000", "0x47b", 0x47b)
    TEST_WRITE_ELPA(0x68002000, "0000800200000000", "0x48b", 0x48b)
}

#define ENABLE_LPA __asm__(                     \
    " .set    noat            \n"               \
    " lui     $2, 0x2000      \n"               \
    " mfc0    $1,$5, 1        \n"               \
    " or      $1,$1,$2        \n"               \
    " mtc0    $1,$5, 1        \n")

#define DISABLE_LPA __asm__(                    \
    " .set    noat            \n"               \
    " lui     $2, 0x2000      \n"               \
    " not     $2              \n"               \
    " mfc0    $1,$5, 1        \n"               \
    " and     $1,$1,$2        \n"               \
    " mtc0    $1,$5, 1        \n")

void entry()
{
    int tmp = 0;

    OUT_serial("\n--- ENABLE LPA\n");
    ENABLE_LPA;
    run_write_test(1);
    run_read_test(1);


    OUT_serial("\n--- DISABLE LPA\n");
    DISABLE_LPA;
    run_write_test(0);
    run_read_test(0);

    STW(0x9f000500, 0x42); // Shutdown
}
