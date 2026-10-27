#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 10000
#define MAXKEY 1000000
#define TRIALS 100
#define STAB_N 1000      /* 안정성 검증용 데이터 수 */
#define STAB_RANGE 50    /* 안정성 검증용 Key1 범위 (0~49, 중복 다수 발생) */
#define PARTIAL_SWAPS 500 /* 부분 정렬: 정렬 데이터에서 무작위 교환 횟수 (전체의 약 10% 원소 교란) */

typedef struct {
    int key1;
    int key2;
    int order; /* 원래 입력 순서 (정렬 기준 아님, 안정성 검증용) */
} Data;

static long long g_cmp, g_mov;

/* ---------- 난수 (xorshift64) : 플랫폼 RAND_MAX 차이 방지 ---------- */
static uint64_t rng_state = 88172645463325252ULL;
static uint64_t rng_next(void) {
    uint64_t x = rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    return rng_state = x;
}
static int rand_range(int lo, int hi) { /* lo~hi 포함 */
    return lo + (int)(rng_next() % (uint64_t)(hi - lo + 1));
}

/* ---------- 정렬 알고리즘 ---------- */
static void selection_sort(Data* a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min = i;
        for (int j = i + 1; j < n; j++) {
            g_cmp++;
            if (a[j].key1 < a[min].key1) min = j;
        }
        if (min != i) {
            Data t = a[i]; g_mov++;
            a[i] = a[min]; g_mov++;
            a[min] = t;    g_mov++;
        }
    }
}

/* 일반 버블 정렬: 모든 순회 수행 */
static void bubble_sort(Data* a, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            g_cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                Data t = a[j]; g_mov++;
                a[j] = a[j + 1]; g_mov++;
                a[j + 1] = t;    g_mov++;
            }
        }
    }
}

