#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    unsigned long long comparisons;
    unsigned long long moves;
    int max_depth;
    size_t extra_heap_bytes;
} Metrics;

typedef struct {
    int key;
    int original_index;
} Record;

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

static void swap_int(int *a, int *b, Metrics *m) {
    int temp = *a;
    *a = *b;
    *b = temp;
    m->moves += 3;
}

static void insertion_sort(int *a, int n, Metrics *m) {
    for (int i = 1; i < n; ++i) {
        int key = a[i];
        int j = i - 1;
        m->moves++;
        while (j >= 0) {
            m->comparisons++;
            if (a[j] <= key) break;
            a[j + 1] = a[j];
            m->moves++;
            --j;
        }
        a[j + 1] = key;
        m->moves++;
    }
}

static int partition_lomuto(int *a, int low, int high, Metrics *m) {
    int pivot = a[high];
    int i = low - 1;
    m->moves++;
    for (int j = low; j < high; ++j) {
        m->comparisons++;
        if (a[j] <= pivot) {
            ++i;
            if (i != j) swap_int(&a[i], &a[j], m);
        }
    }
    if (i + 1 != high) swap_int(&a[i + 1], &a[high], m);
    return i + 1;
}

static void quick_sort_recursive(int *a, int low, int high, Metrics *m, int depth) {
    if (depth > m->max_depth) m->max_depth = depth;
    if (low < high) {
        int pivot_index = partition_lomuto(a, low, high, m);
        quick_sort_recursive(a, low, pivot_index - 1, m, depth + 1);
        quick_sort_recursive(a, pivot_index + 1, high, m, depth + 1);
    }
}

static void quick_sort(int *a, int n, Metrics *m) {
    quick_sort_recursive(a, 0, n - 1, m, 1);
}

static void merge_ranges(int *a, int *temp, int left, int mid, int right,
                         Metrics *m) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        m->comparisons++;
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
        m->moves++;
    }
    while (i <= mid) {
        temp[k++] = a[i++];
        m->moves++;
    }
    while (j <= right) {
        temp[k++] = a[j++];
        m->moves++;
    }
    for (int p = left; p <= right; ++p) {
        a[p] = temp[p];
        m->moves++;
    }
}

static void merge_sort_recursive(int *a, int *temp, int left, int right,
                                 Metrics *m, int depth) {
    if (depth > m->max_depth) m->max_depth = depth;
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_recursive(a, temp, left, mid, m, depth + 1);
        merge_sort_recursive(a, temp, mid + 1, right, m, depth + 1);
        merge_ranges(a, temp, left, mid, right, m);
    }
}

static int merge_sort(int *a, int n, Metrics *m) {
    int *temp = malloc((size_t)n * sizeof(*temp));
    if (!temp) return 0;
    m->extra_heap_bytes = (size_t)n * sizeof(*temp);
    merge_sort_recursive(a, temp, 0, n - 1, m, 1);
    free(temp);
    return 1;
}

static unsigned int xorshift32(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static void fill_data(int *a, int n, const char *pattern, unsigned int seed) {
    if (strcmp(pattern, "sorted") == 0) {
        for (int i = 0; i < n; ++i) a[i] = i;
    } else if (strcmp(pattern, "reversed") == 0) {
        for (int i = 0; i < n; ++i) a[i] = n - i;
    } else if (strcmp(pattern, "duplicates") == 0) {
        for (int i = 0; i < n; ++i) a[i] = (int)(xorshift32(&seed) % 20u);
    } else {
        for (int i = 0; i < n; ++i) a[i] = (int)(xorshift32(&seed) & 0x7fffffffu);
    }
}

static int is_sorted(const int *a, int n) {
    for (int i = 1; i < n; ++i) {
        if (a[i - 1] > a[i]) return 0;
    }
    return 1;
}

static void run_one(const char *algorithm, const int *base, int n,
                    const char *pattern) {
    int *work = malloc((size_t)n * sizeof(*work));
    if (!work) {
        fprintf(stderr, "memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(work, base, (size_t)n * sizeof(*work));

    Metrics metrics = {0};
    double start = now_seconds();
    int success = 1;

    if (strcmp(algorithm, "Insertion") == 0) insertion_sort(work, n, &metrics);
    else if (strcmp(algorithm, "Quick") == 0) quick_sort(work, n, &metrics);
    else success = merge_sort(work, n, &metrics);

    double elapsed_ms = (now_seconds() - start) * 1000.0;
    if (!success || !is_sorted(work, n)) {
        fprintf(stderr, "%s sort verification failed\n", algorithm);
        free(work);
        exit(EXIT_FAILURE);
    }

    printf("RESULT,%s,%s,%d,%.6f,%llu,%llu,%d,%zu\n",
           algorithm, pattern, n, elapsed_ms, metrics.comparisons,
           metrics.moves, metrics.max_depth, metrics.extra_heap_bytes);
    free(work);
}

static void insertion_sort_records(Record *a, int n) {
    for (int i = 1; i < n; ++i) {
        Record key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j].key > key.key) {
            a[j + 1] = a[j];
            --j;
        }
        a[j + 1] = key;
    }
}

static void merge_record_ranges(Record *a, Record *temp, int left, int mid,
                                int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (a[i].key <= a[j].key) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i <= mid) temp[k++] = a[i++];
    while (j <= right) temp[k++] = a[j++];
    for (int p = left; p <= right; ++p) a[p] = temp[p];
}

