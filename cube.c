#define LOW_BITS 0x0706050403020100
#define HIGH_BITS 0x0f0e0d0c0b0a0908

#define ORIENT_CARRY _mm256_set_epi64x(0x3030303030303030, 0x3030303030303030, 0x2020202020202020, 0x2020202020202020)

#define ORIENT_MASK _mm256_set1_epi8(0xf0)
#define PERMUTE_MASK _mm256_set1_epi8(0x0f)

#define SET_CO(x, a) ((x) = _mm256_or_si256((x), _mm256_set_epi64x(0, (a), 0, 0)))
#define SET_CP(x, a) ((x) = _mm256_insert_epi64((x), (a), 2))
#define SET_EO(x, b, a) ((x) = _mm256_or_si256((x), _mm256_set_epi64x(0, 0, (b), (a))))
#define SET_EP(x, b, a) ((x) = _mm256_inserti128_si256((x), _mm_set_epi64x((b), (a)), 0))

static inline unsigned long long set_perm(int c, int s, unsigned long long m, int p)
{
    unsigned long long a, b;
    a = _pdep_u64(c, m);
    m = a << 1 | a;
    a = a << s;
    b = _pdep_u64(p, m);
    b = s>1 ? a|b : b;
    return b;
}

static inline unsigned long long set_comb(int c, int s, unsigned long long m)
{
    return set_perm(c, s, m, 0xe4);
}

