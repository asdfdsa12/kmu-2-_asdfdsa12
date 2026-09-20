/*
 * 트리의 괄호 표기법을 입력받아 여러 정보를 출력하는 프로그램 (C 언어)
 *
 * - 노드 포인터로 이루어진 별도의 트리 자료구조를 만들지 않는다.
 * - 입력 문자열을 한 번(single pass) 스캔하면서 스택을 이용해
 *   필요한 통계(전체 노드 수, 단말/비단말 노드 수, 높이, 차수,
 *   부모/자식 관계)를 계산한다.
 * - 노드는 항상 영문 대문자 한 글자이므로, 자식 목록/부모/깊이 등은
 *   26개 크기의 배열(인덱스 = 'A' - 문자)로 관리한다.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXN 26          /* 노드는 A~Z, 최대 26개 */
#define MAXLEN 4096      /* 입력 문자열 최대 길이 */

 /* ---- 트리 정보를 저장하는 전역 배열들 (노드 인덱스 = 문자 - 'A') ---- */
static int  parentOf[MAXN];        /* 부모 노드 인덱스, 루트는 -1 */
static int  childList[MAXN][MAXN]; /* childList[i][k] = i번 노드의 k번째 자식 인덱스 */
static int  childCount[MAXN];      /* i번 노드의 자식 수 (최종 차수) */
static int  depthOf[MAXN];         /* i번 노드의 깊이 (루트 = 0) */
static int  hasChildren[MAXN];     /* 1이면 비단말 노드 */
static int  exists[MAXN];          /* 1이면 트리에 실제로 등장한 노드 */
static int  appearOrder[MAXN];     /* 등장 순서대로의 노드 인덱스 목록 */
static int  totalNodes = 0;
static int  rootIndex = -1;

/* 오류 메시지를 저장해 두었다가 마지막에 한 번에 출력 */
static char errMsg[256];

/* 공백을 제거한 입력을 저장할 버퍼 */
static char buf[MAXLEN];

/*
 * 괄호 표기법 문자열을 스캔하면서 검증 + 통계 계산을 수행한다.
 * 성공하면 0, 실패하면 -1을 반환하며 errMsg 에 이유를 남긴다.
 *
 * 사용하는 스택 2개:
 *   nodeStack[]   : 현재 조상 노드들의 경로 (부모 추적용 "노드 스택")
 *   degreeStack[] : nodeStack 과 짝을 이루는, 각 노드의 자식 수를 세는
 *                   "카운터 스택" (차수 계산에 사용)
 */
int analyze(const char* s) {
    int nodeStack[MAXN];
    int degreeStack[MAXN];
    int top = -1; /* 두 스택 공용 top 인덱스 */

    int lastNode = -1;      /* 방금 읽은 노드의 인덱스 (-1 이면 없음) */
    int justClosed = 0;     /* 방금 ')' 를 처리했는지 여부 */
    /* expect: 0 = NODE(다음에 노드 문자가 와야 함), 1 = AFTER_NODE */
    int expect = 0;

    int len = (int)strlen(s);

    if (len == 0) {
        strcpy(errMsg, "입력이 비어 있습니다.");
        return -1;
    }

    for (int i = 0; i < len; i++) {
        char ch = s[i];

        if (isalpha((unsigned char)ch)) {
            if (!isupper((unsigned char)ch)) {
                sprintf(errMsg, "'%c': 노드는 영문 대문자만 가능합니다.", ch);
                return -1;
            }
            if (expect != 0) {
                sprintf(errMsg, "'%c' 위치에 노드가 올 수 없습니다.", ch);
                return -1;
            }
            int idx = ch - 'A';
            if (exists[idx]) {
                sprintf(errMsg, "노드 '%c' 가 중복 사용되었습니다.", ch);
                return -1;
            }
            exists[idx] = 1;
            appearOrder[totalNodes++] = idx;
            depthOf[idx] = top + 1; /* 현재 스택에 쌓인 조상 수 = 깊이 */
            parentOf[idx] = -1;
            childCount[idx] = 0;

            if (top >= 0) {
                int par = nodeStack[top];
                parentOf[idx] = par;
                childList[par][childCount[par]] = idx;
                childCount[par]++;
                degreeStack[top]++;
            }
            else {
                rootIndex = idx;
            }

            lastNode = idx;
            expect = 1;
            justClosed = 0;
        }
        else if (ch == '(') {
            if (expect != 1 || lastNode == -1 || justClosed) {
                strcpy(errMsg, "'(' 의 위치가 올바르지 않습니다.");
                return -1;
            }
            hasChildren[lastNode] = 1;
            top++;
            nodeStack[top] = lastNode;   /* 노드 스택 push */
            degreeStack[top] = 0;        /* 카운터 스택 push */
            lastNode = -1;
            expect = 0;
        }
        else if (ch == ',') {
            if (expect != 1) {
                strcpy(errMsg, "',' 의 위치가 올바르지 않습니다.");
                return -1;
            }
            if (top < 0) {
                strcpy(errMsg, "최상위 레벨에는 ',' 를 사용할 수 없습니다.");
                return -1;
            }
            expect = 0;
        }
        else if (ch == ')') {
            if (expect != 1) {
                strcpy(errMsg, "')' 의 위치가 올바르지 않습니다.");
                return -1;
            }
            if (top < 0) {
                strcpy(errMsg, "짝이 맞지 않는 ')' 입니다.");
                return -1;
            }
            int closedNode = nodeStack[top];
            int deg = degreeStack[top];
            top--; /* 두 스택 동시에 pop */
            if (deg == 0) {
                sprintf(errMsg, "'%c' 의 괄호 안에 자식이 없습니다.", closedNode + 'A');
                return -1;
            }
            lastNode = closedNode;
            expect = 1;
            justClosed = 1;
        }
        else {
            sprintf(errMsg, "'%c': 허용되지 않는 문자입니다.", ch);
            return -1;
        }
    }

    if (top >= 0) {
        strcpy(errMsg, "닫히지 않은 '(' 가 있습니다.");
        return -1;
    }
    if (expect != 1 || totalNodes == 0) {
        strcpy(errMsg, "트리 표현이 완전하지 않습니다.");
        return -1;
    }

    return 0;
}