static void merge_sort_records_recursive(Record *a, Record *temp, int left,
                                         int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_records_recursive(a, temp, left, mid);
        merge_sort_records_recursive(a, temp, mid + 1, right);
        merge_record_ranges(a, temp, left, mid, right);
    }
}

static void merge_sort_records(Record *a, int n) {
    Record *temp = malloc((size_t)n * sizeof(*temp));
    if (!temp) exit(EXIT_FAILURE);
    merge_sort_records_recursive(a, temp, 0, n - 1);
    free(temp);
}

static int partition_records(Record *a, int low, int high) {
    Record pivot = a[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (a[j].key <= pivot.key) {
            ++i;
            if (i != j) {
                Record temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
    }
    if (i + 1 != high) {
        Record temp = a[i + 1];
        a[i + 1] = a[high];
        a[high] = temp;
    }
    return i + 1;
}

static void quick_sort_records_recursive(Record *a, int low, int high) {
    if (low < high) {
        int pivot = partition_records(a, low, high);
        quick_sort_records_recursive(a, low, pivot - 1);
        quick_sort_records_recursive(a, pivot + 1, high);
    }
}

static int stability_violations(const Record *a, int n) {
    int count = 0;
    for (int i = 1; i < n; ++i) {
        if (a[i - 1].key == a[i].key &&
            a[i - 1].original_index > a[i].original_index) ++count;
    }
    return count;
}

static void run_stability_test(void) {
    const int n = 1000;
    Record *base = malloc((size_t)n * sizeof(*base));
    Record *work = malloc((size_t)n * sizeof(*work));
    unsigned int seed = 20260921u;
    if (!base || !work) exit(EXIT_FAILURE);

    for (int i = 0; i < n; ++i) {
        base[i].key = (int)(xorshift32(&seed) % 20u);
        base[i].original_index = i;
    }

    memcpy(work, base, (size_t)n * sizeof(*work));
    insertion_sort_records(work, n);
    printf("STABILITY,Insertion,%d\n", stability_violations(work, n));

    memcpy(work, base, (size_t)n * sizeof(*work));
    quick_sort_records_recursive(work, 0, n - 1);
    printf("STABILITY,Quick,%d\n", stability_violations(work, n));

    memcpy(work, base, (size_t)n * sizeof(*work));
    merge_sort_records(work, n);
    printf("STABILITY,Merge,%d\n", stability_violations(work, n));

    free(base);
    free(work);
}

int main(void) {
    const int sizes[] = {1000, 5000, 10000};
    const char *patterns[] = {"random", "sorted", "reversed", "duplicates"};
    const char *algorithms[] = {"Insertion", "Quick", "Merge"};

    puts("TYPE,algorithm,pattern,n,time_ms,comparisons,moves,max_depth,extra_heap_bytes");
    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
        int n = sizes[s];
        int *base = malloc((size_t)n * sizeof(*base));
        if (!base) return EXIT_FAILURE;

        for (size_t p = 0; p < sizeof(patterns) / sizeof(patterns[0]); ++p) {
            fill_data(base, n, patterns[p], 20260921u + (unsigned int)n);
            for (size_t a = 0; a < sizeof(algorithms) / sizeof(algorithms[0]); ++a) {
                run_one(algorithms[a], base, n, patterns[p]);
            }
        }
        free(base);
    }

    run_stability_test();
    return EXIT_SUCCESS;
}
