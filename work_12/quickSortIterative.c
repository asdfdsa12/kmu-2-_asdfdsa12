#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define N 10000
#define MAXKEY 1000000
#define REPS 1000          /* 같은 조건에서 반복 측정하여 평균 사용 */
#define STACK_SIZE 128     /* (lo, hi) 쌍 개수: 큰 구간을 먼저 push 하므로 log2(N) 수준이면 충분 */

/* ---------- 난수 (xorshift64) ---------- */
static uint64_t rng_state = 88172645463325252ULL;
static uint64_t rng_next(void) {
    uint64_t x = rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    return rng_state = x;
}

/* ---------- 시간 측정 (ms 단위) ---------- */
static double now_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
#endif
}

static void swap_int(int* x, int* y) { int t = *x; *x = *y; *y = t; }

/* ---------- 피벗 선택: Median-of-Three (두 방법이 공유) ---------- */
static int pivot_median3(const int* a, int lo, int hi) {
    if (hi - lo + 1 < 3) return lo;
    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];
    if (x < y) {
        if (y < z) return mid;
        return (x < z) ? hi : lo;
    }
    else {
        if (x < z) return lo;
        return (y < z) ? hi : mid;
    }
}

/* ---------- 분할 (두 방법이 공유) ---------- */
static int partition(int* a, int lo, int hi) {
    int p = pivot_median3(a, lo, hi);
    swap_int(&a[lo], &a[p]);
    int pivot = a[lo];

    int i = lo + 1, j = hi;
    while (1) {
        while (i <= hi && a[i] < pivot) i++;
        while (j > lo && a[j] > pivot) j--;
        if (i >= j) break;
        swap_int(&a[i], &a[j]);
        i++; j--;
    }
    swap_int(&a[lo], &a[j]);
    return j;
}

/* ---------- 재귀적 퀵 정렬 ---------- */
static void quick_sort_recursive(int* a, int lo, int hi) {
    if (lo >= hi) return;
    int p = partition(a, lo, hi);
    quick_sort_recursive(a, lo, p - 1);
    quick_sort_recursive(a, p + 1, hi);
}

/* ---------- 반복적 퀵 정렬 (명시적 스택) ----------
   재귀 호출 대신 "아직 정렬해야 할 구간 (lo, hi)"을 스택에 저장한다.
   큰 구간을 먼저 push 하고 작은 구간을 나중에 push 하여(= 먼저 pop) 스택 깊이를 O(log n)으로 유지한다. */
static void quick_sort_iterative(int* a, int n) {
    int stack_lo[STACK_SIZE], stack_hi[STACK_SIZE];
    int top = -1;

    if (n < 2) return;
    top++; stack_lo[top] = 0; stack_hi[top] = n - 1;

    while (top >= 0) {
        int lo = stack_lo[top], hi = stack_hi[top];
        top--;
        if (lo >= hi) continue;

        int p = partition(a, lo, hi);

        int left_size = p - lo, right_size = hi - p;
        if (left_size > right_size) {
            /* 왼쪽이 크면 왼쪽을 먼저 push, 오른쪽(작은 쪽)을 나중에 push */
            if (left_size > 1) { top++; stack_lo[top] = lo;    stack_hi[top] = p - 1; }
            if (right_size > 1) { top++; stack_lo[top] = p + 1; stack_hi[top] = hi; }
        }
        else {
            if (right_size > 1) { top++; stack_lo[top] = p + 1; stack_hi[top] = hi; }
            if (left_size > 1) { top++; stack_lo[top] = lo;    stack_hi[top] = p - 1; }
        }
    }
}

/* ---------- 검증 ---------- */
static int is_sorted(const int* a, int n) {
    for (int i = 0; i < n - 1; i++)
        if (a[i] > a[i + 1]) return 0;
    return 1;
}

static int same_multiset(const int* x, const int* y, int n) {
    long long s1 = 0, s2 = 0, q1 = 0, q2 = 0;
    for (int i = 0; i < n; i++) {
        s1 += x[i]; s2 += y[i];
        q1 += (long long)x[i] * x[i];
        q2 += (long long)y[i] * y[i];
    }
    return s1 == s2 && q1 == q2;
}

int main(void) {
    static int orig[N], work[N], res_rec[N], res_iter[N];

    /* 원본 데이터 생성 (측정 제외) */
    for (int i = 0; i < N; i++)
        orig[i] = (int)(rng_next() % (MAXKEY + 1));

    double t_rec = 0.0, t_iter = 0.0;
    int ok = 1;

    for (int r = 0; r < REPS; r++) {
        /* 재귀: 복사(제외) -> 정렬 시간만 측정 */
        memcpy(work, orig, sizeof(orig));
        double s = now_ms();
        quick_sort_recursive(work, 0, N - 1);
        t_rec += now_ms() - s;
        if (r == 0) memcpy(res_rec, work, sizeof(work));

        /* 반복: 동일한 원본의 복사본 */
        memcpy(work, orig, sizeof(orig));
        s = now_ms();
        quick_sort_iterative(work, N);
        t_iter += now_ms() - s;
        if (r == 0) memcpy(res_iter, work, sizeof(work));
    }

    /* 결과 검증 (측정 제외): 오름차순, 값 보존, 두 방법의 결과 일치 */
    ok = is_sorted(res_rec, N) && is_sorted(res_iter, N)
        && same_multiset(orig, res_rec, N) && same_multiset(orig, res_iter, N)
        && memcmp(res_rec, res_iter, sizeof(res_rec)) == 0;

    printf("Data Size : %d\n", N);
    printf("Pivot     : Median-of-Three\n");
    printf("Repetitions : %d (average per run)\n\n", REPS);
    printf("Recursive Quick Sort : %.4f ms\n", t_rec / REPS);
    printf("Iterative Quick Sort : %.4f ms\n\n", t_iter / REPS);
    printf("Results sorted correctly and identical : %s\n", ok ? "YES" : "NO");
    return 0;
}
