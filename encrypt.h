struct tip {
    char *input;
    char *key;
    long inputsize;
};

struct output{
    char *buffer;
    int size;
    bool end;
};
typedef struct output outpack;
typedef struct tip thread_input;

void left_shift_key(unsigned char *existingKey, long size);
void *getXorOutput(thread_input *threadinput);
void outputData();