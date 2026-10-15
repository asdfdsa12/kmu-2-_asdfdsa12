#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define V          100      /* 정점 수 */
#define SPARSE_E   100      /* 희소 그래프 간선 수 */
#define DENSE_E    4000     /* 밀집 그래프 간선 수 */

#define INS_OPS    10       /* 추가로 삽입할 간선 수 */
#define DEL_OPS    10       /* 삭제할 간선 수 */
#define CONN_OPS   10       /* 연결 여부를 확인할 정점 쌍 수 */
#define ADJ_OPS    1        /* 인접 정점을 출력할 정점 수 */

typedef struct { int u, v; } Pair;

/* ================= 인접 행렬 ================= */

typedef struct { int m[V][V]; } AdjMatrix;

/* 비교 1회 = matrix 원소 1개 확인 */
static int mat_has(const AdjMatrix* g, int u, int v, long* cmp) {
    (*cmp)++;
    return g->m[u][v];
}

static int mat_insert(AdjMatrix* g, int u, int v, long* cmp) {
    if (mat_has(g, u, v, cmp)) return 0;          /* 이미 존재 */
    g->m[u][v] = g->m[v][u] = 1;                  /* 대입은 비교가 아님 */
    return 1;
}

static int mat_delete(AdjMatrix* g, int u, int v, long* cmp) {
    if (!mat_has(g, u, v, cmp)) return 0;         /* 존재하지 않음 */
    g->m[u][v] = g->m[v][u] = 0;
    return 1;
}

/* matrix[u][0..V-1] 전체를 확인 -> 항상 V회 */
static int mat_adjacent(const AdjMatrix* g, int u, int* out, long* cmp) {
    int cnt = 0;
    for (int j = 0; j < V; j++) {
        (*cmp)++;
        if (g->m[u][j]) out[cnt++] = j;
    }
    return cnt;
}

/* ================= 인접 리스트 ================= */

typedef struct ANode {
    int v;
    struct ANode* next;
} ANode;

typedef struct {
    ANode* head[V];
    int node_cnt;                /* 할당된 간선 노드 수 (메모리 계산용) */
} AdjList;

/* 비교 1회 = 리스트 노드 1개 확인 */
static int list_has(const AdjList* g, int u, int v, long* cmp) {
    for (ANode* p = g->head[u]; p != NULL; p = p->next) {
        (*cmp)++;
        if (p->v == v) return 1;
    }
    return 0;
}

static void list_push_front(AdjList* g, int u, int v) {   /* 비교 없음 */
    ANode* n = (ANode*)malloc(sizeof(ANode));
    if (!n) { perror("malloc"); exit(1); }
    n->v = v;
    n->next = g->head[u];
    g->head[u] = n;
    g->node_cnt++;
}

/* u의 리스트에서 v를 찾아 삭제 (확인한 노드 수만큼 비교) */
static int list_remove_one(AdjList* g, int u, int v, long* cmp) {
    ANode* prev = NULL, * p = g->head[u];
    while (p != NULL) {
        (*cmp)++;
        if (p->v == v) {
            if (prev) prev->next = p->next;
            else      g->head[u] = p->next;
            free(p);
            g->node_cnt--;
            return 1;
        }
        prev = p;
        p = p->next;
    }
    return 0;
}

/* 존재 확인은 u의 리스트만 탐색, 없으면 양쪽 리스트 앞에 삽입 */
static int list_insert(AdjList* g, int u, int v, long* cmp) {
    if (list_has(g, u, v, cmp)) return 0;
    list_push_front(g, u, v);
    list_push_front(g, v, u);
    return 1;
}

/* 양쪽 리스트에서 모두 찾아 삭제하므로 두 번의 탐색 비교가 모두 포함됨 */
static int list_delete(AdjList* g, int u, int v, long* cmp) {
    int a = list_remove_one(g, u, v, cmp);
    int b = list_remove_one(g, v, u, cmp);
    return a && b;
}

/* u의 리스트 노드를 모두 확인 -> deg(u)회 */
static int list_adjacent(const AdjList* g, int u, int* out, long* cmp) {
    int cnt = 0;
    for (ANode* p = g->head[u]; p != NULL; p = p->next) {
        (*cmp)++;
        out[cnt++] = p->v;
    }
    return cnt;
}

