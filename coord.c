static char *get_pieces(struct coord *c, cube_t *x)
{
    char *table[] =
    {
        [CORNERS] = x->corners,
        [URF_TETRAD] = x->urf_tetrad,
        [URB_TETRAD] = x->urb_tetrad,
        [EDGES] = x->edges,
        [UD_SLICE] = x->ud_slice,
        [RL_SLICE] = x->rl_slice,
        [FB_SLICE] = x->fb_slice,
    };
    return table[c->pieces] + c->offset;
}

static int end_index(struct coord *c)
{
    return (c->pieces==CORNERS ? 8 : c->pieces==EDGES ? 12 : 4) - c->offset;
}

static void offset(struct coord *c, char *pieces, int sign)
{
    int table[] =
    {
        [CORNERS] = 0,
        [URF_TETRAD] = 0,
        [URB_TETRAD] = 4,
        [EDGES] = 0,
        [UD_SLICE] = 0,
        [RL_SLICE] = 4,
        [FB_SLICE] = 8,
    };
    for (int i=0; i<end_index(c); ++i)
        pieces[i] += sign*(c->offset+table[c->pieces]);
}

static void add_offset(struct coord *c, char *pieces)
{
    offset(c, pieces, +1);
}

static void sub_offset(struct coord *c, char *pieces)
{
    offset(c, pieces, -1);
}

static long long coord_get(struct coord *c, cube_t x)
{
    ASSERT(c->max);
    long long r = 0;
    switch (c->type)
    {
        case RAW:
            if (c->indexer==ORIENTATION)
            {
                switch (c->pieces)
                {
                    case EDGES:
                        return get_eo(x);
                    case CORNERS:
                        return get_co(x);
                    default:
                        UNREACHABLE();
                }
            }
            else
            {
                // TODO when optimising see if this is repeated more than once
                for (int i=0; i<LENGTH(x.cubies); ++i)
                    x.cubies[i] &= 0x0f;
                char *pieces = get_pieces(c, &x);
                sub_offset(c, pieces);
                switch (c->indexer)
                {
                    case PERMUTATION:
                        return get_permutation(pieces, end_index(c));
                    case PARTIAL_PERMUTATION:
                        return get_partial_permutation(pieces, end_index(c), c->length);
                    case COMBINATION:
                        return get_combination(pieces, end_index(c), c->length);
                    default:
                        UNREACHABLE();
                }
            }
        case COMP:
            for (int i=0; i<c->count; ++i)
            {
                r *= c->coords[i].max;
                r += coord_get(&c->coords[i], x);
            }
            return r;
        case SYM_COMP:
            r = coord_get(&c->coords[0], x);
            x = apply_sym(x, c->coord_info[r].sym);
            r = c->coord_info[r].class;
            return r * c->coords[1].max + coord_get(&c->coords[1], x);
        default:
            UNREACHABLE();
    }
}

static cube_t coord_set(struct coord *c, long long r)
{
    ASSERT(c->max);
    cube_t x = new_cube();
    switch (c->type)
    {
        case RAW:
            if (c->indexer==ORIENTATION)
            {
                switch (c->pieces)
                {
                    case CORNERS:
                        set_co(&x, r);
                        break;
                    case EDGES:
                        set_eo(&x, r);
                        break;
                    default:
                        UNREACHABLE();
                }
                return x;
            }
            else
            {
                char *pieces = get_pieces(c, &x);
                switch (c->indexer)
                {
                    case PERMUTATION:
                        set_permutation(pieces, end_index(c), r);
                        break;
                    case PARTIAL_PERMUTATION:
                        set_partial_permutation(pieces, end_index(c), c->length, r);
                        break;
                    case COMBINATION:
                        set_combination(pieces, end_index(c), c->length, r);
                        break;
                    default:
                        UNREACHABLE();
                }
                add_offset(c, pieces);
                return x;
            }
        case COMP:
            for (int i=c->count; i-->0;)
            {
                x = compose(x, coord_set(&c->coords[i], r%c->coords[i].max));
                r /= c->coords[i].max;
            }
            ASSERT(!r);
            return x;
        case SYM_COMP:
            x = compose(x, coord_set(&c->coords[0], c->rep[r/c->coords[1].max]));
            x = compose(x, coord_set(&c->coords[1], r%c->coords[1].max));
            return x;
        default:
            UNREACHABLE();
    }
}

#define CLASS_TO_REP_SIZE(c) (c->classes * sizeof(c->rep[0]))
#define COORD_INFO_SIZE(c) (c->coords[0].max * sizeof(c->coord_info[0]))
#define COORD_SELF_SYMS_SIZE(c) (c->coords[0].max * sizeof(c->self_syms[0]))
#define COORD_SIZE(c) (CLASS_TO_REP_SIZE(c) + COORD_INFO_SIZE(c) + COORD_SELF_SYMS_SIZE(c))

#define COORD_EXTENSION ".sym_info.bin"
#define COORD_HAS_EXTENSION(c) strstr(c->name, COORD_EXTENSION)

static int coord_read(struct coord *c)
{
    ASSERT(!c->rep);
    void *mem = calloc(COORD_SIZE(c), 1);
    c->rep    = mem;
    c->coord_info      = mem + CLASS_TO_REP_SIZE(c);
    c->self_syms = mem + CLASS_TO_REP_SIZE(c) + COORD_INFO_SIZE(c);

    ASSERT(c->name);
    ASSERT(!COORD_HAS_EXTENSION(c));
    strcat(c->name, COORD_EXTENSION);
    FILE *f = fopen(c->name, "rb");
    if (f)
    {
        fread(mem, COORD_SIZE(c), 1, f);
        fclose(f);
        fprintf(stderr, "read '%s'\n", c->name);
        return 1;
    }
    else
    {
        return 0;
    }
}

static int coord_write(struct coord *c)
{
    ASSERT(c->rep);
    void *mem = c->rep;

    ASSERT(c->name);
    ASSERT(COORD_HAS_EXTENSION(c));
    FILE *f = fopen(c->name, "wb"); // TODO filename for tablers is wrong
    if (f)
    {
        fwrite(mem, COORD_SIZE(c), 1, f);
        fclose(f);
        fprintf(stderr, "wrote '%s'\n", c->name);
        return 1;
    }
    else
    {
        fprintf(stderr, "couldn't write '%s'\n", c->name);
        return 0;
    }
}

// TODO maybe move this to another file
static void init_sym(struct coord *c)
{
    if (c->type!=SYM_COMP)
        return;
    if (coord_read(c))
        return;

    struct coord *b = &c->coords[0];
    for (long long i=0; i<b->max; ++i)
    {
        cube_t x = coord_set(b, i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = coord_get(b, y);
            c->self_syms[i] |= (i==k)<<s;
        }
    }

    int class = 0;
    memset(c->coord_info, 0xff, COORD_INFO_SIZE(c));
    for (long long i=0; i<b->max; ++i)
    {
        if (c->coord_info[i].class != 0xffff) // TODO change "class" to just "class"
            continue;
        cube_t x = coord_set(b, i);
        for (int s=0; s<c->num_syms; ++s)
        {
            cube_t y = apply_sym(x, s);
            int k = coord_get(b, y);
            c->coord_info[k].class = class;
            c->coord_info[k].sym = inv_sym[s];
        }
        c->rep[class++] = i;
    }
    ASSERT(class == c->classes);
    ASSERT(class*c->coords[1].max == c->max);

    coord_write(c);
}
