#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define MAXN 20000
#define MAXKEY 1000000
#define WORK_BUDGET 2000000   /* 반복 횟수 = WORK_BUDGET / n  (작은 n일수록 많이 반복하여 평균) */

/* ---------- 측정용 전역 변수 ---------- */
static long long g_cmp;            /* 숫자 비교 횟수 */
static long long g_heap;           /* 정렬을 위해 추가로 할당한 힙 메모리(Byte) */
static uintptr_t g_stack_base;     /* 정렬 시작 시점의 스택 기준 주소 */
static uintptr_t g_stack_max;      /* 기준 주소로부터 가장 깊게 내려간 스택 사용량(Byte) */
static int g_depth, g_depth_max;   /* 현재/최대 재귀 깊이 */

/* 재귀 함수 진입 시 호출: 지역 변수의 주소로 현재 스택 사용량을 계산해 최댓값을 갱신 */
static void track_stack(const char* probe) {
    uintptr_t addr = (uintptr_t)probe;
    uintptr_t used = (g_stack_base > addr) ? g_stack_base - addr : addr - g_stack_base;
    if (used > g_stack_max) g_stack_max = used;
}
static void begin_measure(uintptr_t base) {
    g_cmp = 0; g_heap = 0;
    g_stack_base = base; g_stack_max = 0;
    g_depth = 0; g_depth_max = 0;
}

/* ---------- 난수 (xorshift64) ---------- */
static uint64_t rng_state = 88172645463325252ULL;
static uint64_t rng_next(void) {
    uint64_t x = rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    return rng_state = x;
}

/* ---------- 시간 측정 (ms) ---------- */
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

/* =====================================================
   [1] 힙 정렬 (최대 힙, 입력 배열 자체를 힙으로 사용)
   ===================================================== */
   /* a[i]를 루트로 하는 서브트리를 크기 n 힙 기준으로 복구 (반복문 방식, 재귀 없음) */
static void sift_down(int* a, int i, int n) {
    char probe;
    track_stack(&probe);                      /* 호출 스택 사용량 측정 */
    while (1) {
        int l = 2 * i + 1;
        if (l >= n) break;
        int c = l;                            /* 더 큰 자식의 인덱스 */
        if (l + 1 < n) {
            g_cmp++;                          /* 두 자식끼리 비교 */
            if (a[l + 1] > a[l]) c = l + 1;
        }
        g_cmp++;                              /* 부모와 더 큰 자식 비교 */
        if (a[c] > a[i]) {
            swap_int(&a[c], &a[i]);
            i = c;
        }
        else break;
    }
}

static void heap_sort(int* a, int n) {
    char base;
    begin_measure((uintptr_t)&base);          /* 힙 정렬은 추가 배열 없음: 힙 메모리 0 */
    for (int i = n / 2 - 1; i >= 0; i--)      /* 최대 힙 구성 */
        sift_down(a, i, n);
    for (int end = n - 1; end > 0; end--) {   /* 최댓값을 뒤로 보내고 힙 크기 축소 */
        swap_int(&a[0], &a[end]);
        sift_down(a, 0, end);
    }
}

/* =====================================================
   [2] 병합 정렬 (Top-down)
   ===================================================== */
static void merge_rec(int* a, int* tmp, int lo, int hi) {
    char probe;
    track_stack(&probe);
    if (++g_depth > g_depth_max) g_depth_max = g_depth;

    if (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        merge_rec(a, tmp, lo, mid);
        merge_rec(a, tmp, mid + 1, hi);

        /* 병합: 구간을 임시 배열로 복사한 뒤 a에 정렬하며 되돌려 씀 */
        memcpy(tmp + lo, a + lo, sizeof(int) * (size_t)(hi - lo + 1));
        int i = lo, j = mid + 1, k = lo;
        while (i <= mid && j <= hi) {
            g_cmp++;                                   /* 값 비교 */
            if (tmp[i] <= tmp[j]) a[k++] = tmp[i++];
            else                  a[k++] = tmp[j++];
        }
        while (i <= mid) a[k++] = tmp[i++];
        while (j <= hi)  a[k++] = tmp[j++];
    }
    g_depth--;
}

static void merge_sort(int* a, int n) {
    char base;
    begin_measure((uintptr_t)&base);
    int* tmp = (int*)malloc(sizeof(int) * (size_t)n);   /* 추가 메모리: 임시 배열 */
    g_heap = (long long)sizeof(int) * n;
    merge_rec(a, tmp, 0, n - 1);
    free(tmp);
}

/* =====================================================
   [3] 퀵 정렬 (재귀) - 피벗 선택 3종
   ===================================================== */
typedef int (*PivotFn)(const int* a, int lo, int hi);

static int pivot_first(const int* a, int lo, int hi) {
    (void)a; (void)hi;
    return lo;
}

static int pivot_random(const int* a, int lo, int hi) {
    (void)a;
    return lo + (int)(rng_next() % (uint64_t)(hi - lo + 1));   /* 위치 생성은 비교 아님 */
}