static void list_free(AdjList* g) {
    for (int i = 0; i < V; i++) {
        ANode* p = g->head[i];
        while (p) { ANode* t = p->next; free(p); p = t; }
        g->head[i] = NULL;
    }
    g->node_cnt = 0;
}

/* ================= 보조 함수 ================= */

/* 두 표현이 기준(ref)과 동일한 그래프인지 검증 (비교 횟수에는 포함하지 않음) */
static int verify(const AdjMatrix* M, const AdjList* L, char ref[V][V]) {
    for (int u = 0; u < V; u++) {
        for (int v = 0; v < V; v++) {
            long dummy = 0;
            if (M->m[u][v] != ref[u][v]) return 0;
            if (list_has(L, u, v, &dummy) != ref[u][v]) return 0;
        }
    }
    return 1;
}

static int cmp_int(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

typedef struct {
    size_t mem;
    long ins, del, conn, adj;
} Result;

static void print_result(int case_no, const char* title, const Result* r) {
    printf("Case %d : %s\n", case_no, title);
    printf("Memory                         : %zu Bytes\n", r->mem);
    printf("Edge insertion comparisons     : %ld (%d ops, avg %.2f)\n",
        r->ins, INS_OPS, (double)r->ins / INS_OPS);
    printf("Edge deletion comparisons      : %ld (%d ops, avg %.2f)\n",
        r->del, DEL_OPS, (double)r->del / DEL_OPS);
    printf("Connectivity check comparisons : %ld (%d ops, avg %.2f)\n",
        r->conn, CONN_OPS, (double)r->conn / CONN_OPS);
    printf("Adjacent vertices comparisons  : %ld (%d vertex)\n\n",
        r->adj, ADJ_OPS);
}

/* ================= 그래프 한 종류에 대한 실험 ================= */

static void run(const char* name, int edge_cnt, int case_mat, int case_list) {
    static char ref[V][V];                 /* 생성/검증용 임시 기준표 (메모리 측정 제외) */
    static Pair edges[DENSE_E];
    Pair conn[CONN_OPS];
    int adj_v[ADJ_OPS];

    memset(ref, 0, sizeof(ref));

    /* 1. 무방향 단순 그래프를 랜덤 생성 (자기 루프/중복 간선 없음) */
    for (int k = 0; k < edge_cnt; ) {
        int u = rand() % V, v = rand() % V;
        if (u == v || ref[u][v]) continue;
        ref[u][v] = ref[v][u] = 1;
        edges[k].u = u; edges[k].v = v;
        k++;
    }

    /* 같은 간선 집합으로 두 표현을 구성 */
    AdjMatrix* M = (AdjMatrix*)calloc(1, sizeof(AdjMatrix));
    AdjList* L = (AdjList*)calloc(1, sizeof(AdjList));
    if (!M || !L) { perror("calloc"); exit(1); }

    for (int k = 0; k < edge_cnt; k++) {
        int u = edges[k].u, v = edges[k].v;
        M->m[u][v] = M->m[v][u] = 1;
        list_push_front(L, u, v);
        list_push_front(L, v, u);
    }

    if (!verify(M, L, ref)) { fprintf(stderr, "오류: 그래프 생성 불일치\n"); exit(1); }

    /* 메모리: 구조체 + (리스트의 경우) 간선 노드 */
    Result rm = { 0 }, rl = { 0 };
    rm.mem = sizeof(AdjMatrix);
    rl.mem = sizeof(AdjList) + (size_t)L->node_cnt * sizeof(ANode);

    /* 2. 테스트 데이터 선택 (두 표현에 동일하게 사용) */
    for (int i = 0; i < CONN_OPS; i++) {
        int u, v;
        do { u = rand() % V; v = rand() % V; } while (u == v);
        conn[i].u = u; conn[i].v = v;
    }
    for (int i = 0; i < ADJ_OPS; i++) adj_v[i] = rand() % V;

    /* 3. 연결 여부 확인 / 인접 정점 (원래 그래프 기준) */
    printf("########## %s Graph (V=%d, E=%d) ##########\n\n", name, V, edge_cnt);

    printf("[Connectivity check pairs]\n");
    for (int i = 0; i < CONN_OPS; i++) {
        long cm = 0, cl = 0;
        int a = mat_has(M, conn[i].u, conn[i].v, &cm);
        int b = list_has(L, conn[i].u, conn[i].v, &cl);
        if (a != b) { fprintf(stderr, "오류: 연결 확인 불일치\n"); exit(1); }
        rm.conn += cm; rl.conn += cl;
        printf("  (%2d, %2d) %-9s matrix=%ld list=%ld\n",
            conn[i].u, conn[i].v, a ? "connected" : "no edge", cm, cl);
    }

    printf("\n[Adjacent vertices]\n");
    for (int i = 0; i < ADJ_OPS; i++) {
        int outm[V], outl[V];
        long cm = 0, cl = 0;
        int nm = mat_adjacent(M, adj_v[i], outm, &cm);
        int nl = list_adjacent(L, adj_v[i], outl, &cl);
        qsort(outl, nl, sizeof(int), cmp_int);   /* 리스트는 삽입 역순이므로 정렬해 비교 */
        if (nm != nl || memcmp(outm, outl, nm * sizeof(int)) != 0) {
            fprintf(stderr, "오류: 인접 정점 불일치\n"); exit(1);
        }
        rm.adj += cm; rl.adj += cl;
        printf("  Vertex %d (degree %d): ", adj_v[i], nm);
        for (int j = 0; j < nm; j++) printf("%d%s", outm[j], j + 1 < nm ? " " : "");
        printf("\n");
    }

    /* 4. 간선 삽입: 현재 존재하지 않는 정점 쌍 선택 */
    printf("\n[Edge insertion pairs]\n");
    for (int i = 0; i < INS_OPS; i++) {
        int u, v;
        do { u = rand() % V; v = rand() % V; } while (u == v || ref[u][v]);
        ref[u][v] = ref[v][u] = 1;

        long cm = 0, cl = 0;
        int a = mat_insert(M, u, v, &cm);
        int b = list_insert(L, u, v, &cl);
        if (!a || !b) { fprintf(stderr, "오류: 삽입 실패\n"); exit(1); }
        rm.ins += cm; rl.ins += cl;
        printf("  (%2d, %2d) matrix=%ld list=%ld\n", u, v, cm, cl);
    }

    /* 5. 간선 삭제: 현재 존재하는 간선 선택 */
    printf("\n[Edge deletion pairs]\n");
    for (int i = 0; i < DEL_OPS; i++) {
        int u, v;
        do { u = rand() % V; v = rand() % V; } while (u == v || !ref[u][v]);
        ref[u][v] = ref[v][u] = 0;

        long cm = 0, cl = 0;
        int a = mat_delete(M, u, v, &cm);
        int b = list_delete(L, u, v, &cl);
        if (!a || !b) { fprintf(stderr, "오류: 삭제 실패\n"); exit(1); }
        rm.del += cm; rl.del += cl;
        printf("  (%2d, %2d) matrix=%ld list=%ld\n", u, v, cm, cl);
    }

    if (!verify(M, L, ref)) { fprintf(stderr, "오류: 연산 후 그래프 불일치\n"); exit(1); }

    printf("\n");
    char title[64];
    snprintf(title, sizeof(title), "%s Graph - Adjacency Matrix", name);
    print_result(case_mat, title, &rm);
    snprintf(title, sizeof(title), "%s Graph - Adjacency List", name);
    print_result(case_list, title, &rl);

    list_free(L);
    free(L);
    free(M);
}

int main(void) {
    srand((unsigned)time(NULL));

    printf("sizeof(AdjMatrix) = %zu, sizeof(AdjList) = %zu, sizeof(ANode) = %zu\n\n",
        sizeof(AdjMatrix), sizeof(AdjList), sizeof(ANode));

    run("Sparse", SPARSE_E, 1, 2);
    run("Dense", DENSE_E, 3, 4);
    return 0;
}
