/*
 * 이진트리의 괄호 표기법을 입력받아 "포인터 기반 연결 자료구조"로 표현하고
 * [1] 출력  [2] 트리 정보  [3] 형태 판별 을 구현한다.
 *
 * 괄호 표기법 (이진트리):
 *   - 단말 노드            : A
 *   - 왼쪽 자식만          : A(B)
 *   - 오른쪽 자식만(왼쪽 빔): A(,C)
 *   - 양쪽 자식 모두        : A(B,C)
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Node {
    char data;
    struct Node *left, *right;
} Node;

static char errMsg[256];
static int  seen[26];

/*
 * s[*pos] 부터 시작하는 서브트리를 파싱하여 노드를 만든다.
 * 실패하면 NULL 을 반환하고 errMsg 를 채운다.
 */
Node *parseSubtree(const char *s, int *pos) {
    char ch = s[*pos];

    if (!isalpha((unsigned char)ch)) {
        sprintf(errMsg, "'%c' 위치에 노드 이름이 와야 합니다.", ch ? ch : '?');
        return NULL;
    }
    if (!isupper((unsigned char)ch)) {
        sprintf(errMsg, "'%c': 노드는 영문 대문자만 가능합니다.", ch);
        return NULL;
    }
    if (seen[ch - 'A']) {
        sprintf(errMsg, "노드 '%c' 가 중복 사용되었습니다.", ch);
        return NULL;
    }
    seen[ch - 'A'] = 1;

    Node *node = (Node *)malloc(sizeof(Node));
    node->data = ch;
    node->left = node->right = NULL;
    (*pos)++;

    if (s[*pos] != '(') return node;   /* 단말 노드 */
    (*pos)++;                          /* '(' 소비 */

    if (s[*pos] == ',') {
        (*pos)++;                      /* ',' 소비: 왼쪽 없음 */
        if (isalpha((unsigned char)s[*pos])) {
            node->right = parseSubtree(s, pos);
            if (!node->right) return NULL;
        } else if (s[*pos] == ')') {
            sprintf(errMsg, "'%c' 의 괄호 안이 완전히 비어 있습니다.", ch);
            return NULL;
        } else {
            sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
            return NULL;
        }
    } else if (isalpha((unsigned char)s[*pos])) {
        node->left = parseSubtree(s, pos);
        if (!node->left) return NULL;

        if (s[*pos] == ',') {
            (*pos)++;
            if (isalpha((unsigned char)s[*pos])) {
                node->right = parseSubtree(s, pos);
                if (!node->right) return NULL;
            } else if (s[*pos] == ')') {
                node->right = NULL;    /* "A(B,)" : 오른쪽 명시적으로 없음 */
            } else {
                sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
                return NULL;
            }
        }
        /* 콤마가 없으면 오른쪽은 그냥 없는 것("A(B)") */
    } else {
        sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
        return NULL;
    }

    if (s[*pos] != ')') {
        sprintf(errMsg, "'%c' 의 괄호가 닫히지 않았습니다.", ch);
        return NULL;
    }
    (*pos)++;   /* ')' 소비 */
    return node;
}

Node *parseTree(const char *s) {
    memset(seen, 0, sizeof(seen));
    int n = (int)strlen(s);
    if (n == 0) { strcpy(errMsg, "입력이 비어 있습니다."); return NULL; }

    int pos = 0;
    Node *root = parseSubtree(s, &pos);
    if (!root) return NULL;

    if (pos != n) {
        strcpy(errMsg, "트리 뒤에 불필요한 문자가 남아 있습니다.");
        return NULL;
    }
    return root;
}

