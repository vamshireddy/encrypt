#!/bin/sh


echo "01020304"> key_file

echo "0102030405060708090a">input

encrypt - n 4 -k "key_file <input >output

cat output


# 1. Input length is multiple of key length
# 2. Input length is multiple of key length
# 3. Not all threads have enough data to consume (i.e. input lenght < no of threads * key_length)
# 4. All threads have exactly equal amount of buffres (i.e. input lenght = no of threads * key_length)
# 5. i.e. input lenght > no of threads * key_length)


