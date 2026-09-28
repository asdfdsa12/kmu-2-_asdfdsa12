/*
 * 괄호 표기법으로 이진트리를 입력받아 포인터 기반 연결 자료구조로 구성하고,
 * 전위/중위/후위 순회를 "재귀 없이" 스택만으로 반복적으로 수행하는 프로그램.
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

#define MAXN 256   /* 노드 수 상한(스택 크기용). 여유 있게 잡음 */

typedef struct Node {
    char data;
    struct Node* left, * right;
} Node;

static char errMsg[256];
static int  seen[26];

/* ------------------------------------------------------------------ */
/*  괄호 표기법 파서 (재귀 하강 파서 - 순회와는 무관하게 트리를 만드는  */
/*  단계이므로 재귀 사용 제한 대상이 아님)                              */
/* ------------------------------------------------------------------ */
Node* parseSubtree(const char* s, int* pos) {
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

    Node* node = (Node*)malloc(sizeof(Node));
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
        }
        else if (s[*pos] == ')') {
            sprintf(errMsg, "'%c' 의 괄호 안이 완전히 비어 있습니다.", ch);
            return NULL;
        }
        else {
            sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
            return NULL;
        }
    }
    else if (isalpha((unsigned char)s[*pos])) {
        node->left = parseSubtree(s, pos);
        if (!node->left) return NULL;

        if (s[*pos] == ',') {
            (*pos)++;
            if (isalpha((unsigned char)s[*pos])) {
                node->right = parseSubtree(s, pos);
                if (!node->right) return NULL;
            }
            else if (s[*pos] == ')') {
                node->right = NULL;    /* "A(B,)" : 오른쪽 명시적으로 없음 */
            }
            else {
                sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
                return NULL;
            }
        }
        /* 콤마가 없으면 오른쪽은 그냥 없는 것("A(B)") */
    }
    else {
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

Node* parseTree(const char* s) {
    memset(seen, 0, sizeof(seen));
    int n = (int)strlen(s);
    if (n == 0) { strcpy(errMsg, "입력이 비어 있습니다."); return NULL; }

    int pos = 0;
    Node* root = parseSubtree(s, &pos);
    if (!root) return NULL;

    if (pos != n) {
        strcpy(errMsg, "트리 뒤에 불필요한 문자가 남아 있습니다.");
        return NULL;
    }
    return root;
}

/* ------------------------------------------------------------------ */
/*  트리 구조 출력 ( + 와 - 를 이용한 형태 )                            */
/*  자식은 부모보다 항상 공백 4칸 더 들여써서 "+---노드" 로 표시한다.    */
/*  (화면 출력용이므로 재귀를 써도 "순회는 반복적으로" 조건과 무관)      */
/* ------------------------------------------------------------------ */
void printSubtree(Node* n, const char* prefix, int isLast, int isRoot) {
    if (isRoot) printf("%c\n", n->data);
    else        printf("%s+---%c\n", prefix, n->data);

    char newPrefix[256];
    if (isRoot) newPrefix[0] = '\0';
    else        sprintf(newPrefix, "%s    ", prefix);   /* 항상 공백 4칸만 추가 */

    Node* kids[2]; int kc = 0;
    if (n->left)  kids[kc++] = n->left;
    if (n->right) kids[kc++] = n->right;

    for (int k = 0; k < kc; k++) {
        printSubtree(kids[k], newPrefix, k == kc - 1, 0);
    }
}

void printTreePlusStyle(Node* root) {
    if (root == NULL) {
        printf("트리가 비어 있습니다.\n");
        return;
    }
    printSubtree(root, "", 1, 1);
}

/* ------------------------------------------------------------------ */
/*  1. 전위 순회 (반복적, 스택 1개)                                     */
/* ------------------------------------------------------------------ */
void preorderIterative(Node* root) {
    if (!root) { printf("(빈 트리)\n"); return; }

    Node* stack[MAXN];
    int top = -1;
    stack[++top] = root;

    int first = 1;
    while (top >= 0) {
        Node* cur = stack[top--];
        printf("%s%c", first ? "" : " ", cur->data);
        first = 0;

        /* 왼쪽을 먼저 방문해야 하므로, 스택(LIFO)에는 오른쪽을 먼저 넣는다 */
        if (cur->right) stack[++top] = cur->right;
        if (cur->left)  stack[++top] = cur->left;
    }
    printf("\n");
}

/* ------------------------------------------------------------------ */
/*  2. 중위 순회 (반복적, 스택 1개)                                     */
/* ------------------------------------------------------------------ */
void inorderIterative(Node* root) {
    if (!root) { printf("(빈 트리)\n"); return; }

    Node* stack[MAXN];
    int top = -1;
    Node* cur = root;
    int first = 1;

    while (cur != NULL || top >= 0) {
        while (cur != NULL) {          /* 왼쪽 끝까지 내려가며 스택에 쌓음 */
            stack[++top] = cur;
            cur = cur->left;
        }
        cur = stack[top--];            /* 더 내려갈 수 없으면 방문 */
        printf("%s%c", first ? "" : " ", cur->data);
        first = 0;
        cur = cur->right;              /* 오른쪽 서브트리로 이동 */
    }
    printf("\n");
}

/* ------------------------------------------------------------------ */
/*  3. 후위 순회 (반복적, 스택 2개)                                     */
/*     1번 스택에서 (Root,Right,Left) 순서로 꺼내 2번 스택에 쌓으면,     */
/*     2번 스택은 아래서부터 (Left,Right,Root) 순서가 되어              */
/*     그대로 pop 하면 후위 순회 순서가 된다.                           */
/* ------------------------------------------------------------------ */
void postorderIterative(Node* root) {
    if (!root) { printf("(빈 트리)\n"); return; }

    Node* stack1[MAXN]; int top1 = -1;
    Node* stack2[MAXN]; int top2 = -1;

    stack1[++top1] = root;
    while (top1 >= 0) {
        Node* cur = stack1[top1--];
        stack2[++top2] = cur;
        if (cur->left)  stack1[++top1] = cur->left;
        if (cur->right) stack1[++top1] = cur->right;
    }

    int first = 1;
    while (top2 >= 0) {
        Node* cur = stack2[top2--];
        printf("%s%c", first ? "" : " ", cur->data);
        first = 0;
    }
    printf("\n");
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(void) {
    char buf[4096], clean[4096];

    printf("이진트리를 괄호 표기법으로 입력하세요 {예, A(B(D(H,I),E),C(F(J),G(,K)))}: ");
    fflush(stdout);
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("[오류] 입력을 읽을 수 없습니다.\n");
        return 1;
    }

    int ci = 0;
    for (int i = 0; buf[i] != '\0'; i++)
        if (!isspace((unsigned char)buf[i])) clean[ci++] = buf[i];
    clean[ci] = '\0';

    Node* root = parseTree(clean);
    if (!root) {
        printf("[오류] 올바른 이진트리의 괄호 표기법이 아닙니다: %s\n", errMsg);
        return 1;
    }

    printf("\n[1] 입력된 이진트리의 구조\n");
    printTreePlusStyle(root);

    printf("\n[2] 전위 순회 (Preorder)\n");
    printf("Preorder  : ");
    preorderIterative(root);

    printf("\n[3] 중위 순회 (Inorder)\n");
    printf("Inorder   : ");
    inorderIterative(root);

    printf("\n[4] 후위 순회 (Postorder)\n");
    printf("Postorder : ");
    postorderIterative(root);

    return 0;
}