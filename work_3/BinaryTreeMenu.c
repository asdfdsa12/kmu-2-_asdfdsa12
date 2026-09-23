/*
 * 경로(path) 기반으로 노드를 지정하는 이진트리 관리 프로그램.
 *
 *   /A/B/C  형태로 루트부터 해당 노드까지의 데이터를 슬래시로 이어 표현.
 *
 * 명령:
 *   Insert / DATA                  (빈 트리에 루트 생성)
 *   Insert parent-path L|R DATA    (자식 노드 추가)
 *   Delete leaf-path
 *   Update target-path DATA
 *   Read parent-path
 *   Print
 *
 * 설계: 이진트리 ADT(create_btree, insert_root, insert_child, delete_node,
 * update_value, read_child, print_btree, destroy_btree)는 "이미 찾아낸
 * 노드 포인터"를 받아 기계적인 구조 변경만 수행한다. 경로 문자열을 파싱해서
 * 노드를 찾고, 형제 데이터 중복 등 규칙을 검사하는 것은 명령 처리(메뉴)
 * 계층의 책임으로 분리했다.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

 /* ------------------------------------------------------------------ */
 /*  이진트리 ADT                                                       */
 /* ------------------------------------------------------------------ */
typedef struct Node {
    char data;
    struct Node* left, * right;
} Node;

typedef struct {
    Node* root;
    int   size;   /* 최대 노드 수 (참고용) */
    int   count;  /* 현재 노드 수 */
} BTree;

BTree* create_btree(int size) {
    BTree* t = (BTree*)malloc(sizeof(BTree));
    t->root = NULL;
    t->size = size;
    t->count = 0;
    return t;
}

/* 트리가 비어 있을 때만 루트 생성 가능. 성공 0, 실패 -1 */
int insert_root(BTree* tree, char value) {
    if (tree->root != NULL) return -1;
    tree->root = (Node*)malloc(sizeof(Node));
    tree->root->data = value;
    tree->root->left = tree->root->right = NULL;
    tree->count++;
    return 0;
}

/* parent 아래 child('L'/'R') 위치에 value 노드를 추가. 성공 0, 이미 자식이 있으면 -1 */
int insert_child(BTree* tree, Node* parent, char child, char value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = value;
    newNode->left = newNode->right = NULL;

    if (child == 'L') {
        if (parent->left) { free(newNode); return -1; }
        parent->left = newNode;
    }
    else {
        if (parent->right) { free(newNode); return -1; }
        parent->right = newNode;
    }
    tree->count++;
    return 0;
}

/* leaf 노드를 삭제. parent 는 leaf의 부모(루트면 NULL). 단말이 아니면 -1 */
int delete_node(BTree* tree, Node* parent, Node* leaf) {
    if (leaf->left || leaf->right) return -1;

    if (parent == NULL) {
        tree->root = NULL;
    }
    else if (parent->left == leaf) {
        parent->left = NULL;
    }
    else {
        parent->right = NULL;
    }
    free(leaf);
    tree->count--;
    return 0;
}

int update_value(BTree* tree, Node* target, char value) {
    (void)tree;
    target->data = value;
    return 0;
}

void read_child(Node* parent, Node** outLeft, Node** outRight) {
    *outLeft = parent->left;
    *outRight = parent->right;
}

/* 왼쪽으로 눕힌 형태 출력 (이전 과제와 동일한 +, - 방식) */
void printSubtree(Node* n, const char* prefix, int isLast, int isRoot) {
    if (isRoot) printf("%c\n", n->data);
    else        printf("%s+---%c\n", prefix, n->data);

    char newPrefix[256];
    if (isRoot) newPrefix[0] = '\0';
    else        sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "|   ");

    Node* kids[2]; int kc = 0;
    if (n->left)  kids[kc++] = n->left;
    if (n->right) kids[kc++] = n->right;

    for (int k = 0; k < kc; k++) {
        printSubtree(kids[k], newPrefix, k == kc - 1, 0);
    }
}

