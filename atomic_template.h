/*
 * Atomic helper templates
 * Included from tcg-runtime.c.
 *
 * Copyright (c) 2016 Red Hat, Inc
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#define DATA_SIZE (1 << SHIFT)

#if DATA_SIZE == 8
#define SUFFIX     q
#define DATA_TYPE  uint64_t
#define ABI_TYPE   uint64_t
#define BSWAP      bswap64
#elif DATA_SIZE == 4
#define SUFFIX     l
#define DATA_TYPE  uint32_t
#define ABI_TYPE   uint32_t
#define BSWAP      bswap32
#elif DATA_SIZE == 2
#define SUFFIX     w
#define DATA_TYPE  uint16_t
#define ABI_TYPE   uint32_t
#define BSWAP      bswap16
#elif DATA_SIZE == 1
#define SUFFIX     b
#define DATA_TYPE  uint8_t
#define ABI_TYPE   uint32_t
#else
#error unsupported data size
#endif

#ifdef CONFIG_USER_ONLY

#if DATA_SIZE == 1
# define HE_SUFFIX
#elif defined(HOST_WORDS_BIGENDIAN)
# define HE_SUFFIX  _be
# define RE_SUFFIX  _le
#else
# define HE_SUFFIX  _le
# define RE_SUFFIX  _be
#endif

ABI_TYPE HELPER(glue(glue(atomic_cmpxchg, SUFFIX), HE_SUFFIX))
    (target_ulong addr, ABI_TYPE cmpv, ABI_TYPE newv)
{
    DATA_TYPE *haddr = g2h(addr);
    return atomic_cmpxchg(haddr, cmpv, newv);
}

#define GEN_ATOMIC_HELPER_HE(NAME)                                  \
ABI_TYPE HELPER(glue(glue(atomic_##NAME, SUFFIX), HE_SUFFIX))       \
    (target_ulong addr, ABI_TYPE val)                               \
{                                                                   \
    DATA_TYPE *haddr = g2h(addr);                                   \
    return atomic_##NAME(haddr, val);                               \
}                                                                   \

GEN_ATOMIC_HELPER_HE(fetch_add)
GEN_ATOMIC_HELPER_HE(fetch_and)
GEN_ATOMIC_HELPER_HE(fetch_or)
GEN_ATOMIC_HELPER_HE(fetch_xor)
GEN_ATOMIC_HELPER_HE(add_fetch)
GEN_ATOMIC_HELPER_HE(and_fetch)
GEN_ATOMIC_HELPER_HE(or_fetch)
GEN_ATOMIC_HELPER_HE(xor_fetch)
GEN_ATOMIC_HELPER_HE(xchg)

#undef GEN_ATOMIC_HELPER_HE

#if DATA_SIZE > 1

ABI_TYPE HELPER(glue(glue(atomic_cmpxchg, SUFFIX), RE_SUFFIX))
    (target_ulong addr, ABI_TYPE cmpv, ABI_TYPE newv)
{
    DATA_TYPE *haddr = g2h(addr);
    return BSWAP(atomic_cmpxchg(haddr, BSWAP(cmpv), BSWAP(newv)));
}

#define GEN_ATOMIC_HELPER_RE(NAME)                                  \
ABI_TYPE HELPER(glue(glue(atomic_##NAME, SUFFIX), RE_SUFFIX))       \
    (target_ulong addr, ABI_TYPE val)                               \
{                                                                   \
    DATA_TYPE *haddr = g2h(addr);                                   \
    return BSWAP(atomic_##NAME(haddr, BSWAP(val)));                 \
}

GEN_ATOMIC_HELPER_RE(fetch_and)
GEN_ATOMIC_HELPER_RE(fetch_or)
GEN_ATOMIC_HELPER_RE(fetch_xor)
GEN_ATOMIC_HELPER_RE(and_fetch)
GEN_ATOMIC_HELPER_RE(or_fetch)
GEN_ATOMIC_HELPER_RE(xor_fetch)
GEN_ATOMIC_HELPER_RE(xchg)

/* Note that for addition, we need to use a separate cmpxchg loop instead
   of bswaps for the reverse-host-endian helpers.  */