/* 조기 종료 버블 정렬: 한 순회에서 교환이 없으면 종료 */
static void bubble_sort_early(Data* a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            g_cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                Data t = a[j]; g_mov++;
                a[j] = a[j + 1]; g_mov++;
                a[j + 1] = t;    g_mov++;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

/* 삽입 정렬 Case A: data[j].key1 > current.key1  (안정) */
static void insertion_sort_gt(Data* a, int n) {
    for (int i = 1; i < n; i++) {
        Data cur = a[i]; g_mov++;           /* 임시 변수에 저장 */
        int j = i - 1;
        while (j >= 0) {
            g_cmp++;
            if (a[j].key1 > cur.key1) {
                a[j + 1] = a[j]; g_mov++;   /* 한 칸 이동 */
                j--;
            }
            else break;
        }
        a[j + 1] = cur; g_mov++;            /* 최종 위치에 대입 */
    }
}

/* 삽입 정렬 Case B: data[j].key1 >= current.key1  (불안정) */
static void insertion_sort_ge(Data* a, int n) {
    for (int i = 1; i < n; i++) {
        Data cur = a[i]; g_mov++;
        int j = i - 1;
        while (j >= 0) {
            g_cmp++;
            if (a[j].key1 >= cur.key1) {
                a[j + 1] = a[j]; g_mov++;
                j--;
            }
            else break;
        }
        a[j + 1] = cur; g_mov++;
    }
}

typedef void (*SortFn)(Data*, int);

enum { SEL, BUB, BUBE, INS, NALGO };
static const char* algo_name[NALGO] = {
    "Selection Sort", "Bubble Sort", "Bubble Sort (Early Termination)", "Insertion Sort"
};
static SortFn algo_fn[NALGO] = { selection_sort, bubble_sort, bubble_sort_early, insertion_sort_gt };

/* ---------- 유틸리티 ---------- */
static void gen_random(Data* a, int n, int maxkey) {
    for (int i = 0; i < n; i++) {
        a[i].key1 = rand_range(0, maxkey);
        a[i].key2 = rand_range(0, maxkey);
        a[i].order = i;
    }
}

static int is_sorted(const Data* a, int n) {
    for (int i = 0; i < n - 1; i++)
        if (a[i].key1 > a[i + 1].key1) return 0;
    return 1;
}

/* 정렬 결과 검증: 오름차순 + 데이터 손실/변조 없음(입력 순서 번호 합, key 합 비교) */
static int verify(const Data* orig, const Data* res, int n) {
    if (!is_sorted(res, n)) return 0;
    long long s1 = 0, s2 = 0, o1 = 0, o2 = 0, q1 = 0, q2 = 0;
    for (int i = 0; i < n; i++) {
        s1 += orig[i].key1; s2 += res[i].key1;
        o1 += orig[i].key2; o2 += res[i].key2;
        q1 += orig[i].order; q2 += res[i].order;
        /* 두 키가 하나의 데이터로 함께 이동했는지 확인 */
        if (orig[res[i].order].key1 != res[i].key1 ||
            orig[res[i].order].key2 != res[i].key2) return 0;
    }
    return s1 == s2 && o1 == o2 && q1 == q2;
}

/* 안정성 검사: Key1이 같은 인접 데이터의 order가 증가하는지 */
static int check_stable(const Data* a, int n, int* violations, int* first_idx) {
    int v = 0; *first_idx = -1;
    for (int i = 0; i < n - 1; i++) {
        if (a[i].key1 == a[i + 1].key1 && a[i].order > a[i + 1].order) {
            if (*first_idx < 0) *first_idx = i;
            v++;
        }
    }
    *violations = v;
    return v == 0;
}

/* 사전 정렬용 (측정 대상 아님): key1 오름차순 */
static int cmp_key1(const void* x, const void* y) {
    const Data* p = x, * q = y;
    if (p->key1 != q->key1) return (p->key1 > q->key1) - (p->key1 < q->key1);
    return (p->order > q->order) - (p->order < q->order);
}

/* ---------- 입력 상태별 실험 ---------- */
static void run_once(const char* title, const Data* input) {
    static Data work[N];
    printf("%s\n\n", title);
    for (int k = 0; k < NALGO; k++) {
        memcpy(work, input, sizeof(Data) * N);
        g_cmp = g_mov = 0;
        algo_fn[k](work, N);
        printf("%s\n", algo_name[k]);
        printf("Comparisons : %lld\n", g_cmp);
        printf("Movements   : %lld\n", g_mov);
        printf("Verified    : %s\n\n", verify(input, work, N) ? "OK" : "FAIL");
    }
    printf("------------------------------------\n\n");
}

/* ---------- 안정성 검증 ---------- */
static void stability_case(const char* label, SortFn fn, const Data* input, int n) {
    static Data work[STAB_N];
    int viol, first;
    memcpy(work, input, sizeof(Data) * n);
    fn(work, n);
    int sorted_ok = is_sorted(work, n);
    int stable = check_stable(work, n, &viol, &first);
    printf("%-20s : %s (sorted=%s, 순서 역전 %d건", label,
        stable ? "Stable" : "Not Stable", sorted_ok ? "OK" : "FAIL", viol);
    if (!stable)
        printf(", 예: (%d,%d)#%d 가 (%d,%d)#%d 보다 앞에 위치",
            work[first].key1, work[first].key2, work[first].order,
            work[first + 1].key1, work[first + 1].key2, work[first + 1].order);
    printf(")\n");
}

static void stability_test(void) {
    /* (1) 명세 예시 데이터로 소규모 확인 */
    Data ex[4] = { {527, 100, 0}, {527, 200, 1}, {125, 300, 2}, {892, 400, 3} };
    struct { const char* name; SortFn fn; } t[4] = {
        { "Selection Sort", selection_sort }, { "Bubble Sort", bubble_sort },
        { "Insertion Sort (>)", insertion_sort_gt }, { "Insertion Sort (>=)", insertion_sort_ge }
    };
    printf("[Small example: (527,100) (527,200) (125,300) (892,400)]\n");
    for (int k = 0; k < 4; k++) {
        Data w[4]; memcpy(w, ex, sizeof(ex));
        t[k].fn(w, 4);
        printf("  %-20s :", t[k].name);
        for (int i = 0; i < 4; i++) printf(" (%d,%d)", w[i].key1, w[i].key2);
        printf("\n");
    }

    /* (2) 중복 키가 많은 시험 데이터 (Key1: 0~49, 1000개) */
    static Data dup[STAB_N];
    gen_random(dup, STAB_N, STAB_RANGE - 1);
    printf("\n[Duplicate-key test data: %d items, Key1 in 0..%d]\n", STAB_N, STAB_RANGE - 1);
    printf("Stability Verification\n\n");
    stability_case("Selection Sort", selection_sort, dup, STAB_N);
    stability_case("Bubble Sort", bubble_sort, dup, STAB_N);
    stability_case("Bubble Sort (Early)", bubble_sort_early, dup, STAB_N);
    stability_case("Insertion Sort (>)", insertion_sort_gt, dup, STAB_N);
    stability_case("Insertion Sort (>=)", insertion_sort_ge, dup, STAB_N);

    /* (3) 삽입 정렬 > 와 >= 의 이동 횟수 차이 (동일 입력) */
    static Data w[STAB_N];
    memcpy(w, dup, sizeof(dup)); g_cmp = g_mov = 0; insertion_sort_gt(w, STAB_N);
    long long c1 = g_cmp, m1 = g_mov;
    memcpy(w, dup, sizeof(dup)); g_cmp = g_mov = 0; insertion_sort_ge(w, STAB_N);
    printf("\nInsertion (>)  : comparisons=%lld, movements=%lld\n", c1, m1);
    printf("Insertion (>=) : comparisons=%lld, movements=%lld\n", g_cmp, g_mov);
}

int main(void) {
    static Data base[N], input[N], work[N];

    printf("Data Count : %d\n\n", N);

    /* ===== 무작위 데이터 100회 반복 ===== */
    long long totCmp[NALGO] = { 0 }, totMov[NALGO] = { 0 };
    int allOk = 1;
    for (int t = 0; t < TRIALS; t++) {
        gen_random(input, N, MAXKEY);          /* 모든 알고리즘이 같은 입력 사용 */
        for (int k = 0; k < NALGO; k++) {
            memcpy(work, input, sizeof(Data) * N);
            g_cmp = g_mov = 0;
            algo_fn[k](work, N);
            totCmp[k] += g_cmp;
            totMov[k] += g_mov;
            if (!verify(input, work, N)) allOk = 0;
        }
    }
    printf("Random Data\nNumber of Trials : %d\n\n", TRIALS);
    for (int k = 0; k < NALGO; k++) {
        printf("%s\n", algo_name[k]);
        printf("Total Comparisons   : %lld\n", totCmp[k]);
        printf("Average Comparisons : %.2f\n", (double)totCmp[k] / TRIALS);
        printf("Total Movements     : %lld\n", totMov[k]);
        printf("Average Movements   : %.2f\n\n", (double)totMov[k] / TRIALS);
    }
    printf("All trials sorted correctly : %s\n", allOk ? "YES" : "NO");
    printf("\n------------------------------------\n\n");

    /* ===== 동일한 원본 데이터로부터 세 가지 입력 상태 생성 ===== */
    gen_random(base, N, MAXKEY);

    /* 정렬된 데이터 (사전 정렬: 측정 제외) */
    static Data sorted[N], reversed[N], partial[N];
    memcpy(sorted, base, sizeof(base));
    qsort(sorted, N, sizeof(Data), cmp_key1);
    for (int i = 0; i < N; i++) sorted[i].order = i; /* 입력 순서 번호 재부여 */

    /* 역순 데이터 */
    for (int i = 0; i < N; i++) reversed[i] = sorted[N - 1 - i];
    for (int i = 0; i < N; i++) reversed[i].order = i;

    /* 일부 정렬 데이터: 정렬된 데이터에서 무작위 두 위치를 PARTIAL_SWAPS번 교환 */
    memcpy(partial, sorted, sizeof(sorted));
    for (int s = 0; s < PARTIAL_SWAPS; s++) {
        int i = rand_range(0, N - 1), j = rand_range(0, N - 1);
        Data t = partial[i]; partial[i] = partial[j]; partial[j] = t;
    }
    for (int i = 0; i < N; i++) partial[i].order = i;

    run_once("Sorted Data", sorted);
    run_once("Reverse-Sorted Data", reversed);
    printf("(Partially Sorted: sorted data with %d random pair swaps)\n", PARTIAL_SWAPS);
    run_once("Partially Sorted Data", partial);

    /* ===== 안정성 검증 ===== */
    stability_test();

    return 0;
}