void print_btree(BTree* tree) {
    if (tree->root == NULL) {
        printf("트리가 비어 있습니다.\n");
        return;
    }
    printSubtree(tree->root, "", 1, 1);
}

void freeSubtree(Node* n) {
    if (!n) return;
    freeSubtree(n->left);
    freeSubtree(n->right);
    free(n);
}

void destroy_btree(BTree* tree) {
    freeSubtree(tree->root);
    tree->root = NULL;
    free(tree);
}

/* ------------------------------------------------------------------ */
/*  경로 해석 (메뉴 계층의 책임)                                        */
/* ------------------------------------------------------------------ */

/*
 * "/A/B/C" 형태의 경로를 해석해 해당 노드를 반환한다.
 * outParent 에는 그 노드의 부모(루트면 NULL)를 담아 반환한다.
 * 경로가 존재하지 않으면 NULL 을 반환한다.
 */
Node* resolvePathWithParent(BTree* tree, const char* path, Node** outParent) {
    *outParent = NULL;
    if (path[0] != '/') return NULL;
    if (tree->root == NULL) return NULL;

    char buf[256];
    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* tok = strtok(buf, "/");
    if (!tok) return NULL;                       /* "/" 단독: 노드 없음 */
    if (strlen(tok) != 1 || !isupper((unsigned char)tok[0])) return NULL;
    if (tree->root->data != tok[0]) return NULL;  /* 루트 이름 불일치 */

    Node* cur = tree->root;
    Node* parent = NULL;

    tok = strtok(NULL, "/");
    while (tok) {
        if (strlen(tok) != 1 || !isupper((unsigned char)tok[0])) return NULL;
        char c = tok[0];
        if (cur->left && cur->left->data == c) {
            parent = cur; cur = cur->left;
        }
        else if (cur->right && cur->right->data == c) {
            parent = cur; cur = cur->right;
        }
        else {
            return NULL;   /* 경로상의 노드를 찾을 수 없음 */
        }
        tok = strtok(NULL, "/");
    }

    *outParent = parent;
    return cur;
}

/* ------------------------------------------------------------------ */
/*  명령어 처리 (메뉴)                                                  */
/* ------------------------------------------------------------------ */

int matchCmd(const char* token, const char* full, const char* shortForm) {
    return strcmp(token, full) == 0 || strcmp(token, shortForm) == 0;
}

int isSingleUpper(const char* s) {
    return s != NULL && strlen(s) == 1 && isupper((unsigned char)s[0]);
}

void handleInsert(BTree* tree, char** args, int nargs) {
    if (nargs == 2) {
        /* 루트 생성: Insert / DATA */
        const char* path = args[0];
        const char* data = args[1];

        if (strcmp(path, "/") != 0) {
            printf("[오류] 잘못된 명령 형식입니다.\n");
            return;
        }
        if (!isSingleUpper(data)) {
            printf("[오류] 데이터는 영문 대문자 한 글자여야 합니다.\n");
            return;
        }
        if (tree->root != NULL) {
            printf("[오류] 이미 루트 노드가 존재합니다.\n");
            return;
        }
        insert_root(tree, data[0]);
        return;
    }

    if (nargs == 3) {
        /* 일반 삽입: Insert parent-path L|R DATA */
        const char* path = args[0];
        const char* childStr = args[1];
        const char* data = args[2];

        char child;
        if (strcmp(childStr, "L") == 0) child = 'L';
        else if (strcmp(childStr, "R") == 0) child = 'R';
        else { printf("[오류] 자식 위치는 L 또는 R 이어야 합니다.\n"); return; }

        if (!isSingleUpper(data)) {
            printf("[오류] 데이터는 영문 대문자 한 글자여야 합니다.\n");
            return;
        }

        Node* dummyParent;
        Node* parent = resolvePathWithParent(tree, path, &dummyParent);
        if (!parent) {
            printf("[오류] 존재하지 않는 노드 경로입니다: %s\n", path);
            return;
        }

        Node* targetSlot = (child == 'L') ? parent->left : parent->right;
        if (targetSlot) {
            printf("[오류] 해당 위치에 이미 자식 노드가 존재합니다.\n");
            return;
        }

        Node* sibling = (child == 'L') ? parent->right : parent->left;
        if (sibling && sibling->data == data[0]) {
            printf("[오류] 동일한 부모의 다른 자식이 이미 같은 데이터를 가지고 있습니다.\n");
            return;
        }

        insert_child(tree, parent, child, data[0]);
        return;
    }

    printf("[오류] 인자의 개수가 올바르지 않습니다.\n");
}

