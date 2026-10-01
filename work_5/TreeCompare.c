#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VAL   1000
#define DATA_CNT  100
#define KEY_CNT   50

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

/* ---------- BST ---------- */

static Node* new_node(int v) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) { perror("malloc"); exit(1); }
    n->data = v;
    n->left = n->right = NULL;
    return n;
}

/* 삽입: 기존 노드와 비교할 때마다 *cmp 증가.
   (같은지 / 작은지를 하나의 "숫자 비교"로 센다.) */
static Node* bst_insert(Node* root, int v, long* cmp) {
    if (root == NULL) return new_node(v);

    Node* cur = root;
    while (1) {
        (*cmp)++;
        if (v < cur->data) {
            if (cur->left == NULL) { cur->left = new_node(v); break; }
            cur = cur->left;
        }
        else {                       /* 값이 모두 다르므로 v > cur->data */
            if (cur->right == NULL) { cur->right = new_node(v); break; }
            cur = cur->right;
        }
    }
    return root;
}

/* 탐색: 노드 방문 1회 = 비교 1회 (같다/작다/크다 3방향 판단을 1회로 계산) */
static int bst_search(Node* root, int key, long* cmp) {
    Node* cur = root;
    *cmp = 0;
    while (cur != NULL) {
        (*cmp)++;
        if (key == cur->data) return 1;
        else if (key < cur->data) cur = cur->left;
        else cur = cur->right;
    }
    return 0;
}

static int bst_height(Node* n) {
    if (!n) return 0;
    int l = bst_height(n->left), r = bst_height(n->right);
    return 1 + (l > r ? l : r);
}

static void bst_free(Node* n) {
    if (!n) return;
    bst_free(n->left);
    bst_free(n->right);
    free(n);
}

/* ---------- 순차 탐색 ---------- */

static int seq_search(const int* arr, int n, int key, long* cmp) {
    *cmp = 0;
    for (int i = 0; i < n; i++) {
        (*cmp)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

/* ---------- main ---------- */

int main(void) {
    int arr[DATA_CNT];
    int used[MAX_VAL + 1] = { 0 };      /* 중복 방지용 표 (비교 횟수에는 포함하지 않음) */
    int keys[KEY_CNT];
    Node* root = NULL;
    long build_cmp = 0;

    srand((unsigned)time(NULL));

    /* 1. 서로 다른 정수 100개 생성 -> 배열 + BST */
    for (int i = 0; i < DATA_CNT; ) {
        int v = rand() % (MAX_VAL + 1);
        if (used[v]) continue;        /* 중복이면 다시 생성 */
        used[v] = 1;
        arr[i++] = v;
        root = bst_insert(root, v, &build_cmp);
    }

    printf("=== 생성된 %d개의 정수 (발생 순서) ===\n", DATA_CNT);
    for (int i = 0; i < DATA_CNT; i++) {
        printf("%4d%s", arr[i], (i % 10 == 9) ? "\n" : " ");
    }
    printf("\nBST 생성 총 비교 횟수 : %ld\n", build_cmp);
    printf("BST 높이              : %d\n\n", bst_height(root));

    /* 2. 탐색 키 50개 생성 (존재/미존재 모두 가능, 중복 허용) */
    for (int i = 0; i < KEY_CNT; i++)
        keys[i] = rand() % (MAX_VAL + 1);

    printf("=== 생성된 %d개의 탐색 대상 ===\n", KEY_CNT);
    for (int i = 0; i < KEY_CNT; i++) {
        printf("%4d%s", keys[i], (i % 10 == 9) ? "\n" : " ");
    }
    printf("\n");

    /* 3~4. 탐색 및 비교 횟수 측정 */
    long seq_total = 0, bst_total = 0;
    int found_cnt = 0;
    long seq_found_total = 0, seq_fail_total = 0;
    long bst_found_total = 0, bst_fail_total = 0;

    printf("=== 탐색 결과 ===\n");
    printf("%-4s %-10s %-8s %-18s %-18s\n",
        "No.", "Search Key", "Result", "Seq Comparisons", "BST Comparisons");
    printf("------------------------------------------------------------------\n");

    for (int i = 0; i < KEY_CNT; i++) {
        long sc, bc;
        int sf = seq_search(arr, DATA_CNT, keys[i], &sc);
        int bf = bst_search(root, keys[i], &bc);

        /* 두 방법의 결과는 항상 같아야 함 */
        if (sf != bf) {
            fprintf(stderr, "오류: 탐색 결과 불일치 (key=%d)\n", keys[i]);
            return 1;
        }

        seq_total += sc;
        bst_total += bc;
        if (sf) { found_cnt++; seq_found_total += sc; bst_found_total += bc; }
        else { seq_fail_total += sc; bst_fail_total += bc; }

        printf("%-4d %-10d %-8s %-18ld %-18ld\n",
            i + 1, keys[i], sf ? "Found" : "Not Found", sc, bc);
    }

    int fail_cnt = KEY_CNT - found_cnt;

    /* 요약 */
    printf("\nNumber of searches: %d (Found: %d, Not Found: %d)\n\n",
        KEY_CNT, found_cnt, fail_cnt);

    printf("Sequential Search\n");
    printf("  Total comparisons   : %ld\n", seq_total);
    printf("  Average comparisons : %.2f\n", (double)seq_total / KEY_CNT);
    if (found_cnt) printf("  Avg (Found only)    : %.2f\n", (double)seq_found_total / found_cnt);
    if (fail_cnt)  printf("  Avg (Not Found only): %.2f\n", (double)seq_fail_total / fail_cnt);

    printf("\nBST Search\n");
    printf("  Total comparisons   : %ld\n", bst_total);
    printf("  Average comparisons : %.2f\n", (double)bst_total / KEY_CNT);
    if (found_cnt) printf("  Avg (Found only)    : %.2f\n", (double)bst_found_total / found_cnt);
    if (fail_cnt)  printf("  Avg (Not Found only): %.2f\n", (double)bst_fail_total / fail_cnt);

    /* 5. 생성 비용을 포함한 분석 */
    long bst_all = build_cmp + bst_total;
    printf("\n=== 생성 비용을 포함한 성능 비교 ===\n");
    printf("배열 저장 비용 (비교 횟수)      : 0\n");
    printf("BST 생성 비용 (비교 횟수)       : %ld\n", build_cmp);
    printf("순차 탐색 총 비교 (50회)        : %ld\n", seq_total);
    printf("BST 탐색 총 비교 (50회)         : %ld\n", bst_total);
    printf("BST 생성 + 탐색 총 비교         : %ld\n", bst_all);
    printf("순차 탐색 대비 BST(생성 포함)   : %.2f%%\n", 100.0 * bst_all / seq_total);

    if (bst_all < seq_total)
        printf("결론: 이번 실행에서는 생성 비용을 포함해도 BST가 %ld회 적게 비교했습니다.\n",
            seq_total - bst_all);
    else
        printf("결론: 이번 실행에서는 생성 비용 때문에 순차 탐색이 %ld회 적게 비교했습니다.\n",
            bst_all - seq_total);

    if (seq_total > bst_total) {
        double per_search_gain = (double)(seq_total - bst_total) / KEY_CNT;
        printf("탐색 1회당 절약량 %.2f회 -> 손익분기 탐색 횟수 약 %.1f회\n",
            per_search_gain, build_cmp / per_search_gain);
    }

    bst_free(root);
    return 0;
}