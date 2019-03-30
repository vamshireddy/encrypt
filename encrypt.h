

/*
 * sturct to pack the input that is fed to a worker thread
 */
struct tip {
    char *input;
    char *key;
    long inputsize;
};

/*
 *
 */
struct output{
    char *buffer;
    long size;
    bool end;
};

typedef struct output outpack;
typedef struct tip thread_input;

