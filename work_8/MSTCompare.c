#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define V          50       /* 정점 수 */
#define SPARSE_E   100      /* 희소 그래프 간선 수 */
#define DENSE_E    600      /* 밀집 그래프 간선 수 */
#define MAX_W      1000     /* 가중치 범위: 1 ~ MAX_W */
#define REPS       2000     /* 실행시간 측정 시 반복 횟수 (평균을 사용) */

typedef struct { int u, v, w; } Edge;

/* ================= 시간 측정 ================= */

static double now_us(void) {
#ifdef _WIN32
    LARGE_INTEGER f, c;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1e6 / (double)f.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e6 + (double)ts.tv_nsec / 1e3;
#endif
}

/* ================= Union-Find ================= */

static int uf_find(int* parent, int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];        /* path halving */
        x = parent[x];
    }
    return x;
}

static int uf_union(int* parent, int* rnk, int a, int b) {
    int ra = uf_find(parent, a), rb = uf_find(parent, b);
    if (ra == rb) return 0;                    /* 같은 집합 -> 사이클 */
    if (rnk[ra] < rnk[rb]) { int t = ra; ra = rb; rb = t; }
    parent[rb] = ra;
    if (rnk[ra] == rnk[rb]) rnk[ra]++;
    return 1;
}

static void uf_init(int* parent, int* rnk) {
    for (int i = 0; i < V; i++) { parent[i] = i; rnk[i] = 0; }
}

/* ================= 인접 리스트 (가중치 포함) ================= */

typedef struct ANode {
    int v;
    int w;
    struct ANode* next;
} ANode;

typedef struct {
    ANode* head[V];
    int node_cnt;
} AdjList;

static void list_push_front(AdjList* g, int u, int v, int w) {
    ANode* n = (ANode*)malloc(sizeof(ANode));
    if (!n) { perror("malloc"); exit(1); }
    n->v = v; n->w = w;
    n->next = g->head[u];
    g->head[u] = n;
    g->node_cnt++;
}

static void list_free(AdjList* g) {
    for (int i = 0; i < V; i++) {
        ANode* p = g->head[i];
        while (p) { ANode* t = p->next; free(p); p = t; }
        g->head[i] = NULL;
    }
    g->node_cnt = 0;
}

/* ================= 인접 행렬 (0 = 간선 없음, 가중치는 양수) ================= */

typedef struct { int w[V][V]; } AdjMatrix;

/* ================= Kruskal (인접 리스트) ================= */

static int cmp_edge(const void* a, const void* b) {
    const Edge* x = (const Edge*)a, * y = (const Edge*)b;
    if (x->w != y->w) return x->w - y->w;
    if (x->u != y->u) return x->u - y->u;
    return x->v - y->v;
}

/* mst에 선택된 간선을 저장, 선택된 간선 수 반환 */
static int kruskal(const AdjList* g, Edge* mst, long* total) {
    int m = g->node_cnt / 2;
    Edge* edges = (Edge*)malloc((size_t)m * sizeof(Edge));
    if (!edges) { perror("malloc"); exit(1); }

    /* 1. 인접 리스트에서 간선 추출 (무방향이므로 u < v 인 쪽만) */
    int k = 0;
    for (int u = 0; u < V; u++)
        for (ANode* p = g->head[u]; p != NULL; p = p->next)
            if (u < p->v) { edges[k].u = u; edges[k].v = p->v; edges[k].w = p->w; k++; }

    /* 2. 가중치 오름차순 정렬 */
    qsort(edges, (size_t)k, sizeof(Edge), cmp_edge);

    /* 3. Union-Find로 사이클 검사하며 선택 */
    int parent[V], rnk[V];
    uf_init(parent, rnk);

    int cnt = 0;
    *total = 0;
    for (int i = 0; i < k && cnt < V - 1; i++) {
        if (uf_union(parent, rnk, edges[i].u, edges[i].v)) {
            mst[cnt++] = edges[i];
            *total += edges[i].w;
        }
    }
    free(edges);
    return cnt;
}

/* ================= Prim (인접 행렬, O(V^2)) ================= */

