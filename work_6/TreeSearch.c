#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VAL   1000
#define GEN_CNT   100
#define KEY_CNT   50

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
    int height;                 /* AVL 전용 (노드 수 기준, 리프 = 1) */
} Node;

static Node* new_node(int v) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) { perror("malloc"); exit(1); }
    n->data = v;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

/* ================= 배열 ================= */

/* 순차 탐색: 비교 1회 = 배열 원소 1개와 비교 */
static int seq_search(const int* arr, int n, int key, long* cmp) {
    for (int i = 0; i < n; i++) {
        (*cmp)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

/* ================= BST ================= */

/* 삽입 성공 시 1, 중복이면 0. 노드 방문 1회 = 비교 1회 */
static int bst_insert(Node** root, int v, long* cmp) {
    Node** pp = root;
    while (*pp != NULL) {
        (*cmp)++;
        if (v == (*pp)->data) return 0;
        pp = (v < (*pp)->data) ? &(*pp)->left : &(*pp)->right;
    }
    *pp = new_node(v);
    return 1;
}

/* BST/AVL 공용 탐색 */
static int tree_search(Node* root, int key, long* cmp) {
    Node* cur = root;
    while (cur != NULL) {
        (*cmp)++;
        if (key == cur->data) return 1;
        cur = (key < cur->data) ? cur->left : cur->right;
    }
    return 0;
}

/* 노드 수 기준 높이 (빈 트리 0, 루트만 1) */
static int tree_height(Node* n) {
    if (!n) return 0;
    int l = tree_height(n->left), r = tree_height(n->right);
    return 1 + (l > r ? l : r);
}

static void tree_free(Node* n) {
    if (!n) return;
    tree_free(n->left);
    tree_free(n->right);
    free(n);
}

/* ================= AVL ================= */
/* 높이/균형 계수 계산, 회전 판단은 숫자 비교 횟수에 포함하지 않는다. */

static int h(Node* n) { return n ? n->height : 0; }

static void update_height(Node* n) {
    int l = h(n->left), r = h(n->right);
    n->height = 1 + (l > r ? l : r);
}

static int balance_factor(Node* n) { return h(n->left) - h(n->right); }

static Node* rotate_right(Node* y) {        /* LL */
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

static Node* rotate_left(Node* x) {         /* RR */
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

/* *inserted: 실제 삽입되면 1, 중복이면 0 */
static Node* avl_insert(Node* node, int v, long* cmp, int* inserted) {
    if (node == NULL) {
        *inserted = 1;
        return new_node(v);
    }

    (*cmp)++;                                /* 저장된 값과 삽입 값의 비교 */
    if (v == node->data) { *inserted = 0; return node; }
    else if (v < node->data) node->left = avl_insert(node->left, v, cmp, inserted);
    else                     node->right = avl_insert(node->right, v, cmp, inserted);

    if (!*inserted) return node;             /* 중복이면 구조 변화 없음 */

    update_height(node);
    int bf = balance_factor(node);

    if (bf > 1) {
        if (balance_factor(node->left) < 0)  /* LR */
            node->left = rotate_left(node->left);
        return rotate_right(node);           /* LL (또는 LR의 2단계) */
    }
    if (bf < -1) {
        if (balance_factor(node->right) > 0) /* RL */
            node->right = rotate_right(node->right);
        return rotate_left(node);            /* RR (또는 RL의 2단계) */
    }
    return node;
}

/* AVL 속성 검증 (디버그/신뢰성용) */
static int avl_check(Node* n, int lo, int hi) {
    if (!n) return 1;
    if (n->data <= lo || n->data >= hi) return 0;
    int d = h(n->left) - h(n->right);
    if (d < -1 || d > 1) return 0;
    return avl_check(n->left, lo, n->data) && avl_check(n->right, n->data, hi);
}

/* ================= main ================= */

int main(void) {
    int arr[GEN_CNT];
    int arr_len = 0;
    int gen[GEN_CNT];
    int keys[KEY_CNT];
    Node* bst = NULL, * avl = NULL;
    long arr_build = 0, bst_build = 0, avl_build = 0;
    int dup_cnt = 0;

    srand((unsigned)time(NULL));

    /* 1~2. 난수 100번 생성, 세 자료구조에 같은 순서로 처리 */
    for (int i = 0; i < GEN_CNT; i++) {
        int v = rand() % (MAX_VAL + 1);
        gen[i] = v;

        /* 배열: 순차 탐색으로 중복 확인 후 끝에 추가 */
        int dup_arr = seq_search(arr, arr_len, v, &arr_build);
        if (!dup_arr) arr[arr_len++] = v;

        int ins_bst = bst_insert(&bst, v, &bst_build);

        int ins_avl = 0;
        avl = avl_insert(avl, v, &avl_build, &ins_avl);

        if (dup_arr == ins_bst || dup_arr == ins_avl) {  /* dup_arr=1 <-> ins=0 */
            fprintf(stderr, "오류: 중복 판정 불일치 (v=%d)\n", v);
            return 1;
        }
        if (dup_arr) dup_cnt++;
    }

    printf("=== 생성된 %d개의 정수 (발생 순서) ===\n", GEN_CNT);
    for (int i = 0; i < GEN_CNT; i++)
        printf("%4d%s", gen[i], (i % 10 == 9) ? "\n" : " ");

    printf("\n저장된 값의 수        : %d\n", arr_len);
    printf("중복된 값의 수        : %d\n", dup_cnt);

    printf("\n생성시 비교 횟수\n");
    printf("  배열       : %6ld\n", arr_build);
    printf("  BST        : %6ld\n", bst_build);
    printf("  AVL 트리   : %6ld\n", avl_build);

    printf("\n구조의 길이&높이\n");
    printf("  배열 길이     : %d\n", arr_len);
    printf("  BST 높이      : %d\n", tree_height(bst));
    printf("  AVL 트리 높이   : %d\n", tree_height(avl));

    if (!avl_check(avl, -1, MAX_VAL + 1) || tree_height(avl) != h(avl)) {
        fprintf(stderr, "오류: AVL 트리 속성 위반\n");
        return 1;
    }

    /* 4. 탐색 대상 50개 */
    for (int i = 0; i < KEY_CNT; i++)
        keys[i] = rand() % (MAX_VAL + 1);

    printf("\n=== 생성된 %d개의 탐색 대상 ===\n", KEY_CNT);
    for (int i = 0; i < KEY_CNT; i++)
        printf("%4d%s", keys[i], (i % 10 == 9) ? "\n" : " ");

    /* 5. 탐색 및 비교 횟수 */
    long seq_total = 0, bst_total = 0, avl_total = 0;
    int found_cnt = 0;

    printf("\n=== 탐색 결과 ===\n");
    printf("%-4s %-10s %-10s %-10s %-10s %-10s\n",
        "No.", "Key", "Result", "순차 비교", "BST 비교", "AVL 비교");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < KEY_CNT; i++) {
        long sc = 0, bc = 0, ac = 0;
        int sf = seq_search(arr, arr_len, keys[i], &sc);
        int bf = tree_search(bst, keys[i], &bc);
        int af = tree_search(avl, keys[i], &ac);

        if (sf != bf || sf != af) {
            fprintf(stderr, "오류: 탐색 결과 불일치 (key=%d)\n", keys[i]);
            return 1;
        }

        seq_total += sc; bst_total += bc; avl_total += ac;
        if (sf) found_cnt++;

        printf("%-4d %-10d %-10s %-9ld %-9ld %-9ld\n",
            i + 1, keys[i], sf ? "Found" : "Not Found", sc, bc, ac);
    }

    printf("\n탐색 결과 : %d (성공: %d, 실패: %d)\n",
        KEY_CNT, found_cnt, KEY_CNT - found_cnt);

    printf("\n순차 탐색\n");
    printf("  총 비교 횟수   : %ld\n", seq_total);
    printf("  평균 비교 횟수 : %.2f\n", (double)seq_total / KEY_CNT);
    printf("\nBST 탐색\n");
    printf("  총 비교 횟수   : %ld\n", bst_total);
    printf("  평균 비교 횟수 : %.2f\n", (double)bst_total / KEY_CNT);
    printf("\nAVL 트리 탐색\n");
    printf("  총 비교 횟수   : %ld\n", avl_total);
    printf("  평균 비교 횟수 : %.2f\n", (double)avl_total / KEY_CNT);

    /* 생성 비용 + 탐색 비용 */
    printf("\n=== 생성 + 탐색 총 비교 횟수 ===\n");
    printf("  배열 : %ld + %ld = %ld\n", arr_build, seq_total, arr_build + seq_total);
    printf("  BST   : %ld + %ld = %ld\n", bst_build, bst_total, bst_build + bst_total);
    printf("  AVL 트리   : %ld + %ld = %ld\n", avl_build, avl_total, avl_build + avl_total);

    tree_free(bst);
    tree_free(avl);
    return 0;
}