/* [1] 포인터 기반 출력 (왼쪽으로 눕힌 형태) */
void printPtrNode(Node *n, const char *prefix, int isLast, int isRoot) {
    if (isRoot) printf("%c\n", n->data);
    else        printf("%s+---%c\n", prefix, n->data);

    char newPrefix[256];
    if (isRoot) newPrefix[0] = '\0';
    else        sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "|   ");

    Node *kids[2]; int kc = 0;
    if (n->left)  kids[kc++] = n->left;
    if (n->right) kids[kc++] = n->right;

    for (int k = 0; k < kc; k++) {
        printPtrNode(kids[k], newPrefix, k == kc - 1, 0);
    }
}

int ptrHeight(Node *n) {
    if (!n) return 0;
    int l = ptrHeight(n->left);
    int r = ptrHeight(n->right);
    return 1 + (l > r ? l : r);
}

/* [2] 포인터 기반 트리 정보 : 재귀로 전체/단말/차수 집계 */
void collectPtrInfo(Node *n, int *total, int *leaf, int *degree) {
    if (!n) return;
    (*total)++;
    int childCnt = (n->left ? 1 : 0) + (n->right ? 1 : 0);
    if (childCnt == 0) (*leaf)++;
    if (childCnt > *degree) *degree = childCnt;
    collectPtrInfo(n->left, total, leaf, degree);
    collectPtrInfo(n->right, total, leaf, degree);
}

void printPtrInfo(Node *root) {
    int total = 0, leaf = 0, degree = 0;
    collectPtrInfo(root, &total, &leaf, &degree);
    int height = ptrHeight(root);

    printf("전체 노드의 수   : %d\n", total);
    printf("단말 노드의 수   : %d\n", leaf);
    printf("비단말 노드의 수 : %d\n", total - leaf);
    printf("트리의 높이      : %d\n", height-1);
    printf("트리의 차수      : %d\n", degree);
}

/* [3] 포인터 기반 형태 판별 */

/* 완전 이진트리 : 큐(배열로 구현)를 이용한 레벨 순회.
 * NULL 을 만난 이후에 실제 노드가 또 나오면 완전 이진트리가 아니다. */
int isCompletePtr(Node *root) {
    if (!root) return 1;

    Node *queue[1 << 16];   /* 노드 수가 최대 26개이므로 충분히 큰 여유 크기 */
    int qh = 0, qt = 0;
    queue[qt++] = root;
    int seenNull = 0;

    while (qh < qt) {
        Node *cur = queue[qh++];
        if (cur == NULL) {
            seenNull = 1;
        } else {
            if (seenNull) return 0;
            queue[qt++] = cur->left;
            queue[qt++] = cur->right;
        }
    }
    return 1;
}

/*
 * 편향 이진트리 판별: 두 자식을 모두 가진 노드가 없어야 할 뿐 아니라,
 * 한쪽 자식만 있는 노드들이 전부 "같은 방향"이어야 한다.
 * (예: A(,B(C)) 는 A는 오른쪽, B는 왼쪽으로 방향이 섞여 있으므로 편향이 아니다.)
 *
 * dir: 0 = 아직 방향 안 정해짐, 1 = 왼쪽으로 편향, 2 = 오른쪽으로 편향
 * 모순 없이 끝까지 검사되면 1, 도중에 어긋나면 0을 반환한다.
 */
int checkSkewed(Node *n, int *dir) {
    if (!n) return 1;

    int childCnt = (n->left ? 1 : 0) + (n->right ? 1 : 0);
    if (childCnt == 2) return 0;             /* 두 자식 모두 있으면 편향 아님 */

    if (childCnt == 1) {
        int thisDir = n->left ? 1 : 2;
        if (*dir == 0) *dir = thisDir;
        else if (*dir != thisDir) return 0;  /* 방향이 도중에 바뀜 */
    }

    return checkSkewed(n->left, dir) && checkSkewed(n->right, dir);
}