static inline cube_t new_cube(void)
{
    return CUBE(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
}

static void print_cube(cube_t x)
{
    char mem[32];
    _mm256_storeu_si256((__m256i *)mem, x);
    printf("CUBE(");
    for (int i=0; i<8; ++i)
        printf("%s0x%02x", i?", ":"", mem[i+16]);
    printf(",    ");
    for (int i=0; i<12; ++i)
        printf("%s0x%02x", i?", ":"", mem[i]);
    printf(")");
}

static inline int cube_lt(cube_t x, cube_t y)
{
    unsigned a, b;
    __m256i c;
    c = _mm256_cmpgt_epi8(x, y);
    a = _mm256_movemask_epi8(c);
    c = _mm256_cmpgt_epi8(y, x);
    b = _mm256_movemask_epi8(c);
    return a < b;
}

static inline int cube_eq(cube_t x, cube_t y)
{
    unsigned r;
    __m256i c;
    c = _mm256_cmpeq_epi8(x, y);
    r = _mm256_movemask_epi8(c);
    return r == -1u;
}

static int check_get_parity(cube_t x)
{
    char a[32];
    _mm256_storeu_si256((__m256i *)a, x);
    for (int i=0; i<LENGTH(a); ++i)
        a[i]&=0x0f,printf("%3d",(int)a[i]);

    int parity(char *a, int n)
    {
        int r=0;
        for (int j=0; j<n; ++j)
            for (int i=0; i<j; ++i)
                r+=a[i]>a[j];
        return r%2;
    }

    return parity(a,12)<<1|parity(a+16,8);
}

static inline cube_t cube_canonicalise(cube_t x, int *sym)
{
    ASSERT(sym);
    *sym = 0;
    cube_t y, r=x;
    for (int s=1; s<NUM_SYMS; s++)
        if (cube_lt(y=apply_sym(x, s), r))
            r=y, *sym=s;
    return r;
}

static inline cube_t mirrored_compose(cube_t x, cube_t y, int mirror)
{
    __m256i r, o;
    x = _mm256_shuffle_epi8(x, y);
    o = _mm256_and_si256(y, ORIENT_MASK);
    if (mirror)
    {
        x = _mm256_sub_epi8(x, o);
        r = _mm256_add_epi8(x, ORIENT_CARRY);
    }
    else
    {
        x = _mm256_add_epi8(x, o);
        r = _mm256_sub_epi8(x, ORIENT_CARRY);
    }
    r = _mm256_min_epu8(x, r);
    return r;
}

static inline cube_t compose(cube_t x, cube_t y)
{
    return mirrored_compose(x, y, 0);
}

static inline long long get_eo(cube_t x)
{
    long long r;
    x = _mm256_and_si256(x, ORIENT_MASK);
    // move orientation to high bit
    x = _mm256_slli_epi32(x, 3);
    // create mask from high bits
    r = _mm256_movemask_epi8(x);
    r = r & ((1 << (NUM_EDGES-1)) - 1);
    return r;
}

static inline void set_eo(cube_t *x, long long r)
{
    unsigned long long l, h;
    r = _mm_popcnt_u64(r) % 2 << 11 | r;
    l = _pdep_u64(r, 0x1010101010101010);
    r = r >> 8;
    h = _pdep_u64(r, 0x1010101010101010);
    SET_EO(*x, h, l);
}

static inline long long get_co(cube_t x)
{
    unsigned long long r;
    __m128i c, y;
    // create 8x16bit vector y of corner orientations
    x = _mm256_and_si256(x, ORIENT_MASK);
    y = _mm256_extracti128_si256(x, 1);
    y = _mm_cvtepu8_epi16(y);
    // multiply each corner orientation by its place value, then sum
    c = _mm_set_epi16(0, 729, 243, 81, 27, 9, 3, 1);
    y = _mm_madd_epi16(y, c);
    r = _mm_extract_epi64(y, 0);
    r = _mm_extract_epi64(y, 1) + r;
    r = r + (r >> 32);
    r = r >> 4;
    r = r & 0xffffffff;
    return r;
}

static inline void set_co(cube_t *x, long long r)
{
    unsigned long long b = 0;
    for (int i=0, p=0; i<8; ++i, p+=r%3, r/=3)
        b |= (i==7?3-p%3:r)%3<<(i*8+4);
    SET_CO(*x, b);
}

static inline long long get_csep(cube_t x)
{
    unsigned char b;
    x = _mm256_slli_epi32(x, 5);
    b = _mm256_movemask_epi8(x) >> 16;
    return rank_8C4[b];
};

static inline void set_csep(cube_t *x, long long r)
{
    unsigned long long b;
    unsigned char t;
    t = unrank_8C4[r];
    b = 0;
    b = b | set_comb(t, 2, 0x0101010101010101);
    t = t ^ 0xff;
    b = b | set_comb(t, 1, 0x0101010101010101);
    SET_CP(*x, b);
}

static inline long long get_esep(cube_t x)
{
    unsigned m, s;
    // mask of positions of the S-slice edges
    x = _mm256_slli_epi32(x, 4);
    s = _mm256_movemask_epi8(x);
    s = s & 0xfff;
    // mask of positions of the M-slice edges
    x = _mm256_slli_epi32(x, 1);
    m = _mm256_movemask_epi8(x);
    // "filter out" the S-slice edges from the M-slice edges
    m = _pext_u32(m, ~s);
    m = m & 0xff;
    return rank_8C4[m] * choose[12][4] + rank_12C4[s];
}

static inline void set_esep(cube_t *x, long long r)
{
    unsigned long long b, l, h;
    unsigned short e, m, s;
    s = unrank_12C4[r%choose[12][4]];
    m = unrank_8C4[r/choose[12][4]];
    m = _pdep_u32(m, ~s);
    e = 0xfff ^ m ^ s;
    b = 0xfedc000000000000;
    b = b | set_comb(s, 3, 0x111111111111);
    b = b | set_comb(m, 2, 0x111111111111);
    b = b | set_comb(e, 1, 0x111111111111);
    l = _pdep_u64(b, 0x0f0f0f0f0f0f0f0f);
    b = b >> 32;
    h = _pdep_u64(b, 0x0f0f0f0f0f0f0f0f);
    SET_EP(*x, h, l);
}

static inline long long get_cp(cube_t x)
{
    long long b, m, h, l;
    b = _mm256_extract_epi64(x, 2);
    m = b & 0x0404040404040404;
    m = m >> 1 | m >> 2;
    h = _pext_u64(b, m);
    m = m ^ 0x0303030303030303;
    l = _pext_u64(b, m);
    b = _pext_u64(b, 0x0404040404040404);
    return rank_8C4[b] * fact[4] * fact[4] + rank_4P4[h] * fact[4] + rank_4P4[l];
}

static inline void set_cp(cube_t *x, long long r)
{
    unsigned long long b;
    unsigned char t;
    t = unrank_8C4[r / fact[4] / fact[4]];
    b = 0;
    b = b | set_perm(t, 2, 0x0101010101010101, unrank_4P4[r / fact[4] % fact[4]]);
    t = t ^ 0xff;
    b = b | set_perm(t, 1, 0x0101010101010101, unrank_4P4[r % fact[4]]);
    SET_CP(*x, b);
}

static inline long long get_ep(cube_t x)
{
    unsigned long long b, h, l, e, m, s;
    x = _mm256_and_si256(x, PERMUTE_MASK);
    h = _mm256_extract_epi64(x, 1);
    l = _mm256_extract_epi64(x, 0);
    b = _pext_u64(h, 0x0f0f0f0f0f0f0f0f) << 32;
    b = _pext_u64(l, 0x0f0f0f0f0f0f0f0f) | b;
    s = 0x0000888888888888 & b;
    s = s >> 2 | s >> 3;
    m = 0x0000444444444444 & b;
    m = m >> 1 | m >> 2;
    e = 0x0000333333333333 ^ m ^ s;
#define R(x) rank_4P4[_pext_u64(b, x)]
    return get_esep(x) * fact[4] * fact[4] * fact[4] + R(e) * fact[4] * fact[4] + R(m) * fact[4] + R(s);
#undef R
}

static inline void set_ep(cube_t *x, long long r)
{
    unsigned long long b, h, l;
    unsigned short e, m, s;
    s = unrank_12C4[r / fact[4] / fact[4] / fact[4] % choose[12][4]];
    m = unrank_8C4[r / fact[4] / fact[4] / fact[4] / choose[12][4]];
    m = _pdep_u32(m, ~s);
    e = 0xfff ^ m ^ s;
    b = 0xfedc000000000000;
    b = b | set_perm(s, 3, 0x1111111111111111, unrank_4P4[r % fact[4]]);
    b = b | set_perm(m, 2, 0x1111111111111111, unrank_4P4[r / fact[4] % fact[4]]);
    b = b | set_perm(e, 1, 0x1111111111111111, unrank_4P4[r / fact[4] / fact[4] % fact[4]]);
    l = _pdep_u64(b, 0x0f0f0f0f0f0f0f0f);
    b = b >> 32;
    h = _pdep_u64(b, 0x0f0f0f0f0f0f0f0f);
    SET_EP(*x, h, l);
}

// assume cubies are placed in their tetrad/slice
static inline long long get_orbit_fast(cube_t x)
{
    unsigned long long a[4], b;
    long long r = 0;
    _mm256_storeu_si256((__m256i *)a, x);
    for (int i=5; i>=0; --i)
    {
        if (i==3) continue;
        b = _pext_u64(a[i/2], 0x03030303ull << i%2*32);
        r = r * fact[4] + rank_4P4[b] / (i==5 ? 2 : 1);
    }
    return r;
}

static inline void set_orbit_fast(cube_t *x, long long r)
{
    unsigned long long a[4] = {0x0404040400000000, 0x0f0e0d0c08080808, 0x0404040400000000, HIGH_BITS}, b;
    for (int i=0, j, p=0; i<6; ++i)
    {
        if (i==3) continue;
        j = r % fact[4] * (i==5 ? 2 : 1); // index of permutation of 4 pieces
        p = p ^ j&1;
        if (i==5) j += p; // set corner parity to edge parity
        b = unrank_4P4[j];
        a[i/2] = _pdep_u64(b, 0x03030303ull << i%2*32) | a[i/2];
        r /= fact[4];
    }
    *x = _mm256_lddqu_si256((__m256i *)a);
}

static inline cube_t inverse(cube_t x)
{
    // - any permutation of the cubies can be borken into cycles
    // - cubies in the same cycle exchange places each time the permutation is
    //   applied
    // - the order of a cycle is its length
    // - the possible cycle lengths for the cubies is are (1,12]
    // - their LCM is 27720
    // - therefore, raising a position to the 27719th power gives its inverse
    // - see https://github.com/Voltara/vcube/blob/master/src/avx2_cube.h
    __m256i o, y, z;
    o = _mm256_and_si256(x, ORIENT_MASK);
    // http://wwwhomes.uni-bielefeld.de/achim/addition_chain.html
    y = _mm256_and_si256(x, PERMUTE_MASK);
    x = _mm256_shuffle_epi8(y, y); // 2
    z = _mm256_shuffle_epi8(x, y); // 3 (+1)
    x = _mm256_shuffle_epi8(z, z); // 6
    x = _mm256_shuffle_epi8(x, x); // 12
    x = _mm256_shuffle_epi8(x, x); // 24
    x = _mm256_shuffle_epi8(x, z); // 27 (+3)
    x = _mm256_shuffle_epi8(x, x); // 54
    x = _mm256_shuffle_epi8(x, x); // 108
    x = _mm256_shuffle_epi8(x, x); // 216
    x = _mm256_shuffle_epi8(x, x); // 432
    x = _mm256_shuffle_epi8(x, y); // 433 (+1)
    x = _mm256_shuffle_epi8(x, x); // 866
    x = _mm256_shuffle_epi8(x, x); // 1732
    x = _mm256_shuffle_epi8(x, x); // 3464
    x = _mm256_shuffle_epi8(x, x); // 6928
    x = _mm256_shuffle_epi8(x, x); // 13856
    x = _mm256_shuffle_epi8(x, z); // 13859 (+3)
    x = _mm256_shuffle_epi8(x, x); // 27718
    x = _mm256_shuffle_epi8(x, y); // 27719 (+1)
    // invert orientations
    o = _mm256_sub_epi8(ORIENT_CARRY, o);
    y = _mm256_sub_epi8(o, ORIENT_CARRY);
    o = _mm256_min_epu8(o, y);
    // permute the orientations
    o = _mm256_shuffle_epi8(o, x);
    // combine the orientations
    x = _mm256_or_si256(x, o);
    return x;
}

////////////////////////////////////////////////////////////////////////////////

static inline cube_t apply_sym(cube_t x, int s)
{
    cube_t compose_3(cube_t x, cube_t y, cube_t z)
    {
        return mirrored_compose(mirrored_compose(x, y, s&1), z, s&1);
    }

    return compose_3(get_sym_cube(inv_sym[s]), x, get_sym_cube(s));
}

static inline cube_t apply_move(cube_t x, int move)
{
    return compose(x, get_move_cube(move));
}

static inline cube_t apply_pre_move(cube_t x, int move)
{
    return compose(get_move_cube(inverse_move(move)), x);
}

static inline cube_t apply_moves(cube_t x, int *moves, int length)
{
    for (int i=0; i<length; ++i)
        x = apply_move(x, moves[i]);
    return x;
}
