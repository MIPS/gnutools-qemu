#define STW(addr, val) *(volatile unsigned int *) addr = val

static inline void TEST_END(const char * str)
{
    while (*str) {
        *(volatile char *)0xb80003f8 = *str++;
    }
    STW(0x9f000500, 0x42); // Shutdown
}

void pass()
{
    TEST_END("pass");
}

void fail()
{
    TEST_END("fail");
}

