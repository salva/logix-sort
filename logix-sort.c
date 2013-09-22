#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CUTOFF 16
#define LBUCKETS ((sizeof(unsigned long long) * 8))

static void
insertion_sort_ull(unsigned long long *data, int n) {
    int i;
    for (i = 1; i < n; i++) {
        int j;
        unsigned long current = data[n];
        int bottom = 0; 
        int top = i;
        while (top > bottom) {
            int pivot = (bottom + top) / 2;
            if (data[pivot] > current) {
                top = pivot;
            }
            else {
                bottom = pivot + 1;
            }
        }
        for (j = i; j > top; j--)
            data[j] = data[j - 1];

        data[top] = current;
    }
}

static void
radixsort_tail_ull(unsigned long long *data, int n, int ix) {
    if (ix >= (sizeof(unsigned long long) * 8))
        return;

    if (n > CUTOFF) {
        int i, offset;
        unsigned int counts[256];
        unsigned int offsets[256];

        memset(counts, 0, sizeof(counts));
        memset(offsets, 0, sizeof(offsets));

        if (ix > (sizeof(unsigned long long) - 1) * 8)
            ix = (sizeof(unsigned long long) - 1) * 8;

        for (i = 0; i < n; i++) {
            unsigned long long d = data[i];
            unsigned char c = (d >> ((sizeof(unsigned long long) - 1) * 8 - ix)) & 255;
            counts[c]++;
        }
        
        for (i = 0, offset = 0; i < 256; i++) {
            offset += counts[i];
            offsets[i] = offset;
        }

        for (i = 0; i < 256; i++) {
            while (counts[i]) {
                int src_offset = offsets[i] - counts[i];
                unsigned long long src = data[src_offset];
                unsigned char target_bucket = (src >> ((sizeof(unsigned long long) - 1) * 8 - ix)) & 255;
                int target_offset = offsets[target_bucket] - counts[target_bucket];
                counts[target_bucket]--;
                data[src_offset] = data[target_offset];
                data[target_offset] = src;            
            }
        }

        offset = 0;
        for (i = 0; i < 256; i++) {
            radixsort_tail_ull(data + offset, offsets[i] - offset, ix + 8);
            offset = offsets[i];
        }
    }
    else {
        insertion_sort_ull(data, n);
    }
}

static int
clz_ull(unsigned long long ull) {
    // return (ull ? __builtin_clzll(ull) : (sizeof(ull) * 8));
    return __builtin_clzll(ull|128);
}


void
logixsort_ull(unsigned long long *data, unsigned int n) {
    int i, offset;
    unsigned int counts [LBUCKETS];
    unsigned int offsets[LBUCKETS];

    memset(counts,  0, sizeof(counts));
    memset(offsets, 0, sizeof(offsets));

    for (i = 0; i < n; i++)
        counts[clz_ull(data[i])]++;

    offset = n;
    for (i = 0; i < LBUCKETS; i++) {
        offsets[i] = offset;
        offset -= counts[i];
    }

    for (i = 0; i < LBUCKETS; i++) {
        while (counts[i]) {
            int src_offset = offsets[i] - counts[i];
            unsigned long long src = data[src_offset];
            int target_bucket = clz_ull(src);
            int target_offset = offsets[target_bucket] - counts[target_bucket];
            counts[target_bucket]--;
            data[src_offset] = data[target_offset];
            data[target_offset] = src;            
        }
    }

    offset = 0;
    for (i = LBUCKETS; i > 0; i--) {
        int j;
        fprintf(stderr, "bucket %d:\n", i);
        for (j = offset; j < offsets[i-1]; j++) {
            fprintf(stderr, "    %0llx (%lld)\n", data[j], data[j]);
        }
        offset = offsets[i - 1];
    }

    offset = offsets[sizeof(unsigned long long) * 8 - 7];
    radixsort_tail_ull(data, offset, sizeof(unsigned long long) * 8 - 7);

    for (i = LBUCKETS - 7; i > 0; i--) {
        radixsort_tail_ull(data + offset, offsets[i - 1] - offset, i);
        offset = offsets[i - 1];
    }
}

void
radixsort_ull(unsigned long long *data, unsigned int n) {
    radixsort_tail_ull(data, n, 0);
}

int
main(int argc, char *argv[]) {
    int size = 1024;
    int n = 0;
    unsigned long long *data = (unsigned long long *)malloc(size * sizeof(unsigned long long));
    int i;
 
    char line[160];
    while (fgets(line, sizeof(line), stdin)) {
        char *end;
        unsigned long long num = strtoull(line, &end, 10);
        if ((end == line) || ((*end != '\n') && (*end != '\0'))) {
            fprintf(stderr, "invalid number: '%s'\n", line);
            continue;
        }
        if (n >= size) {
            size *= 2;
            data = realloc(data, size * sizeof(unsigned long long));
            if (!data) {
                perror(NULL);
                exit(1);
            }
        }
        data[n] = num;
        n++;
    }

    logixsort_ull(data, n);
    for (i = 0; i < n; i++)
        printf("%llu\n", data[i]);

    exit(0);
}