/*
 * 같은 레벨에서 왼쪽부터 오른쪽으로(BFS, 레벨 순회) 보았을 때
 * 노드 이름이 A, B, C, ... 순서로 정확히 매겨져 있는지 검사한다.
 * 큐(배열로 구현)를 사용해 레벨 순회를 수행한다.
 *
 *   예) A(B(E,F),C,D(G))  -> BFS 순서: A,B,C,D,E,F,G  (통과)
 *       C(A)              -> BFS 순서: C,A            (실패)
 *       A(D,C(B,E))       -> BFS 순서: A,D,C,B,E       (실패)
 */
int checkLevelOrder(void) {
    int queue[MAXN];
    int qh = 0, qt = 0;

    queue[qt++] = rootIndex;

    int expected = 0;
    while (qh < qt) {
        int node = queue[qh++];
        if (node != expected) {
            sprintf(errMsg,
                "노드 이름은 레벨 순서(같은 레벨에서 왼쪽->오른쪽) 기준으로 "
                "A부터 알파벳 순서대로 부여되어야 합니다. "
                "('%c' 가 나온 자리에는 '%c' 가 와야 합니다.)",
                node + 'A', expected + 'A');
            return -1;
        }
        expected++;
        for (int k = 0; k < childCount[node]; k++) {
            queue[qt++] = childList[node][k];
        }
    }
    return 0;
}

/* 왼쪽으로 눕힌 형태로 트리를 계층적으로 출력한다 (+, -, |, 들여쓰기 사용) */
void printTree(int node, const char* prefix, int isLast, int isRoot) {
    char newPrefix[512];

    if (isRoot) {
        printf("%c\n", node + 'A');
        newPrefix[0] = '\0';
    }
    else {
        printf("%s+---%c\n", prefix, node + 'A');
        if (isLast)
            sprintf(newPrefix, "%s    ", prefix);
        else
            sprintf(newPrefix, "%s|   ", prefix);
    }

    for (int k = 0; k < childCount[node]; k++) {
        int kid = childList[node][k];
        printTree(kid, newPrefix, (k == childCount[node] - 1), 0);
    }
}

/* 특정 노드(label)의 부모/자식 정보를 출력한다 */
void describeNode(char label) {
    int idx = label - 'A';

    if (idx < 0 || idx >= MAXN || !exists[idx]) {
        printf("'%c' 노드는 트리에 존재하지 않습니다.\n", label);
        return;
    }

    if (parentOf[idx] == -1) {
        printf("'%c' 의 부모 노드: 없음 (루트 노드)\n", label);
    }
    else {
        printf("'%c' 의 부모 노드: %c\n", label, parentOf[idx] + 'A');
    }

    if (childCount[idx] == 0) {
        printf("'%c' 의 자식 노드: 없음 (단말 노드)\n", label);
    }
    else {
        printf("'%c' 의 자식 노드: ", label);
        for (int k = 0; k < childCount[idx]; k++) {
            printf("%c", childList[idx][k] + 'A');
            if (k != childCount[idx] - 1) printf(", ");
        }
        printf("\n");
    }
}

int main(void) {
    printf("트리를 괄호 표기법으로 입력하세요: ");
    fflush(stdout);

    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("[오류] 입력을 읽을 수 없습니다.\n");
        return 1;
    }

    /* 공백류(개행, 스페이스, 탭) 제거 */
    char clean[MAXLEN];
    int ci = 0;
    for (int i = 0; buf[i] != '\0'; i++) {
        if (!isspace((unsigned char)buf[i])) {
            clean[ci++] = buf[i];
        }
    }
    clean[ci] = '\0';

    if (analyze(clean) != 0) {
        printf("[오류] 올바른 트리의 괄호 표기법이 아닙니다: %s\n", errMsg);
        return 1;
    }

    if (checkLevelOrder() != 0) {
        printf("[오류] 올바른 트리의 괄호 표기법이 아닙니다: %s\n", errMsg);
        return 1;
    }

    int leafCount = 0, internalCount = 0, height = 0, degree = 0;
    for (int i = 0; i < totalNodes; i++) {
        int idx = appearOrder[i];
        if (hasChildren[idx]) internalCount++; else leafCount++;
        if (depthOf[idx] + 1 > height) height = depthOf[idx]; /* 루트 = 레벨 0 */
        if (childCount[idx] > degree) degree = childCount[idx];
    }

    printf("\n");
    printf("전체 노드의 수   : %d\n", totalNodes);
    printf("단말 노드의 수   : %d\n", leafCount);
    printf("비단말 노드의 수 : %d\n", internalCount);
    printf("트리의 높이      : %d\n", height);
    printf("트리의 차수      : %d\n", degree);
    printf("\n");

    describeNode('C');
    printf("\n");

    printf("트리 구조:\n");
    printTree(rootIndex, "", 1, 1);

    return 0;
}
