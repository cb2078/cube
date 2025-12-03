static int move_type(int x)
{
    switch (x/UW)
    {
        case 0:
            return FACE_TURN;
        case 1:
            return WIDE_MOVE;
        case 2:
            return move_face(x)<3 ? ROTATION : SLICE_MOVE;
        default:
            UNREACHABLE();
    }
}

static int move_axis(int x)
{
    return x%3;
}

static int move_face(int x)
{
    return x%6;
}

static int move_opposite_face(int x)
{
    return move_face(x+3);
}

static int move_side(int x)
{
    return move_face(x)/3;
}

static int move_amount(int x)
{
    return x%UW/6+1;
}

static int make_move(int face, int amount)
{
    return face+U2*(amount-1);
}

static int inverse_move(int x)
{
    return make_move(move_face(x), 3-move_amount(x));
}

static int prune_move(int x, int y)
{
    return move_axis(x)==move_axis(y) && move_face(x)>=move_face(y);
}

static void possible_moves(int *moves, int *length, int move, int move_mask)
{
    *length = 0;
    for (int m=0; m<NUM_FACE_TURNS; ++m)
        if ((move==0xff || !prune_move(move, m)) && ~move_mask>>m&1)
            moves[(*length)++] = m;
}

static void print_moves(int *moves, int length)
{
    for (int i=0; i<length; ++i)
        printf("%s%s", i?" ":"", move_str[moves[i]]);
}

static void make_scramble(int *moves, int length)
{
    for (int i=0; i<length; i+=i==0||!prune_move(moves[i-1], moves[i]))
        moves[i]=rand()%NUM_FACE_TURNS;
}

static void read_moves(char *s, int *moves, int *length)
{
    int read_move(char *m)
    {
        for (int i=0; i<NUM_MOVES; i+=UW)
            for (int j=0; j<6; ++j)
                if (0 == strncmp(m, move_str[i+j], strlen(move_str[i+j])))
                    return i+j;
        if (s[strlen(s)-1] == '\n')
            s[strlen(s)-1] = '\0';
        ERROR("invalid input '%s'\n", s);
    }

    *length=0;
    for (int i=0; i<(long long)strlen(s);)
    {
        if (s[i] == ' ' || s[i] == '\n')
        {
            i++;
            continue;
        }
        int m = read_move(s+i);
        i += strlen(move_str[m]);
        switch (s[i])
        {
            case '1':
                i++;
                break;
            case '2':
                m += U2;
                i++;
                break;
            case '3':
            case '\'':
                m += U3;
                i++;
                break;
        }
        moves[(*length)++] = m;
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
