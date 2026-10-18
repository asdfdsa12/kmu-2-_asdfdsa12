#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define V       10      /* 정점 수 */
#define E       20      /* 간선 수 */
#define MAX_W   100     /* 가중치 범위: 1 ~ MAX_W */
#define INF     1000000000

typedef struct { int u, v, w; } Edge;

static Edge edges[E];
static int  wt[V][V];          /* 인접 행렬: 0 = 간선 없음 (가중치는 양수) */
static int  dist[V][V];        /* Floyd-Warshall 결과: 최단거리 */
static int  nxt[V][V];         /* nxt[i][j] = i에서 j로 가는 최단경로의 다음 정점 */

/* ================= 그래프 생성 ================= */

static int cmp_edge(const void* a, const void* b) {
    const Edge* x = (const Edge*)a, * y = (const Edge*)b;
    if (x->u != y->u) return x->u - y->u;
    return x->v - y->v;
}

static int find(int* p, int x) {
    while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; }
    return x;
}

static int is_connected(void) {
    int p[V], comps = V;
    for (int i = 0; i < V; i++) p[i] = i;
    for (int i = 0; i < E; i++) {
        int a = find(p, edges[i].u), b = find(p, edges[i].v);
        if (a != b) { p[a] = b; comps--; }
    }
    return comps == 1;
}

/* 연결 보장: 랜덤 신장 트리(V-1개)를 먼저 만들고 나머지 간선을 랜덤 추가 */
static void generate_graph(void) {
    int perm[V];
    do {
        memset(wt, 0, sizeof(wt));
        for (int i = 0; i < V; i++) perm[i] = i;
        for (int i = V - 1; i > 0; i--) {          /* Fisher-Yates 셔플 */
            int j = rand() % (i + 1);
            int t = perm[i]; perm[i] = perm[j]; perm[j] = t;
        }

        int k = 0;
        for (int i = 1; i < V; i++) {
            int a = perm[i], b = perm[rand() % i];
            int w = rand() % MAX_W + 1;
            wt[a][b] = wt[b][a] = w;
            edges[k].u = a < b ? a : b;
            edges[k].v = a < b ? b : a;
            edges[k].w = w;
            k++;
        }
        while (k < E) {                            /* 자기 루프/중복 간선 금지 */
            int a = rand() % V, b = rand() % V;
            if (a == b || wt[a][b]) continue;
            int w = rand() % MAX_W + 1;
            wt[a][b] = wt[b][a] = w;
            edges[k].u = a < b ? a : b;
            edges[k].v = a < b ? b : a;
            edges[k].w = w;
            k++;
        }
    } while (!is_connected());

    qsort(edges, E, sizeof(Edge), cmp_edge);       /* 출력용 정렬 (u < v 기준) */
}

/* ================= Floyd-Warshall + 경로 복원 ================= */

static void floyd_warshall(void) {
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (i == j) { dist[i][j] = 0;        nxt[i][j] = i; }
            else if (wt[i][j]) { dist[i][j] = wt[i][j]; nxt[i][j] = j; }
            else { dist[i][j] = INF;      nxt[i][j] = -1; }
        }
    }
    for (int k = 0; k < V; k++)
        for (int i = 0; i < V; i++) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < V; j++) {
                if (dist[k][j] == INF) continue;
                int d = dist[i][k] + dist[k][j];
                if (d < dist[i][j]) {
                    dist[i][j] = d;
                    nxt[i][j] = nxt[i][k];        /* i->j 의 첫 걸음 = i->k 의 첫 걸음 */
                }
            }
        }
}

/* u -> v 경로를 path[]에 복원하고 정점 수 반환 */
static int get_path(int u, int v, int* path) {
    int n = 0;
    path[n++] = u;
    while (u != v) {
        u = nxt[u][v];
        if (u < 0 || n > V) return -1;             /* 경로 없음/무한 루프 방지 */
        path[n++] = u;
    }
    return n;
}

/* ================= 검증용 Dijkstra (독립적인 구현) ================= */