static int pivot_median3(const int* a, int lo, int hi) {
    if (hi - lo + 1 < 3) return lo;
    int mid = lo + (hi - lo) / 2;
    int x = a[lo], y = a[mid], z = a[hi];
    g_cmp++;
    if (x < y) {
        g_cmp++;
        if (y < z) return mid;
        g_cmp++;
        return (x < z) ? hi : lo;
    }
    else {
        g_cmp++;
        if (x < z) return lo;
        g_cmp++;
        return (y < z) ? hi : mid;
    }
}

static int partition(int* a, int lo, int hi, PivotFn choose) {
    int p = choose(a, lo, hi);
    swap_int(&a[lo], &a[p]);
    int pivot = a[lo];

    int i = lo + 1, j = hi;
    while (1) {
        while (i <= hi) {
            g_cmp++;
            if (a[i] < pivot) i++;
            else break;
        }
        while (j > lo) {
            g_cmp++;
            if (a[j] > pivot) j--;
            else break;
        }
        if (i >= j) break;
        swap_int(&a[i], &a[j]);
        i++; j--;
    }
    swap_int(&a[lo], &a[j]);
    return j;
}

static void quick_rec(int* a, int lo, int hi, PivotFn choose) {
    char probe;
    track_stack(&probe);
    if (++g_depth > g_depth_max) g_depth_max = g_depth;

    if (lo < hi) {
        int p = partition(a, lo, hi, choose);
        quick_rec(a, lo, p - 1, choose);
        quick_rec(a, p + 1, hi, choose);
    }
    g_depth--;
}

static void quick_first(int* a, int n) { char base; begin_measure((uintptr_t)&base); quick_rec(a, 0, n - 1, pivot_first); }
static void quick_random(int* a, int n) { char base; begin_measure((uintptr_t)&base); quick_rec(a, 0, n - 1, pivot_random); }
static void quick_median3(int* a, int n) { char base; begin_measure((uintptr_t)&base); quick_rec(a, 0, n - 1, pivot_median3); }

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

typedef void (*SortFn)(int*, int);

#define NALGO 5

typedef struct {
    long long cmp;       /* 첫 번째 실행의 비교 횟수 */
    double    time_ms;   /* 반복 평균 실행시간 */
    long long heap;      /* 추가 힙 메모리 */
    long long stack;     /* 측정된 최대 스택 사용량 */
    int       depth;     /* 최대 재귀 깊이 */
} Result;

int main(void) {
    static int orig[MAXN], work[MAXN];
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int nsizes = sizeof(sizes) / sizeof(sizes[0]);
    const char* names[NALGO] = { "Heap Sort      ", "Merge Sort     ", "Quick (First)  ", "Quick (Random) ", "Quick (Median3)" };
    SortFn fns[NALGO] = { heap_sort, merge_sort, quick_first, quick_random, quick_median3 };
    static Result res[6][NALGO];
    int allOk = 1;

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        int reps = WORK_BUDGET / n;
        if (reps < 1) reps = 1;

        /* 원본 데이터 생성 (측정 제외) */
        for (int i = 0; i < n; i++)
            orig[i] = (int)(rng_next() % (MAXKEY + 1));

        for (int k = 0; k < NALGO; k++) {
            double total = 0.0;
            Result* r = &res[s][k];
            memset(r, 0, sizeof(*r));

            for (int t = 0; t < reps; t++) {
                memcpy(work, orig, sizeof(int) * n);       /* 복사 (측정 제외) */

                double st = now_ms();
                fns[k](work, n);
                total += now_ms() - st;

                if (t == 0) {
                    r->cmp = g_cmp;
                    r->heap = g_heap;
                    if (!(is_sorted(work, n) && same_multiset(orig, work, n))) allOk = 0;
                }
                if ((long long)g_stack_max > r->stack) r->stack = (long long)g_stack_max;
                if (g_depth_max > r->depth) r->depth = g_depth_max;
            }
            r->time_ms = total / reps;
        }

        printf("Data Size : %d\n\n", n);
        printf("Algorithm       Comparisons    Time(ms)    Memory(Bytes)\n");
        printf("--------------------------------------------------------\n");
        for (int k = 0; k < NALGO; k++) {
            Result* r = &res[s][k];
            printf("%s %-14lld %-11.4f %lld\n", names[k], r->cmp, r->time_ms, r->heap + r->stack);
        }
        printf("\n");
    }

    printf("Memory = additional heap + measured max stack usage (original array excluded)\n\n");
    printf("[Memory breakdown]\n");
    printf("%-7s %-16s %-12s %-12s %s\n", "n", "Algorithm", "Heap(B)", "Stack(B)", "MaxDepth");
    for (int s = 0; s < nsizes; s++)
        for (int k = 0; k < NALGO; k++)
            printf("%-7d %-16s %-12lld %-12lld %d\n", sizes[s], names[k],
                res[s][k].heap, res[s][k].stack, res[s][k].depth);

    printf("\nAll results sorted correctly : %s\n", allOk ? "YES" : "NO");
    return 0;
}
