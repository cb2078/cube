#define LOW_BITS 0x0706050403020100
#define HIGH_BITS 0x0f0e0d0c0b0a0908

#define ORIENT_CARRY _mm256_set_epi64x(0x3030303030303030, 0x3030303030303030, 0x2020202020202020, 0x2020202020202020)

#define ORIENT_MASK _mm256_set1_epi8(0xf0)
#define PERMUTE_MASK _mm256_set1_epi8(0x0f)

#define SET_EDGES(x, y) _mm256_set_epi64x(HIGH_BITS, LOW_BITS, (x), (y))
#define SET_CORNERS(x) _mm256_set_epi64x(HIGH_BITS, (x), HIGH_BITS, LOW_BITS)
#define MERGE_EDGES(x, y) SET_EDGES((x)|HIGH_BITS, (y)|LOW_BITS)
#define MERGE_CORNERS(x) SET_CORNERS((x)|LOW_BITS)

static inline unsigned long long set_comb(int, int, unsigned long long);

static cube_t new_cube(void)
{
    return CUBE(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
}

static void print_cube(cube_t x)
{
    char mem[32];
    _mm256_storeu_si256((__m256i *)mem, x);
    printf("//");
    for (int i=0; i<20; ++i)
        printf(" %3s", cubie_str[i]);
    printf("\n");
    for (int j=0; j<2; ++j)
    {
        printf("%c:", j?'o':'p');
        for (int i=0; i<8; ++i)
            printf(" %3d", mem[i+16]>>4*j&0x0f);
        for (int i=0; i<12; ++i)
            printf(" %3d", mem[i]>>4*j&0x0f);
        printf("\n");
    }
}

static int cube_eq(cube_t x, cube_t y)
{
    int r;
    __m256i c;
    c = _mm256_cmpeq_epi8(x, y);
    r = _mm256_movemask_epi8(c);
    return r == -1;
}

static cube_t mirrored_compose(cube_t x, cube_t y, int mirror)
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

static cube_t compose(cube_t x, cube_t y)
{
    return mirrored_compose(x, y, 0);
}

static long long get_eo(cube_t x)
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

static cube_t set_eo(long long r)
{
    unsigned long long l, h;
    r = r | ((_mm_popcnt_u64(r)&1)<<11);
    l = _pdep_u64(r, 0x1010101010101010);
    h = _pdep_u64(r>>8, 0x1010101010101010);
    return MERGE_EDGES(h, l);
}

static long long get_co(cube_t x)
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

static cube_t set_co(long long r)
{
    unsigned long long b = 0;
    for (int i=0, p=0; i<8; ++i, p+=r%3, r/=3)
        b |= (i==7?3-p%3:r)%3<<(i*8+4);
    return MERGE_CORNERS(b);
}

static long long get_csep(cube_t x)
{
    unsigned char b;
    x = _mm256_slli_epi32(x, 5);
    b = _mm256_movemask_epi8(x) >> 16;
    return rank_8C4[b];
};

static cube_t set_csep(long long r)
{
    unsigned long long a, b, m;
    a = unrank_8C4[r];
    m = 0x0101010101010101;
    b = set_comb(a, 2, m) | set_comb(0xff ^ a, 1, m);
    return SET_CORNERS(b);
}

static long long get_esep(cube_t x)
{
    unsigned e, s;
    // mask of positions of the E-slice edges
    x = _mm256_slli_epi32(x, 4);
    e = _mm256_movemask_epi8(x);
    e = e & 0xfff;
    // mask of positions of the S-slice edges
    x = _mm256_slli_epi32(x, 1);
    s = _mm256_movemask_epi8(x);
    // "filter out" the E-slice edges from the S-slice edges
    s = _pext_u32(s, ~e);
    s = s & 0xff;
    return rank_8C4[s] * 512 + INSERT_ESLICE_GAPS(rank_12C4[e]);
}

static cube_t set_esep(long long r)
{
    unsigned long long b, l, h;
    unsigned short e, s, m;
    e = unrank_12C4[REMOVE_ESLICE_GAPS(r%512)];
    s = unrank_8C4[r/512];
    s = _pdep_u32(s, ~e);
    m = 0xfff ^ s ^ e;
    b = 0xfedc000000000000;
    b = set_comb(e, 3, 0x111111111111) | b;
    b = set_comb(s, 2, 0x111111111111) | b;
    b = set_comb(m, 1, 0x111111111111) | b;
    l = _pdep_u64(b, 0x0f0f0f0f0f0f0f0f);
    b = b >> 32;
    h = _pdep_u64(b, 0x0f0f0f0f0f0f0f0f);
    return SET_EDGES(h, l);
}

static cube_t inverse(cube_t x)
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

static inline unsigned long long set_comb(int b, int s, unsigned long long m)
{
    unsigned long long a, r;
    a = _pdep_u64(b, m << s);
    r = _pdep_u64(0xe4, a>>(s) | a>>(s-1));
    r = s>1 ? r|a : r;
    return r;
}

////////////////////////////////////////////////////////////////////////////////

static cube_t apply_sym(cube_t x, int sym)
{
    cube_t compose_3(cube_t x, cube_t y, cube_t z)
    {
        return mirrored_compose(mirrored_compose(x, y, sym&1), z, sym&1);
    }

    return compose_3(get_sym_cube(inv_sym[sym]), x, get_sym_cube(sym));
}

static cube_t apply_move(cube_t x, int move)
{
    return compose(x, get_move_cube(move));
}

static cube_t apply_pre_move(cube_t x, int move)
{
    return compose(get_move_cube(inverse_move(move)), x);
}

static cube_t apply_moves(cube_t x, int *moves, int length)
{
    for (int i=0; i<length; ++i)
        x = apply_move(x, moves[i]);
    return x;
}