void handleDelete(BTree* tree, char** args, int nargs) {
    if (nargs != 1) { printf("[오류] 인자의 개수가 올바르지 않습니다.\n"); return; }

    Node* parent;
    Node* target = resolvePathWithParent(tree, args[0], &parent);
    if (!target) {
        printf("[오류] 존재하지 않는 노드 경로입니다: %s\n", args[0]);
        return;
    }
    if (target->left || target->right) {
        printf("[오류] 단말 노드가 아니므로 삭제할 수 없습니다.\n");
        return;
    }
    delete_node(tree, parent, target);
}

void handleUpdate(BTree* tree, char** args, int nargs) {
    if (nargs != 2) { printf("[오류] 인자의 개수가 올바르지 않습니다.\n"); return; }

    const char* path = args[0];
    const char* data = args[1];
    if (!isSingleUpper(data)) {
        printf("[오류] 데이터는 영문 대문자 한 글자여야 합니다.\n");
        return;
    }

    Node* parent;
    Node* target = resolvePathWithParent(tree, path, &parent);
    if (!target) {
        printf("[오류] 존재하지 않는 노드 경로입니다: %s\n", path);
        return;
    }

    if (parent) {
        Node* sibling = (parent->left == target) ? parent->right : parent->left;
        if (sibling && sibling->data == data[0]) {
            printf("[오류] 변경 결과 형제 노드와 데이터가 중복됩니다.\n");
            return;
        }
    }

    update_value(tree, target, data[0]);
}

void handleRead(BTree* tree, char** args, int nargs) {
    if (nargs != 1) { printf("[오류] 인자의 개수가 올바르지 않습니다.\n"); return; }

    Node* parent;
    Node* target = resolvePathWithParent(tree, args[0], &parent);
    if (!target) {
        printf("[오류] 존재하지 않는 노드 경로입니다: %s\n", args[0]);
        return;
    }

    Node* L, * R;
    read_child(target, &L, &R);

    if (!L && !R) {
        printf("자식이 없습니다 (단말 노드).\n");
        return;
    }
    int first = 1;
    if (L) { printf("%c(L)", L->data); first = 0; }
    if (R) { if (!first) printf(", "); printf("%c(R)", R->data); }
    printf("\n");
}

void handlePrint(BTree* tree, int nargs) {
    if (nargs != 0) { printf("[오류] 인자의 개수가 올바르지 않습니다.\n"); return; }
    print_btree(tree);
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(void) {
    BTree* tree = create_btree(1000);
    char line[512];

    printf("이진트리 관리 프로그램입니다. (Insert/Delete/Update/Read/Print), (I/D/U/R/P)\n");

    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;   /* EOF */

        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        char* tokens[10];
        int nt = 0;
        char* tok = strtok(line, " \t");
        while (tok && nt < 10) { tokens[nt++] = tok; tok = strtok(NULL, " \t"); }
        if (nt == 0) continue;

        char* cmd = tokens[0];
        char** args = tokens + 1;
        int nargs = nt - 1;

        if (matchCmd(cmd, "Insert", "I"))      handleInsert(tree, args, nargs);
        else if (matchCmd(cmd, "Delete", "D")) handleDelete(tree, args, nargs);
        else if (matchCmd(cmd, "Update", "U")) handleUpdate(tree, args, nargs);
        else if (matchCmd(cmd, "Read", "R"))   handleRead(tree, args, nargs);
        else if (matchCmd(cmd, "Print", "P"))  handlePrint(tree, nargs);
        else printf("[오류] 알 수 없는 명령어입니다: %s\n", cmd);
    }

    destroy_btree(tree);
    return 0;
}