static void dijkstra(int src, int* d) {
    int done[V] = { 0 };
    for (int i = 0; i < V; i++) d[i] = INF;
    d[src] = 0;
    for (int it = 0; it < V; it++) {
        int u = -1;
        for (int i = 0; i < V; i++)
            if (!done[i] && (u == -1 || d[i] < d[u])) u = i;
        if (u == -1 || d[u] == INF) break;
        done[u] = 1;
        for (int v = 0; v < V; v++)
            if (wt[u][v] && !done[v] && d[u] + wt[u][v] < d[v])
                d[v] = d[u] + wt[u][v];
    }
}

/* ================= 메인 ================= */

int main(void) {
    srand((unsigned)time(NULL));
    generate_graph();

    /* ---- 그래프 출력 ---- */
    printf("Graph\n");
    printf("Vertices : %d\n", V);
    printf("Edges    : %d\n\n", E);
    printf("Edges\n");
    for (int i = 0; i < E; i++)
        printf("(%d, %d) Weight: %d\n", edges[i].u, edges[i].v, edges[i].w);

    /* ---- 최단경로 계산 ---- */
    floyd_warshall();

    printf("\nShortest Paths\n");
    int pairs = 0, errors = 0;
    int dj[V][V];
    for (int s = 0; s < V; s++) dijkstra(s, dj[s]);

    for (int u = 0; u < V; u++) {
        for (int v = u + 1; v < V; v++) {          /* u < v 인 45쌍만 */
            int path[V + 1];
            int n = get_path(u, v, path);

            printf("\n%d -> %d\n", u, v);
            if (n < 0) { printf("Path     : (none)\n"); errors++; pairs++; continue; }

            printf("Path     : ");
            int sum = 0, valid = 1, seen[V] = { 0 };
            for (int i = 0; i < n; i++) {
                printf("%d%s", path[i], i + 1 < n ? " -> " : "\n");
                if (seen[path[i]]++) valid = 0;                    /* 정점 중복 */
                if (i + 1 < n) {
                    int w = wt[path[i]][path[i + 1]];
                    if (!w) valid = 0;                             /* 존재하지 않는 간선 */
                    sum += w;
                }
            }
            printf("Distance : %d\n", dist[u][v]);

            /* 검증: 경로 합 == Floyd 거리 == Dijkstra 거리 */
            if (!valid || sum != dist[u][v] || dist[u][v] != dj[u][v]) {
                printf("  [ERROR] 검증 실패 (경로합=%d, Dijkstra=%d)\n", sum, dj[u][v]);
                errors++;
            }
            pairs++;
        }
    }
    printf("\nTotal vertex pairs : %d\n", pairs);

    /* ---- 확인 사항 ---- */
    int self_loop = 0, dup = 0, wrong_w = 0;
    for (int i = 0; i < E; i++) {
        if (edges[i].u == edges[i].v) self_loop++;
        if (edges[i].w < 1 || edges[i].w > MAX_W) wrong_w++;
        for (int j = i + 1; j < E; j++)
            if (edges[i].u == edges[j].u && edges[i].v == edges[j].v) dup++;
    }
    int connected_all = 1;
    for (int u = 0; u < V; u++)
        for (int v = 0; v < V; v++)
            if (dist[u][v] >= INF) connected_all = 0;

    printf("\n=== Verification ===\n");
    printf("Vertices = %d (expected %d)           : %s\n", V, 10, V == 10 ? "OK" : "FAIL");
    printf("Edges    = %d (expected %d)           : %s\n", E, 20, E == 20 ? "OK" : "FAIL");
    printf("Weights in 1..%d                      : %s\n", MAX_W, wrong_w == 0 ? "OK" : "FAIL");
    printf("No self-loop / duplicate edge         : %s\n", (self_loop == 0 && dup == 0) ? "OK" : "FAIL");
    printf("Connected graph                       : %s\n", connected_all ? "OK" : "FAIL");
    printf("All 45 pairs printed                  : %s\n", pairs == 45 ? "OK" : "FAIL");
    printf("Path sum == distance (Floyd, Dijkstra): %s\n", errors == 0 ? "OK" : "FAIL");

    return errors == 0 ? 0 : 1;
}
