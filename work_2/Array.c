/*
 * 이진트리의 괄호 표기법을 입력받아 "배열"만으로 표현하고
 * [1] 출력  [2] 트리 정보  [3] 형태 판별 을 구현한다.
 *
 * 배열 인덱스 규칙: 루트 = 1, 어떤 노드가 idx 이면
 *   왼쪽 자식 = 2*idx, 오른쪽 자식 = 2*idx + 1
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

static char errMsg[256];
static int  seen[26];

static char *g_arr = NULL;   /* 1번 인덱스가 루트 */
static int   g_capacity = 0; /* 현재 할당된 크기 (0 ~ capacity-1 사용 가능) */

/* idx 인덱스를 쓸 수 있을 때까지 배열을 동적으로 2배씩 늘린다. */
void ensureCapacity(int idx) {
    if (idx < g_capacity) return;
    int newCap = (g_capacity == 0) ? 2 : g_capacity;
    while (idx >= newCap) newCap *= 2;
    char *newArr = (char *)realloc(g_arr, newCap);
    memset(newArr + g_capacity, 0, newCap - g_capacity);
    g_arr = newArr;
    g_capacity = newCap;
}

/*
 * s[*pos] 부터 시작하는 서브트리를 파싱하여 g_arr[idx] 이하에 채워 넣는다.
 * 성공하면 1, 실패하면 0을 반환하고 errMsg 를 채운다.
 */
int parseIntoArray(const char *s, int *pos, int idx) {
    char ch = s[*pos];

    if (!isalpha((unsigned char)ch)) {
        sprintf(errMsg, "'%c' 위치에 노드 이름이 와야 합니다.", ch ? ch : '?');
        return 0;
    }
    if (!isupper((unsigned char)ch)) {
        sprintf(errMsg, "'%c': 노드는 영문 대문자만 가능합니다.", ch);
        return 0;
    }
    if (seen[ch - 'A']) {
        sprintf(errMsg, "노드 '%c' 가 중복 사용되었습니다.", ch);
        return 0;
    }
    seen[ch - 'A'] = 1;

    ensureCapacity(idx);
    g_arr[idx] = ch;
    (*pos)++;

    if (s[*pos] != '(') return 1;      /* 단말 노드 */
    (*pos)++;                          /* '(' 소비 */

    if (s[*pos] == ',') {
        (*pos)++;                      /* ',' 소비: 왼쪽 없음 */
        if (isalpha((unsigned char)s[*pos])) {
            if (!parseIntoArray(s, pos, idx * 2 + 1)) return 0;
        } else if (s[*pos] == ')') {
            sprintf(errMsg, "'%c' 의 괄호 안이 완전히 비어 있습니다.", ch);
            return 0;
        } else {
            sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
            return 0;
        }
    } else if (isalpha((unsigned char)s[*pos])) {
        if (!parseIntoArray(s, pos, idx * 2)) return 0;

        if (s[*pos] == ',') {
            (*pos)++;
            if (isalpha((unsigned char)s[*pos])) {
                if (!parseIntoArray(s, pos, idx * 2 + 1)) return 0;
            } else if (s[*pos] == ')') {
                /* "A(B,)" : 오른쪽 명시적으로 없음 */
            } else {
                sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
                return 0;
            }
        }
        /* 콤마가 없으면 오른쪽은 그냥 없는 것("A(B)") */
    } else {
        sprintf(errMsg, "'%c' 뒤 괄호 안의 내용이 올바르지 않습니다.", ch);
        return 0;
    }

    if (s[*pos] != ')') {
        sprintf(errMsg, "'%c' 의 괄호가 닫히지 않았습니다.", ch);
        return 0;
    }
    (*pos)++;   /* ')' 소비 */
    return 1;
}

int buildArray(const char *s) {
    memset(seen, 0, sizeof(seen));
    int n = (int)strlen(s);
    if (n == 0) { strcpy(errMsg, "입력이 비어 있습니다."); return 0; }

    int pos = 0;
    if (!parseIntoArray(s, &pos, 1)) return 0;

    if (pos != n) {
        strcpy(errMsg, "트리 뒤에 불필요한 문자가 남아 있습니다.");
        return 0;
    }
    return 1;
}

/* [1] 배열 기반 출력 (왼쪽으로 눕힌 형태) */
void printArrayNode(int idx, const char *prefix, int isLast, int isRoot) {
    if (isRoot) printf("%c\n", g_arr[idx]);
    else        printf("%s+---%c\n", prefix, g_arr[idx]);

    char newPrefix[256];
    if (isRoot) newPrefix[0] = '\0';
    else        sprintf(newPrefix, "%s%s", prefix, isLast ? "    " : "|   ");

    int li = 2 * idx, ri = 2 * idx + 1;
    int kids[2], kc = 0;
    if (li < g_capacity && g_arr[li]) kids[kc++] = li;
    if (ri < g_capacity && g_arr[ri]) kids[kc++] = ri;

    for (int k = 0; k < kc; k++) {
        printArrayNode(kids[k], newPrefix, k == kc - 1, 0);
    }
}