void printPtrShape(Node *root) {
    int total = 0, leaf = 0, degree = 0;
    collectPtrInfo(root, &total, &leaf, &degree);
    int height = ptrHeight(root);

    int complete = isCompletePtr(root);
    int perfect  = complete && (total == (1 << height) - 1);

    int dir = 0;
    int skewed = (total > 1) && checkSkewed(root, &dir);

    printf("완전 이진트리 여부 : %s\n", perfect ? "예" : "아니오");
    printf("포화 이진트리 여부 : %s\n", complete  ? "예" : "아니오");
    printf("편향 이진트리 여부 : %s\n", skewed   ? "예" : "아니오");
}

/*
 * 트리에서 target 이름을 가진 노드를 찾는다. 찾으면 그 부모(parentOut)와
 * 부모의 왼쪽 자식이었는지 여부(isLeftOut)도 함께 채워 반환한다.
 */
Node *findNodeRec(Node *cur, Node *parent, char target, Node **parentOut, int *isLeftOut) {
    if (!cur) return NULL;
    if (cur->data == target) {
        *parentOut = parent;
        *isLeftOut = (parent && parent->left == cur) ? 1 : 0;
        return cur;
    }
    Node *found = findNodeRec(cur->left, cur, target, parentOut, isLeftOut);
    if (found) return found;
    return findNodeRec(cur->right, cur, target, parentOut, isLeftOut);
}

/* 특정 노드(target)의 자식 / 부모 / 형제 노드를 출력한다. */
void printNodeRelations(Node *root, char target) {
    Node *parent = NULL;
    int isLeft = 0;
    Node *found = findNodeRec(root, NULL, target, &parent, &isLeft);

    if (!found) {
        printf("'%c' 노드는 트리에 존재하지 않습니다.\n", target);
        return;
    }

    printf("'%c' 의 자식 노드: ", target);
    if (!found->left && !found->right) {
        printf("없음\n");
    } else {
        int first = 1;
        if (found->left)  { printf("%c", found->left->data); first = 0; }
        if (found->right) { if (!first) printf(", "); printf("%c", found->right->data); }
        printf("\n");
    }

    if (!parent) {
        printf("'%c' 의 부모 노드: 없음 (루트 노드)\n", target);
        printf("'%c' 의 형제 노드: 없음 (루트 노드)\n", target);
    } else {
        printf("'%c' 의 부모 노드: %c\n", target, parent->data);
        Node *sibling = isLeft ? parent->right : parent->left;
        if (sibling) printf("'%c' 의 형제 노드: %c\n", target, sibling->data);
        else         printf("'%c' 의 형제 노드: 없음\n", target);
    }
}

int main(void) {
    char buf[4096], clean[4096];

    printf("이진트리를 괄호 표기법으로 입력하세요: ");
    fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("[오류] 입력을 읽을 수 없습니다.\n");
        return 1;
    }

    int ci = 0;
    for (int i = 0; buf[i] != '\0'; i++)
        if (!isspace((unsigned char)buf[i])) clean[ci++] = buf[i];
    clean[ci] = '\0';

    Node *root = parseTree(clean);
    if (!root) {
        printf("[오류] 올바른 이진트리의 괄호 표기법이 아닙니다: %s\n", errMsg);
        return 1;
    }

    printf("\n[1] 트리 출력\n");
    printPtrNode(root, "", 1, 1);
    printf("\n[2] 트리 정보\n");
    printPtrInfo(root);
    printf("\n[3] 트리 형태 판별\n");
    printPtrShape(root);

    printf("\n정보를 조회할 노드를 입력하세요: ");
    fflush(stdout);
    char qbuf[64];
    if (fgets(qbuf, sizeof(qbuf), stdin)) {
        char target = 0;
        for (int i = 0; qbuf[i] != '\0'; i++) {
            if (isalpha((unsigned char)qbuf[i])) { target = qbuf[i]; break; }
        }
        if (target == 0) {
            printf("올바른 노드 이름을 입력하지 않았습니다.\n");
        } else {
            printf("\n");
            printNodeRelations(root, target);
        }
    }

    return 0;
}