ABI_TYPE HELPER(glue(glue(atomic_fetch_add, SUFFIX), RE_SUFFIX))
    (target_ulong addr, ABI_TYPE val)
{
    DATA_TYPE ldo, ldn, ret, sto;
    DATA_TYPE *haddr = g2h(addr);

    ldo = *haddr;
    while (1) {
        ret = BSWAP(ldo);
        sto = BSWAP(ret + val);
        ldn = atomic_cmpxchg(haddr, ldo, sto);
        if (ldn == ldo) {
            return ret;
        }
        ldo = ldn;
    }
}

ABI_TYPE HELPER(glue(glue(atomic_add_fetch, SUFFIX), RE_SUFFIX))
    (target_ulong addr, ABI_TYPE val)
{
    DATA_TYPE ldo, ldn, ret, sto;
    DATA_TYPE *haddr = g2h(addr);

    ldo = *haddr;
    while (1) {
        ret = BSWAP(ldo) + val;
        sto = BSWAP(ret);
        ldn = atomic_cmpxchg(haddr, ldo, sto);
        if (ldn == ldo) {
            return ret;
        }
        ldo = ldn;
    }
}

#undef GEN_ATOMIC_HELPER_RE
#endif /* DATA_SIZE > 1 */

#undef HE_SUFFIX
#undef RE_SUFFIX

#else /* !CONFIG_USER_ONLY */

#if DATA_SIZE == 1
#define LE_SUFFIX
#else
#define LE_SUFFIX  _le
#endif

ABI_TYPE HELPER(glue(glue(atomic_cmpxchg, SUFFIX), LE_SUFFIX))
    (CPUArchState *env, target_ulong addr,
     ABI_TYPE cmpv, ABI_TYPE newv, uint32_t oi)
{
    return glue(glue(glue(helper_atomic_cmpxchg, SUFFIX), LE_SUFFIX), _mmu)
        (env, addr, cmpv, newv, oi, GETPC());
}

#define GEN_ATOMIC_HELPER(NAME, S)                                      \
ABI_TYPE HELPER(glue(glue(atomic_##NAME, SUFFIX), S))                   \
    (CPUArchState *env, target_ulong addr, ABI_TYPE val, uint32_t oi)   \
{                                                                       \
    return glue(glue(glue(helper_atomic_##NAME, SUFFIX), S), _mmu)      \
        (env, addr, val, oi, GETPC());                                  \
}

GEN_ATOMIC_HELPER(fetch_add, LE_SUFFIX)
GEN_ATOMIC_HELPER(fetch_and, LE_SUFFIX)
GEN_ATOMIC_HELPER(fetch_or, LE_SUFFIX)
GEN_ATOMIC_HELPER(fetch_xor, LE_SUFFIX)
GEN_ATOMIC_HELPER(add_fetch, LE_SUFFIX)
GEN_ATOMIC_HELPER(and_fetch, LE_SUFFIX)
GEN_ATOMIC_HELPER(or_fetch, LE_SUFFIX)
GEN_ATOMIC_HELPER(xor_fetch, LE_SUFFIX)
GEN_ATOMIC_HELPER(xchg, LE_SUFFIX)

#if DATA_SIZE > 1

ABI_TYPE HELPER(glue(glue(atomic_cmpxchg, SUFFIX), _be))
    (CPUArchState *env, target_ulong addr,
     ABI_TYPE cmpv, ABI_TYPE newv, uint32_t oi)
{
    return glue(glue(helper_atomic_cmpxchg, SUFFIX), _be_mmu)
        (env, addr, cmpv, newv, oi, GETPC());
}

GEN_ATOMIC_HELPER(fetch_add, _be)
GEN_ATOMIC_HELPER(fetch_and, _be)
GEN_ATOMIC_HELPER(fetch_or, _be)
GEN_ATOMIC_HELPER(fetch_xor, _be)
GEN_ATOMIC_HELPER(add_fetch, _be)
GEN_ATOMIC_HELPER(and_fetch, _be)
GEN_ATOMIC_HELPER(or_fetch, _be)
GEN_ATOMIC_HELPER(xor_fetch, _be)
GEN_ATOMIC_HELPER(xchg, _be)

#endif /* DATA_SIZE > 1 */

#undef GEN_ATOMIC_HELPER
#undef LE_SUFFIX

#endif /* CONFIG_USER_ONLY */

#undef BSWAP
#undef ABI_TYPE
#undef DATA_TYPE
#undef SUFFIX
#undef DATA_SIZE
#undef SHIFT
