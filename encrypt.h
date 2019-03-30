

/*
 * sturct to pack the input that is fed to a worker thread
 */
struct tip {
    char *input;
    char *key;
    long inputsize;
};

/*
 * struck to package the output to printer thread
 */
struct output{
    char *buffer;
    size_t size;
    bool end;
};

typedef struct output outpack;
typedef struct tip thread_input;