static int prim(const AdjMatrix* g, Edge* mst, long* total) {
    int key[V], parent[V];
    char in_mst[V];

    for (int i = 0; i < V; i++) { key[i] = INT_MAX; parent[i] = -1; in_mst[i] = 0; }
    key[0] = 0;                                   /* 시작 정점 0 */

    int cnt = 0;
    *total = 0;
    for (int iter = 0; iter < V; iter++) {
        /* MST 밖 정점 중 key가 최소인 정점 선택 */
        int u = -1;
        for (int i = 0; i < V; i++)
            if (!in_mst[i] && (u == -1 || key[i] < key[u])) u = i;
        if (u == -1 || key[u] == INT_MAX) break;  /* 연결 그래프가 아님 */

        in_mst[u] = 1;
        if (parent[u] != -1) {
            mst[cnt].u = parent[u]; mst[cnt].v = u; mst[cnt].w = key[u];
            cnt++;
            *total += key[u];
        }
        /* u와 인접한 정점들의 최소 연결 비용 갱신 */
        for (int v = 0; v < V; v++) {
            int w = g->w[u][v];
            if (w != 0 && !in_mst[v] && w < key[v]) { key[v] = w; parent[v] = u; }
        }
    }
    return cnt;
}

/* ================= 그래프 생성 ================= */

static int is_connected(const Edge* edges, int m) {
    int parent[V], rnk[V], comps = V;
    uf_init(parent, rnk);
    for (int i = 0; i < m; i++)
        if (uf_union(parent, rnk, edges[i].u, edges[i].v)) comps--;
    return comps == 1;
}

/* 연결 보장: 랜덤 신장 트리를 먼저 만든 뒤 나머지 간선을 랜덤하게 추가 */
static void generate_graph(Edge* edges, int m) {
    static char used[V][V];
    int perm[V];

    do {
        memset(used, 0, sizeof(used));
        for (int i = 0; i < V; i++) perm[i] = i;
        for (int i = V - 1; i > 0; i--) {         /* Fisher-Yates 셔플 */
            int j = rand() % (i + 1);
            int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
        }

        int k = 0;
        for (int i = 1; i < V; i++) {             /* 신장 트리 간선 V-1개 */
            int a = perm[i], b = perm[rand() % i];
            used[a][b] = used[b][a] = 1;
            edges[k].u = a; edges[k].v = b; edges[k].w = rand() % MAX_W + 1;
            k++;
        }
        while (k < m) {                           /* 남은 간선 (자기 루프/중복 불가) */
            int a = rand() % V, b = rand() % V;
            if (a == b || used[a][b]) continue;
            used[a][b] = used[b][a] = 1;
            edges[k].u = a; edges[k].v = b; edges[k].w = rand() % MAX_W + 1;
            k++;
        }
    } while (!is_connected(edges, m));            /* 연결 그래프인지 확인 */
}

/* ================= 검증 ================= */

/* 선택된 간선이 V-1개이고 사이클 없이 모든 정점을 연결하는지 확인 */
static int verify_tree(const Edge* mst, int cnt) {
    if (cnt != V - 1) return 0;
    int parent[V], rnk[V];
    uf_init(parent, rnk);
    for (int i = 0; i < cnt; i++)
        if (!uf_union(parent, rnk, mst[i].u, mst[i].v)) return 0;
    return 1;
}

/* 리스트와 행렬이 edges와 같은 그래프인지 확인 */
static int verify_graph(const AdjList* L, const AdjMatrix* M, const Edge* edges, int m) {
    if (L->node_cnt != 2 * m) return 0;
    for (int i = 0; i < m; i++) {
        int u = edges[i].u, v = edges[i].v, w = edges[i].w;
        if (M->w[u][v] != w || M->w[v][u] != w) return 0;
        int found = 0;
        for (ANode* p = L->head[u]; p; p = p->next)
            if (p->v == v && p->w == w) { found = 1; break; }
        if (!found) return 0;
    }
    return 1;
}

/* ================= 출력 ================= */

typedef struct {
    int no, m, cnt;
    char title[96];
    long cost;
    size_t mem;
    double avg_us;
    Edge mst[V - 1];
} CaseResult;

static CaseResult results[4];      /* Case 1~4를 순서대로 출력하기 위해 저장 */

static void print_case(const CaseResult* r) {
    int no = r->no, m = r->m, cnt = r->cnt;
    const char* title = r->title;
    long cost = r->cost;
    size_t mem = r->mem;
    double avg_us = r->avg_us;
    const Edge* mst = r->mst;
    printf("Case %d : %s\n", no, title);
    printf("Vertices      : %d\n", V);
    printf("Edges         : %d\n", m);
    printf("MST Edges     : %d\n", cnt);
    printf("MST Cost      : %ld\n", cost);
    printf("Memory        : %zu Bytes\n", mem);
    printf("Execution Time: %.4f ms (avg of %d runs, %.2f us)\n",
        avg_us / 1000.0, REPS, avg_us);
    printf("MST edge list (u - v : weight)\n");
    for (int i = 0; i < cnt; i++)
        printf("  %2d - %2d : %4d%s", mst[i].u, mst[i].v, mst[i].w,
            (i % 4 == 3 || i == cnt - 1) ? "\n" : "   ");
    printf("\n");
}

