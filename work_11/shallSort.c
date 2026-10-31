#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAXN 20000
#define MAXKEY 1000000

static long long g_cmp; /* 데이터 값 비교 횟수 */

/* ---------- 난수 (xorshift64) ---------- */
static uint64_t rng_state = 88172645463325252ULL;
static uint64_t rng_next(void) {
    uint64_t x = rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    return rng_state = x;
}

/* ---------- [1] 삽입 정렬 ---------- */
static void insertion_sort(int* a, int n) {
    for (int i = 1; i < n; i++) {
        int cur = a[i];
        int j = i - 1;
        while (j >= 0) {
            g_cmp++;                    /* a[j] 와 cur 의 값 비교 */
            if (a[j] > cur) {
                a[j + 1] = a[j];
                j--;
            }
            else break;
        }
        a[j + 1] = cur;
    }
}

/* 간격 gap 으로 수행하는 삽입 정렬 (쉘 정렬의 한 단계) */
static void gap_insertion(int* a, int n, int gap) {
    for (int i = gap; i < n; i++) {
        int cur = a[i];
        int j = i - gap;
        while (j >= 0) {
            g_cmp++;
            if (a[j] > cur) {
                a[j + gap] = a[j];
                j -= gap;
            }
            else break;
        }
        a[j + gap] = cur;
    }
}

/* ---------- [2] 쉘 정렬 - 기본 간격: n/2, n/4, ..., 1 ---------- */
static void shell_sort_original(int* a, int n) {
    for (int gap = n / 2; gap >= 1; gap /= 2)
        gap_insertion(a, n, gap);
}

/* ---------- [3] 쉘 정렬 - Knuth 간격: 1, 4, 13, 40, ... ---------- */
static void shell_sort_knuth(int* a, int n) {
    int h = 1;
    while (3 * h + 1 < n) h = 3 * h + 1;   /* n 보다 작은 가장 큰 Knuth 간격 */
    for (; h >= 1; h = (h - 1) / 3)         /* 역순: h(k) = (h(k+1) - 1) / 3 */
        gap_insertion(a, n, h);
}

/* ---------- 검증 ---------- */
static int is_sorted(const int* a, int n) {
    for (int i = 0; i < n - 1; i++)
        if (a[i] > a[i + 1]) return 0;
    return 1;
}

/* 정렬 결과가 원본의 순열인지(값 손실 없음) 확인: 합과 제곱합 비교 */
static int same_multiset(const int* x, const int* y, int n) {
    long long s1 = 0, s2 = 0, q1 = 0, q2 = 0;
    for (int i = 0; i < n; i++) {
        s1 += x[i]; s2 += y[i];
        q1 += (long long)x[i] * x[i];
        q2 += (long long)y[i] * y[i];
    }
    return s1 == s2 && q1 == q2;
}

typedef void (*SortFn)(int*, int);

int main(void) {
    static int orig[MAXN], work[MAXN];
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int nsizes = sizeof(sizes) / sizeof(sizes[0]);
    const char* names[3] = {
        "Insertion Sort       ",
        "Shell Sort (Original)",
        "Shell Sort (Knuth)   "
    };
    SortFn fns[3] = { insertion_sort, shell_sort_original, shell_sort_knuth };
    int allOk = 1;

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];

        /* 원본 데이터 생성 (측정 제외) */
        for (int i = 0; i < n; i++)
            orig[i] = (int)(rng_next() % (MAXKEY + 1));

        printf("Data Size : %d\n\n", n);
        for (int k = 0; k < 3; k++) {
            memcpy(work, orig, sizeof(int) * n);   /* 복사 (측정 제외) */
            g_cmp = 0;
            fns[k](work, n);
            int ok = is_sorted(work, n) && same_multiset(orig, work, n);
            if (!ok) allOk = 0;
            printf("%s: %lld comparisons%s\n", names[k], g_cmp, ok ? "" : "  [SORT FAILED]");
        }
        printf("\n");
    }

    printf("All results sorted correctly : %s\n", allOk ? "YES" : "NO");
    return 0;
}