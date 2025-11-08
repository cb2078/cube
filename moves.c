static int move_face(int x)
{
    return x%6;
}

static int move_axis(int x)
{
    return x%3;
}

static int move_side(int x)
{
    return move_face(x)/3;
}

static int move_amount(int x)
{
    return x/U2+1;
}

static int make_move(int face, int amount)
{
    return face+U2*(amount-1);
}

static int inverse_move(int x)
{
    return make_move(move_face(x), 3-move_amount(x));
}

static int transform_move_rl2(int x)
{
    return make_move(move_face(move_axis(x)==R ? 3+x : x), 4-move_amount(x));
}

static int transform_move_f2(int x)
{
    return make_move(move_face(move_axis(x)==F ? x : x+3), move_amount(x));
}

static int transform_move_u4(int x)
{
    return make_move(move_face(move_axis(x)+x), move_amount(x));
}

static int transform_move_urf3(int x)
{
    return make_move(move_face(x)/3*3+move_axis(x+1), move_amount(x));
}

static int transform_move(int move, int sym)
{

    int (*transforms[4])(int) = {transform_move_urf3, transform_move_u4, transform_move_f2, transform_move_rl2};
    int orders[4] = {16, 4, 2, 1};
    for (int i=0; i<4; ++i)
        for (; sym>=orders[i]; sym-=orders[i])
            move = transforms[i](move);
    return move;
}

static void transform_moves(int *moves, int length, int sym)
{
    for (int i=0; i<length; ++i)
        moves[i] = transform_move(moves[i], sym);
}

static int prune_move(int x, int y)
{
    return move_axis(x)==move_axis(y) && move_face(x)>=move_face(y);
}

static void possible_moves(int *moves, int *length, int move, int move_mask)
{
    *length = 0;
    for (int i=0; i<LENGTH(move_set); ++i)
        if ((move==0xff || !prune_move(move, move_set[i])) && ~move_mask>>i&1)
            moves[(*length)++] = move_set[i];
}

static void print_moves(int *moves, int length)
{
    for (int i=0; i<length; ++i) printf("%s%s", i?" ":"", move_str[moves[i]]);
}

static void make_scramble(int *moves, int length)
{
    for (int i=0; i<length; i+=i==0||!prune_move(moves[i-1], moves[i]))
        moves[i]=move_set[rand()%LENGTH(move_set)];
}

static void read_moves(char *s, int *moves, int *length)
{
    *length=0;
    for (int i=0; i<(int)strlen(s);)
    {
        int n=0;
        while (s[i+n]!=' ' && s[i+n]!='\0')
            ++n;
        int j=0;
        for (; j<NUM_MOVES; ++j)
            if (0 == strncmp(s+i, move_str[j], n))
                break;
        if (j==NUM_MOVES)
        {
            printf("invalid scramble\n%d\n%s\n", i, s+i);
            exit(1);
        }
        moves[(*length)++] = j;
        i += 1+n;
    }
}

static int apply_cancellations(int *moves, int *length)
{
    int cancelled=0;
    for (int i=*length-2; i>=0; --i)
    {
        if (!prune_move(moves[i], moves[i+1]))
            continue;
        if (move_face(moves[i])==move_face(moves[i+1]))
        {
            int amount=(move_amount(moves[i])+move_amount(moves[i+1]))%4;
            if (amount)
            {
                moves[i]=move_face(moves[i])+U2*(amount-1);
                memmove(&moves[i+1], &moves[i+2], sizeof(int)*(*length-(i+2)));
                *length-=1;
            }
            else
            {
                memmove(&moves[i], &moves[i+2], sizeof(int)*(*length-(i+2)));
                *length-=2;
            }
        }
        else
        {
            SWAP(moves[i], moves[i+1]);
        }
        cancelled=1;
    }
    if (cancelled) apply_cancellations(moves, length);
    return cancelled;
}

static void inverse_moves(int *moves, int length)
{
    for (int i=0; i<length; ++i) moves[i]=inverse_move(moves[i]);
    for (int i=0, j=length-1; i<j; ++i, --j) SWAP(moves[i], moves[j]);
}