/* ================= 그래프 한 종류에 대한 실험 ================= */

static void run(const char* name, int m, int case_list, int case_mat) {
    Edge* edges = (Edge*)malloc((size_t)m * sizeof(Edge));   /* 생성용 임시 배열 */
    AdjList* L = (AdjList*)calloc(1, sizeof(AdjList));
    AdjMatrix* M = (AdjMatrix*)calloc(1, sizeof(AdjMatrix));
    Edge mst_k[V - 1], mst_p[V - 1];
    long cost_k = 0, cost_p = 0;
    int cnt_k = 0, cnt_p = 0;
    if (!edges || !L || !M) { perror("alloc"); exit(1); }

    /* 같은 그래프로 두 표현을 구성 (구성 시간은 측정에서 제외) */
    generate_graph(edges, m);
    for (int i = 0; i < m; i++) {
        int u = edges[i].u, v = edges[i].v, w = edges[i].w;
        list_push_front(L, u, v, w);
        list_push_front(L, v, u, w);
        M->w[u][v] = M->w[v][u] = w;
    }
    if (!verify_graph(L, M, edges, m)) { fprintf(stderr, "오류: 그래프 구성 불일치\n"); exit(1); }

    /* Kruskal 실행시간: 알고리즘 시작 ~ MST 완성 (REPS회 반복 후 평균) */
    double t0 = now_us();
    for (int r = 0; r < REPS; r++) cnt_k = kruskal(L, mst_k, &cost_k);
    double kr_us = (now_us() - t0) / REPS;

    /* Prim 실행시간 */
    t0 = now_us();
    for (int r = 0; r < REPS; r++) cnt_p = prim(M, mst_p, &cost_p);
    double pr_us = (now_us() - t0) / REPS;

    /* 결과 검증 */
    if (!verify_tree(mst_k, cnt_k) || !verify_tree(mst_p, cnt_p)) {
        fprintf(stderr, "오류: MST가 신장 트리가 아님\n"); exit(1);
    }
    if (cost_k != cost_p) {
        fprintf(stderr, "오류: Kruskal(%ld)과 Prim(%ld)의 MST 비용 불일치\n", cost_k, cost_p);
        exit(1);
    }

    /* 메모리: 그래프 표현 + 알고리즘 보조 자료구조 + MST 결과 배열 */
    size_t mem_k = sizeof(AdjList) + (size_t)L->node_cnt * sizeof(ANode)   /* 인접 리스트 */
        + (size_t)m * sizeof(Edge)                                /* 정렬용 간선 배열 */
        + 2 * V * sizeof(int)                                     /* Union-Find parent, rank */
        + (V - 1) * sizeof(Edge);                                 /* MST 결과 */
    size_t mem_p = sizeof(AdjMatrix)                                       /* 인접 행렬 */
        + 2 * V * sizeof(int) + V * sizeof(char)                  /* key, parent, in_mst */
        + (V - 1) * sizeof(Edge);                                 /* MST 결과 */

    CaseResult* rk = &results[case_list - 1], * rp = &results[case_mat - 1];
    rk->no = case_list; rk->m = m; rk->cnt = cnt_k; rk->cost = cost_k;
    rk->mem = mem_k; rk->avg_us = kr_us;
    memcpy(rk->mst, mst_k, sizeof(mst_k));
    snprintf(rk->title, sizeof(rk->title), "%s Graph - Adjacency List - Kruskal", name);

    rp->no = case_mat; rp->m = m; rp->cnt = cnt_p; rp->cost = cost_p;
    rp->mem = mem_p; rp->avg_us = pr_us;
    memcpy(rp->mst, mst_p, sizeof(mst_p));
    snprintf(rp->title, sizeof(rp->title), "%s Graph - Adjacency Matrix - Prim", name);

    list_free(L);
    free(L); free(M); free(edges);
}

int main(void) {
    srand((unsigned)time(NULL));

    printf("sizeof(Edge)=%zu, sizeof(ANode)=%zu, sizeof(AdjList)=%zu, sizeof(AdjMatrix)=%zu\n\n",
        sizeof(Edge), sizeof(ANode), sizeof(AdjList), sizeof(AdjMatrix));

    run("Sparse", SPARSE_E, 1, 3);
    run("Dense", DENSE_E, 2, 4);

    for (int i = 0; i < 4; i++) print_case(&results[i]);
    return 0;
}
