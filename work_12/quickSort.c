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

static void swap_int(int *x, int *y) { int t = *x; *x = *y; *y = t; }

/* ---------- 피벗 선택: 선택한 피벗의 인덱스를 반환 ---------- */
typedef int (*PivotFn)(const int *a, int lo, int hi);

/* [1] 첫 번째 원소 */
static int pivot_first(const int *a, int lo, int hi) {
    (void)a; (void)hi;
    return lo;
}

/* [2] 무작위 원소: 분할할 때마다 새로 선택 (위치 생성은 비교 횟수에 미포함) */
static int pivot_random(const int *a, int lo, int hi) {
    (void)a;
    return lo + (int)(rng_next() % (uint64_t)(hi - lo + 1));
}

/* [3] 첫/가운데/마지막 세 값의 중앙값 (중앙값 결정을 위한 비교도 횟수에 포함) */
static int pivot_median3(const int *a, int lo, int hi) {
    if (hi - lo + 1 < 3) return lo;          /* 원소가 3개 미만이면 첫 원소 */
    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];
    g_cmp++;
    if (x < y) {
        g_cmp++;
        if (y < z) return mid;               /* x < y < z */
        g_cmp++;
        return (x < z) ? hi : lo;            /* x < z <= y  또는  z <= x < y */
    } else {
        g_cmp++;
        if (x < z) return lo;                /* y <= x < z */
        g_cmp++;
        return (y < z) ? hi : mid;           /* y < z <= x  또는  z <= y <= x */
    }
}

/* ---------- 분할 (세 방법 모두 동일) ----------
   피벗을 a[lo]로 옮긴 뒤, 양쪽에서 좁혀 오며 피벗과 비교한다. */
static int partition(int *a, int lo, int hi, PivotFn choose) {
    int p = choose(a, lo, hi);
    swap_int(&a[lo], &a[p]);                 /* 데이터 이동: 비교 횟수에 미포함 */
    int pivot = a[lo];

    int i = lo + 1, j = hi;
    while (1) {
        while (i <= hi) {                    /* 피벗보다 크거나 같은 값을 찾음 */
            g_cmp++;
            if (a[i] < pivot) i++;
            else break;
        }
        while (j > lo) {                     /* 피벗보다 작거나 같은 값을 찾음 */
            g_cmp++;
            if (a[j] > pivot) j--;
            else break;
        }
        if (i >= j) break;
        swap_int(&a[i], &a[j]);
        i++; j--;
    }
    swap_int(&a[lo], &a[j]);                 /* 피벗을 제자리로 */
    return j;
}

static void quick_sort(int *a, int lo, int hi, PivotFn choose) {
    if (lo >= hi) return;
    int p = partition(a, lo, hi, choose);
    quick_sort(a, lo, p - 1, choose);
    quick_sort(a, p + 1, hi, choose);
}

/* ---------- 검증 ---------- */
static int is_sorted(const int *a, int n) {
    for (int i = 0; i < n - 1; i++)
        if (a[i] > a[i + 1]) return 0;
    return 1;
}

static int same_multiset(const int *x, const int *y, int n) {
    long long s1 = 0, s2 = 0, q1 = 0, q2 = 0;
    for (int i = 0; i < n; i++) {
        s1 += x[i]; s2 += y[i];
        q1 += (long long)x[i] * x[i];
        q2 += (long long)y[i] * y[i];
    }
    return s1 == s2 && q1 == q2;
}

int main(void) {
    static int orig[MAXN], work[MAXN];
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int nsizes = sizeof(sizes) / sizeof(sizes[0]);
    const char *names[3] = {
        "Quick Sort (First)   ",
        "Quick Sort (Random)  ",
        "Quick Sort (Median-3)"
    };
    PivotFn fns[3] = { pivot_first, pivot_random, pivot_median3 };
    int allOk = 1;

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];

        /* 원본 데이터 생성 (측정 제외, 중복 허용) */
        for (int i = 0; i < n; i++)
            orig[i] = (int)(rng_next() % (MAXKEY + 1));

        printf("Data Size : %d\n\n", n);
        for (int k = 0; k < 3; k++) {
            memcpy(work, orig, sizeof(int) * n);   /* 복사 (측정 제외) */
            g_cmp = 0;
            quick_sort(work, 0, n - 1, fns[k]);
            int ok = is_sorted(work, n) && same_multiset(orig, work, n);
            if (!ok) allOk = 0;
            printf("%s : %lld comparisons%s\n", names[k], g_cmp, ok ? "" : "  [SORT FAILED]");
        }
        printf("\n");
    }

    printf("All results sorted correctly : %s\n", allOk ? "YES" : "NO");
    return 0;
}
