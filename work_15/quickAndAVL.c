#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#define MAXKEY 1000000
#define MAXN 20000
#define MAXOPS 4000
#define REPS 20                 /* 시간은 같은 조건에서 반복 측정한 평균 */

/* ---------- 측정용 전역 변수 ---------- */
static long long g_cmp;         /* 데이터 값 비교 횟수 */
static long long g_mov;         /* 데이터 값 복사/대입 횟수 */
static uintptr_t g_stack_base, g_stack_max;
static long long g_live_nodes, g_peak_nodes;   /* AVL 노드 수 */

static void track_stack(const char* probe) {
    uintptr_t addr = (uintptr_t)probe;
    uintptr_t used = (g_stack_base > addr) ? g_stack_base - addr : addr - g_stack_base;
    if (used > g_stack_max) g_stack_max = used;
}

/* ---------- 난수 (xorshift64) ---------- */
static uint64_t rng_state = 88172645463325252ULL;
static uint64_t rng_next(void) {
    uint64_t x = rng_state;
    x ^= x << 13; x ^= x >> 7; x ^= x << 17;
    return rng_state = x;
}
static int rand_key(void) { return (int)(rng_next() % (MAXKEY + 1)); }

/* ---------- 시간 (ms) ---------- */
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

/* 두 값의 비교 1회 = 1 (3분기 결과 <, ==, > 를 한 번의 비교로 계산) */
static int cmp3(int a, int b) {
    g_cmp++;
    return (a > b) - (a < b);
}

/* =====================================================
   [1] 퀵 정렬 (재귀, 피벗: Median-of-Three)
   ===================================================== */