/* [2] 배열 기반 트리 정보 */
void printArrayInfo(void) {
    int total = 0, leaf = 0, maxIndex = 0, degree = 0;

    for (int i = 1; i < g_capacity; i++) {
        if (!g_arr[i]) continue;
        total++;
        if (i > maxIndex) maxIndex = i;

        int li = 2 * i, ri = 2 * i + 1;
        int childCnt = 0;
        if (li < g_capacity && g_arr[li]) childCnt++;
        if (ri < g_capacity && g_arr[ri]) childCnt++;

        if (childCnt == 0) leaf++;
        if (childCnt > degree) degree = childCnt;
    }

    int height = 0;
    while ((1 << (height + 1)) <= maxIndex) height++;   /* height = floor(log2(maxIndex)), 0부터 시작 */

    printf("전체 노드의 수   : %d\n", total);
    printf("단말 노드의 수   : %d\n", leaf);
    printf("비단말 노드의 수 : %d\n", total - leaf);
    printf("트리의 높이      : %d\n", height);
    printf("트리의 차수      : %d\n", degree);
}

/* [3] 배열 기반 형태 판별 */
void printArrayShape(void) {
    int total = 0, maxIndex = 0;
    for (int i = 1; i < g_capacity; i++) {
        if (g_arr[i]) { total++; if (i > maxIndex) maxIndex = i; }
    }
    int height = 0;
    while ((1 << (height + 1)) <= maxIndex) height++;   /* 높이 0부터 시작 */

    /* 왼쪽부터 채워짐(마지막 레벨만 부분적으로 채워질 수 있음) :
     * 1..total 이 모두 채워져 있고, 그 뒤는 모두 비어 있어야 함 */
    int leftFilled = 1;
    for (int i = 1; i <= total; i++) if (!g_arr[i]) { leftFilled = 0; break; }
    if (leftFilled) {
        for (int i = total + 1; i < g_capacity; i++) if (g_arr[i]) { leftFilled = 0; break; }
    }

    /* 모든 레벨이 빈틈없이 꽉 참 : 노드 수가 2^(height+1) - 1 */
    int allLevelsFull = leftFilled && (total == (1 << (height + 1)) - 1);

    /*
     * 편향 이진트리 : 두 자식을 모두 가진 노드가 없어야 할 뿐 아니라,
     * 한쪽 자식만 있는 노드들이 전부 "같은 방향"이어야 한다.
     * (예: A(,B(C)) 는 A는 오른쪽, B는 왼쪽으로 방향이 섞여 있으므로 편향이 아니다.)
     * dir: 0 = 아직 방향 안 정해짐, 1 = 왼쪽, 2 = 오른쪽
     */
    int skewed = (total > 1);
    int dir = 0;
    for (int i = 1; i < g_capacity && skewed; i++) {
        if (!g_arr[i]) continue;
        int li = 2 * i, ri = 2 * i + 1;
        int hasL = (li < g_capacity && g_arr[li]);
        int hasR = (ri < g_capacity && g_arr[ri]);
        if (hasL && hasR) { skewed = 0; break; }
        if (hasL || hasR) {
            int thisDir = hasL ? 1 : 2;
            if (dir == 0) dir = thisDir;
            else if (dir != thisDir) { skewed = 0; break; }
        }
    }

    /* 완전 이진트리 = 모든 레벨이 꽉 참 / 포화 이진트리 = 마지막 레벨만 왼쪽부터 채워짐 */
    printf("완전 이진트리 여부 : %s\n", allLevelsFull ? "예" : "아니오");
    printf("포화 이진트리 여부 : %s\n", leftFilled    ? "예" : "아니오");
    printf("편향 이진트리 여부 : %s\n", skewed        ? "예" : "아니오");
}

int findIndexByChar(char target) {
    for (int i = 1; i < g_capacity; i++) {
        if (g_arr[i] == target) return i;
    }
    return -1;
}

/* 특정 노드(target)의 자식 / 부모 / 형제 노드를 출력한다. */
void printNodeRelations(char target) {
    int idx = findIndexByChar(target);
    if (idx == -1) {
        printf("'%c' 노드는 트리에 존재하지 않습니다.\n", target);
        return;
    }

    int li = 2 * idx, ri = 2 * idx + 1;
    int hasL = (li < g_capacity && g_arr[li]);
    int hasR = (ri < g_capacity && g_arr[ri]);

    printf("'%c' 의 자식 노드: ", target);
    if (!hasL && !hasR) {
        printf("없음\n");
    } else {
        int first = 1;
        if (hasL) { printf("%c", g_arr[li]); first = 0; }
        if (hasR) { if (!first) printf(", "); printf("%c", g_arr[ri]); }
        printf("\n");
    }

    if (idx == 1) {
        printf("'%c' 의 부모 노드: 없음 (루트 노드)\n", target);
        printf("'%c' 의 형제 노드: 없음 (루트 노드)\n", target);
    } else {
        int pIdx = idx / 2;
        printf("'%c' 의 부모 노드: %c\n", target, g_arr[pIdx]);

        int sibIdx = (idx % 2 == 0) ? idx + 1 : idx - 1;
        int hasSib = (sibIdx < g_capacity && g_arr[sibIdx]);
        if (hasSib) printf("'%c' 의 형제 노드: %c\n", target, g_arr[sibIdx]);
        else        printf("'%c' 의 형제 노드: 없음\n", target);
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

    if (!buildArray(clean)) {
        printf("[오류] 올바른 이진트리의 괄호 표기법이 아닙니다: %s\n", errMsg);
        return 1;
    }

    printf("\n[1] 트리 출력\n");
    printArrayNode(1, "", 1, 1);
    printf("\n[2] 트리 정보\n");
    printArrayInfo();
    printf("\n[3] 트리 형태 판별\n");
    printArrayShape();

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
            printNodeRelations(target);
        }
    }

    return 0;
}
