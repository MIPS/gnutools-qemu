/* 
   Mapping (done in TLB FILL exception handler -> tlbfill.s):

   VIRTUAL ADDRESS: 0x6xyz 0000

   is translated to

   PHYSICAL ADDRESS: 0x0000 00xy z000 0000

   If "x" == 0, "y" == 0, "z" > 0, then accessing phys addr <= 32
   If "x" == 0, "y" >  0           then accessing phys addr <= 36
   If "x" >  0,                    then accessing phys addr <= 40

   The test uses only 1 tlb entry (index 0), so there is always TLB miss. Therefore
   there is no need to clean TLB after enabling/disabling ELPA flag.
 */

static void OUT_serial(const char * str)
{
    while (*str) {
        *(volatile char *)0xb80003f8 = *str++;
    }
}

#define STW(addr, val) *(volatile unsigned int *) addr = val
#define LDW(addr, var) var = *(volatile unsigned int *)addr

static inline void run_read_test(int elpa)
{
    unsigned int k;
    OUT_serial("TEST:READ,00000000a0000000,0x32b:");
    LDW(0x600A0000, k);

#ifdef TARGET64
    OUT_serial("TEST:READ,0000000120000000,0x33b:");
#else
    if (elpa) {
        OUT_serial("TEST:READ,0000000120000000,0x33b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
#endif
    LDW(0x60120000, k);

#ifdef TARGET64
    OUT_serial("TEST:READ,0000000220000000,0x34b:");
#else
    if (elpa) {
        OUT_serial("TEST:READ,0000000220000000,0x34b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
#endif
    LDW(0x60220000, k);

#ifdef TARGET64
    OUT_serial("TEST:READ,0000000420000000,0x35b:");
#else
    if (elpa) {
        OUT_serial("TEST:READ,0000000420000000,0x35b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
#endif
    LDW(0x60420000, k);

#ifdef TARGET64
    OUT_serial("TEST:READ,0000000820000000,0x36b:");
#else
    if (elpa) {
        OUT_serial("TEST:READ,0000000820000000,0x36b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
#endif
    LDW(0x60820000, k);

    if (elpa) {
        OUT_serial("TEST:READ,0000001020000000,0x37b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
    LDW(0x61020000, k);

    if (elpa) {
        OUT_serial("TEST:READ,0000002020000000,0x38b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
    LDW(0x62020000, k);

    if (elpa) {
        OUT_serial("TEST:READ,0000004020000000,0x39b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
    LDW(0x64020000, k);

    if (elpa) {
        OUT_serial("TEST:READ,0000008020000000,0x40b:");
    } else {
        OUT_serial("TEST:READ,0000000020000000,0x40b:");
    }
    LDW(0x68020000, k);
}

static inline void run_write_test(int elpa)
{
    // Physical address <= 32-bit
    // Expected: Always accessible
    OUT_serial("TEST:WRITE,00000000a0000000,0x32b:");
    STW(0x600A0000, 0x0000032b);

#ifdef TARGET64
    OUT_serial("TEST:WRITE,0000000120000000,0x33b:");
#else
    // Physical address <= 36-bit
    // Expected: Always accessible
    if (elpa) {
        OUT_serial("TEST:WRITE,0000000120000000,0x33b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x33b:");
    }
#endif
    STW(0x60120000, 0x0000033b);

#ifdef TARGET64
    OUT_serial("TEST:WRITE,0000000220000000,0x34b:");
#else
    if (elpa) {
        OUT_serial("TEST:WRITE,0000000220000000,0x34b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x34b:");
    }
#endif
    STW(0x60220000, 0x0000034b);

#ifdef TARGET64
    OUT_serial("TEST:WRITE,0000000420000000,0x35b:");
#else
    if (elpa) {
        OUT_serial("TEST:WRITE,0000000420000000,0x35b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x35b:");
    }
#endif
    STW(0x60420000, 0x0000035b);

#ifdef TARGET64
    OUT_serial("TEST:WRITE,0000000820000000,0x36b:");
#else
    if (elpa) {
        OUT_serial("TEST:WRITE,0000000820000000,0x36b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x36b:");
    }
#endif
    STW(0x60820000, 0x0000036b);

    // Physical address <= 40-bit
    // Expected: 0x20000000 to be accessed if ELPA = 0
    if (elpa) {
        OUT_serial("TEST:WRITE,0000001020000000,0x37b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x37b:");
    }
    STW(0x61020000, 0x0000037b);

    if (elpa) {
        OUT_serial("TEST:WRITE,0000002020000000,0x38b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x38b:");
    }
    STW(0x62020000, 0x0000038b);

    if (elpa) {
        OUT_serial("TEST:WRITE,0000004020000000,0x39b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x39b:");
    }
    STW(0x64020000, 0x0000039b);

    if (elpa) {
        OUT_serial("TEST:WRITE,0000008020000000,0x40b:");
    } else {
        OUT_serial("TEST:WRITE,0000000020000000,0x40b:");
    }
    STW(0x68020000, 0x0000040b);
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