static void swap_int(int* x, int* y) {
    if (x == y) return;
    int t = *x; *x = *y; *y = t;
    g_mov += 3;                          /* temp = x; x = y; y = temp; */
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

static int partition(int* a, int lo, int hi) {
    int p = pivot_median3(a, lo, hi);
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

static void quick_rec(int* a, int lo, int hi) {
    char probe;
    track_stack(&probe);
    if (lo >= hi) return;
    int p = partition(a, lo, hi);
    quick_rec(a, lo, p - 1);
    quick_rec(a, p + 1, hi);
}

/* ---------- 정렬된 배열의 삽입/삭제 (선형 탐색) ---------- */
static void arr_insert(int* a, int* cnt, int v) {
    int pos = 0;
    while (pos < *cnt) {                       /* v 보다 큰 첫 원소의 위치를 선형 탐색 */
        if (cmp3(a[pos], v) > 0) break;
        pos++;
    }
    for (int i = *cnt; i > pos; i--) {         /* 뒤로 한 칸씩 이동 */
        a[i] = a[i - 1];
        g_mov++;
    }
    a[pos] = v;
    g_mov++;
    (*cnt)++;
}

static int arr_delete(int* a, int* cnt, int v) {
    int pos = 0, found = 0;
    while (pos < *cnt) {                       /* v 이상인 첫 원소를 선형 탐색 */
        int c = cmp3(a[pos], v);
        if (c >= 0) { found = (c == 0); break; }
        pos++;
    }
    if (!found) return 0;
    for (int i = pos; i < *cnt - 1; i++) {     /* 앞으로 한 칸씩 이동 */
        a[i] = a[i + 1];
        g_mov++;
    }
    (*cnt)--;
    return 1;
}

/* =====================================================
   [2] AVL 트리 (중복은 노드의 count 에 저장)
   ===================================================== */
typedef struct Node {
    int key;
    int count;
    int height;
    struct Node* left, * right;
} Node;

static int height(const Node* n) { return n ? n->height : 0; }
static void update(Node* n) {
    int hl = height(n->left), hr = height(n->right);
    n->height = 1 + (hl > hr ? hl : hr);
}
static int balance_factor(const Node* n) { return height(n->left) - height(n->right); }

static Node* rotate_right(Node* y) {           /* 포인터 연결만 변경: 데이터 이동 아님 */
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    update(y); update(x);
    return x;
}
static Node* rotate_left(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    update(x); update(y);
    return y;
}
static Node* rebalance(Node* t) {
    update(t);
    int bf = balance_factor(t);
    if (bf > 1) {
        if (balance_factor(t->left) < 0) t->left = rotate_left(t->left);
        return rotate_right(t);
    }
    if (bf < -1) {
        if (balance_factor(t->right) > 0) t->right = rotate_right(t->right);
        return rotate_left(t);
    }
    return t;
}

static Node* new_node(int key) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->key = key; g_mov++;                     /* 키 값 저장 = 데이터 이동 1회 */
    n->count = 1;
    n->height = 1;
    n->left = n->right = NULL;
    if (++g_live_nodes > g_peak_nodes) g_peak_nodes = g_live_nodes;
    return n;
}

static Node* avl_insert(Node* t, int key) {
    char probe;
    track_stack(&probe);
    if (!t) return new_node(key);
    int c = cmp3(key, t->key);
    if (c < 0)      t->left = avl_insert(t->left, key);
    else if (c > 0) t->right = avl_insert(t->right, key);
    else { t->count++; return t; }             /* 중복: 횟수만 증가 */
    return rebalance(t);
}

/* 서브트리에서 가장 작은 노드를 떼어 내고(해제하지 않음) 새 서브트리 루트를 반환 */
static Node* remove_min(Node* t, Node** min_out) {
    char probe;
    track_stack(&probe);
    if (!t->left) {
        *min_out = t;
        return t->right;
    }
    t->left = remove_min(t->left, min_out);
    return rebalance(t);
}

static Node* avl_delete(Node* t, int key, int* removed) {
    char probe;
    track_stack(&probe);
    if (!t) return NULL;
    int c = cmp3(key, t->key);
    if (c < 0)      t->left = avl_delete(t->left, key, removed);
    else if (c > 0) t->right = avl_delete(t->right, key, removed);
    else {
        *removed = 1;
        if (t->count > 1) { t->count--; return t; }   /* 중복 하나만 제거 */
        if (!t->left || !t->right) {
            Node* child = t->left ? t->left : t->right;
            free(t); g_live_nodes--;
            return child;
        }
        Node* s;                                       /* 후속자(오른쪽 서브트리의 최솟값) */
        t->right = remove_min(t->right, &s);
        t->key = s->key;                               /* 키 값 복사 = 데이터 이동 1회 */
        t->count = s->count;
        g_mov++;
        free(s); g_live_nodes--;
    }
    return rebalance(t);
}

/* ---------- 검증용 (측정 제외) ---------- */
static int check_avl(const Node* t, int* ok) {
    if (!t) return 0;
    int hl = check_avl(t->left, ok), hr = check_avl(t->right, ok);
    int h = 1 + (hl > hr ? hl : hr);
    if (hl - hr > 1 || hr - hl > 1 || t->height != h || t->count < 1) *ok = 0;
    return h;
}
static void inorder_fill(const Node* t, int* out, int* pos) {
    if (!t) return;
    inorder_fill(t->left, out, pos);
    for (int i = 0; i < t->count; i++) out[(*pos)++] = t->key;
    inorder_fill(t->right, out, pos);
}
static void free_tree(Node* t) {
    if (!t) return;
    free_tree(t->left); free_tree(t->right);
    free(t);
}
static int is_sorted(const int* a, int n) {
    for (int i = 0; i < n - 1; i++)
        if (a[i] > a[i + 1]) return 0;
    return 1;
}

/* ---------- 결과 구조체 ---------- */
typedef struct {
    double t_sort, t_ops;                 /* 반복 평균 시간 (ms) */
    long long cmp_sort, cmp_ops;
    long long mov_sort, mov_ops;
    long long mem_ds, mem_stack;          /* 자료구조 메모리, 최대 스택 사용량 */
    int final_count;
    int inserts, deletes, misses;
    int distinct;                         /* AVL 노드 수 (배열은 final_count) */
} Result;

typedef struct { int is_insert; int value; } Op;

static int g_ok = 1;                      /* 전체 검증 결과 */
static int g_snapshot[MAXN];              /* 퀵 정렬 직후의 정렬 결과 (AVL 결과와 대조) */

/* ---------- 퀵 정렬 + 배열 삽입/삭제 ---------- */
static void run_quick(const int* orig, int n, const Op* ops, int nops, int* arr, Result* r) {
    char base;
    double t_sort = 0, t_ops = 0;
    for (int rep = 0; rep < REPS; rep++) {
        memcpy(arr, orig, sizeof(int) * n);        /* 복사 (측정 제외) */
        g_cmp = g_mov = 0;
        g_stack_base = (uintptr_t)&base;
        if (rep == 0) g_stack_max = 0;

        int cnt = n, ins = 0, del = 0, miss = 0;
        double s0 = now_ms();
        quick_rec(arr, 0, n - 1);
        double s1 = now_ms();
        long long cs = g_cmp, ms = g_mov;

        if (rep == 0) {                            /* 검증 (시간 측정 구간 밖) */
            if (!is_sorted(arr, n)) g_ok = 0;
            memcpy(g_snapshot, arr, sizeof(int) * n);
        }

        double o0 = now_ms();
        for (int i = 0; i < nops; i++) {
            if (ops[i].is_insert) { arr_insert(arr, &cnt, ops[i].value); ins++; }
            else if (arr_delete(arr, &cnt, ops[i].value)) del++;
            else miss++;
        }
        double o1 = now_ms();

        t_sort += s1 - s0;
        t_ops += o1 - o0;
        if (rep == 0) {
            r->cmp_sort = cs;           r->mov_sort = ms;
            r->cmp_ops = g_cmp - cs;    r->mov_ops = g_mov - ms;
            r->final_count = cnt;
            r->inserts = ins; r->deletes = del; r->misses = miss;
            if (!is_sorted(arr, cnt)) g_ok = 0;
        }
    }
    r->t_sort = t_sort / REPS;
    r->t_ops = t_ops / REPS;
    r->mem_ds = (long long)sizeof(int) * (n + nops);   /* 삽입 최대치까지 확보한 배열 */
    r->mem_stack = (long long)g_stack_max;
}

/* ---------- AVL 트리 정렬 + 삽입/삭제 ---------- */
static void run_avl(const int* orig, int n, const Op* ops, int nops, Result* r, const int* quick_final, int quick_cnt) {
    char base;
    static int expand[MAXN + MAXOPS];
    double t_sort = 0, t_ops = 0;
    for (int rep = 0; rep < REPS; rep++) {
        g_cmp = g_mov = 0;
        g_live_nodes = g_peak_nodes = 0;
        g_stack_base = (uintptr_t)&base;
        if (rep == 0) g_stack_max = 0;

        Node* root = NULL;
        int ins = 0, del = 0, miss = 0;
        double s0 = now_ms();
        for (int i = 0; i < n; i++)
            root = avl_insert(root, orig[i]);
        double s1 = now_ms();
        long long cs = g_cmp, ms = g_mov;

        if (rep == 0) {                            /* 검증: 중위 순회 결과 = 정렬된 배열 */
            int ok = 1, pos = 0;
            check_avl(root, &ok);
            inorder_fill(root, expand, &pos);
            if (!ok || pos != n || memcmp(expand, g_snapshot, sizeof(int) * n) != 0) g_ok = 0;
        }

        double o0 = now_ms();
        for (int i = 0; i < nops; i++) {
            if (ops[i].is_insert) { root = avl_insert(root, ops[i].value); ins++; }
            else {
                int removed = 0;
                root = avl_delete(root, ops[i].value, &removed);
                if (removed) del++; else miss++;
            }
        }
        double o1 = now_ms();

        t_sort += s1 - s0;
        t_ops += o1 - o0;
        if (rep == 0) {
            r->cmp_sort = cs;           r->mov_sort = ms;
            r->cmp_ops = g_cmp - cs;    r->mov_ops = g_mov - ms;
            r->inserts = ins; r->deletes = del; r->misses = miss;
            r->distinct = (int)g_live_nodes;
            int ok = 1, pos = 0;
            check_avl(root, &ok);
            inorder_fill(root, expand, &pos);
            r->final_count = pos;
            if (!ok || pos != quick_cnt || memcmp(expand, quick_final, sizeof(int) * pos) != 0) g_ok = 0;
            r->mem_ds = (long long)g_peak_nodes * (long long)sizeof(Node);
        }
        free_tree(root);                            /* 정리 (측정 제외) */
    }
    r->t_sort = t_sort / REPS;
    r->t_ops = t_ops / REPS;
    r->mem_stack = (long long)g_stack_max;
}

int main(void) {
    static int orig[MAXN], arr[MAXN + MAXOPS];
    static Op ops[MAXOPS];
    const int sizes[] = { 1000, 2000, 4000, 8000, 16000, 20000 };
    const int nsizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        int nops = n / 5;
        Result q, a;
        memset(&q, 0, sizeof(q)); memset(&a, 0, sizeof(a));

        /* 원본 데이터와 연산 목록 생성 (측정 제외) */
        for (int i = 0; i < n; i++) orig[i] = rand_key();
        for (int i = 0; i < nops; i++) {
            ops[i].is_insert = (rand_key() % 2 == 0);   /* 짝수: 삽입, 홀수: 삭제 */
            ops[i].value = rand_key();                  /* 실제 값은 별도의 난수 */
        }

        run_quick(orig, n, ops, nops, arr, &q);
        /* AVL 결과와 대조할 최종 배열은 run_quick 의 마지막 반복 결과(arr) */
        run_avl(orig, n, ops, nops, &a, arr, q.final_count);

        long long q_cmp = q.cmp_sort + q.cmp_ops, a_cmp = a.cmp_sort + a.cmp_ops;
        long long q_mov = q.mov_sort + q.mov_ops, a_mov = a.mov_sort + a.mov_ops;
        double q_t = q.t_sort + q.t_ops, a_t = a.t_sort + a.t_ops;
        long long q_mem = q.mem_ds + q.mem_stack, a_mem = a.mem_ds + a.mem_stack;

        printf("Data Size : %d\n", n);
        printf("Operations: %d\n\n", nops);
        printf("                  Quick Sort      AVL Tree\n");
        printf("------------------------------------------------\n");
        printf("Total Time        %-8.4f ms      %-8.4f ms\n", q_t, a_t);
        printf("Comparisons       %-15lld %lld\n", q_cmp, a_cmp);
        printf("Data Movements    %-15lld %lld\n", q_mov, a_mov);
        printf("Memory Usage      %-6lld Bytes     %lld Bytes\n\n", q_mem, a_mem);
        printf("Final Data Count  %-15d %d\n\n", q.final_count, a.final_count);

        printf("[Breakdown]                Quick Sort      AVL Tree\n");
        printf("  Sort time (ms)           %-15.4f %.4f\n", q.t_sort, a.t_sort);
        printf("  Ops time (ms)            %-15.4f %.4f\n", q.t_ops, a.t_ops);
        printf("  Sort comparisons         %-15lld %lld\n", q.cmp_sort, a.cmp_sort);
        printf("  Ops comparisons          %-15lld %lld\n", q.cmp_ops, a.cmp_ops);
        printf("  Sort movements           %-15lld %lld\n", q.mov_sort, a.mov_sort);
        printf("  Ops movements            %-15lld %lld\n", q.mov_ops, a.mov_ops);
        printf("  Data structure (Bytes)   %-15lld %lld\n", q.mem_ds, a.mem_ds);
        printf("  Max stack (Bytes)        %-15lld %lld\n", q.mem_stack, a.mem_stack);
        printf("  Inserts / Deletes / Delete misses : %d / %d / %d (AVL: %d / %d / %d)\n",
            q.inserts, q.deletes, q.misses, a.inserts, a.deletes, a.misses);
        printf("  AVL distinct nodes (final) : %d\n\n", a.distinct);
        printf("------------------------------------------------\n\n");
    }

    printf("AVL node size : %zu Bytes\n", sizeof(Node));
    printf("All results verified (sorted, AVL balance, array == inorder) : %s\n", g_ok ? "YES" : "NO");
    return 0;
}
