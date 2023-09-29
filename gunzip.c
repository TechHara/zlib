// A simple command line program in C for Linux that reads a .gz file from stdin and decompresses and writes to stdout
// Make use of zlib library

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

#define CHUNK 16384 // Buffer size

// Decompress from source to dest until EOF on source
int decompress(FILE *source, FILE *dest) {
    int ret; // Return value
    unsigned have; // Number of bytes in output buffer
    z_stream strm; // Stream structure
    unsigned char in[CHUNK]; // Input buffer
    unsigned char out[CHUNK]; // Output buffer

    // Initialize stream structure
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    // Initialize inflation
    ret = inflateInit2(&strm, 15 + 32); // 15 + 32 to enable gzip decoding
    if (ret != Z_OK) {
        return ret; // Error occurred
    }

    // Inflate until end of input
    do {
        // Read from source into input buffer
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm); // Clean up
            return Z_ERRNO; // Error occurred
        }
        if (strm.avail_in == 0) {
            break; // End of input
        }
        strm.next_in = in;

        // Inflate into output buffer until output buffer is full or end of stream is reached
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH); // No flush
            if (ret == Z_STREAM_ERROR) {
                return ret; // Error occurred
            }
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR; // Fall through
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm); // Clean up
                    return ret; // Error occurred
            }
            have = CHUNK - strm.avail_out; // Number of bytes in output buffer

            // Write output buffer to dest
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm); // Clean up
                return Z_ERRNO; // Error occurred
            }
        } while (strm.avail_out == 0);

        // Done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // Clean up and return
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

// Main function
int main(int argc, char **argv) {
    int ret; // Return value

    // Check arguments
    if (argc != 1) {
        fprintf(stderr, "Usage: %s < input.gz > output\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Decompress from stdin to stdout
    ret = decompress(stdin, stdout);
    
    // Check for errors and exit accordingly
    if (ret != Z_OK) {
        fprintf(stderr, "An error occurred while decompressing: %d\n", ret);
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, "Decompression successful\n");
        exit(EXIT_SUCCESS);
    }
}

