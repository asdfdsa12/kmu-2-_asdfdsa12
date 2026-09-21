* 배열 방식의 구현은 루트 노드가 인덱스 1번에 위치한다.

[1]

비교에 사용된 이진트리는 다음과 같다.

일반 이진트리 : A(B(D,E(,H)),C(F(I,J(K)),G(,L(,M(,N(,O))))))
완전 이진트리 : A(B(D(H,I),E(J,K)),C(F(L,M),G(N,O)))
편향 이진트리 : A(B(C(D(E(F(G(H(I(J(K(L(M(N(O))))))))))))))

먼저, 배열 방식 구현에서의 메모리 할당량은 다음과 같다.

일반 이진트리 : char 배열 2^(6+1) = 128칸 = 128바이트
완전 이진트리 : 2^(3+1) = 16칸 = 16바이트
편향 이진트리 : 2^(14+1) = 32,768칸 = 32,768바이트

포인터 방식 구현에서의 메모리 할당량은 다음과 같다.

일반 이진트리 : char 1개(정렬됨) + struct 2개 = (8+8+8)*15 = 360바이트
완전 이진트리 : 360바이트
편향 이진트리 : 360바이트

이때, 배열에서의 메모리 실제 사용량은 다음과 같다.

일반 이진트리 : 15/128 = 11.7%
완전 이진트리 : 15/16 = 93.75%
편향 이진트리 : 15/32768 = 0.0046%

완전 이진트리를 비교해봤을 때, 포인터의 할당량인 360바이트보다 훨씬 적은 16바이트를 할당받고, 실제 사용률도 93.75%로 포인터보다 우수하다.
일반 이진트리를 비교해봤을 때, 128바이트를 할당받고, 11.7%의 사용률을 가져 포인터보다 우수하다.
그러나, 편향 이진트리를 비교해봤을 때, 32768바이트를 할당받고, 0.0046%의 사용률을 가져 효율이 크게 떨어지고, 이는 포인터 방식이 더 우수하다.


[2]

배열 방식 이진트리 구현에서의 노드 정보 출력 함수는 다음과 같다.

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


포인터 방식 이진트리 구현에서의 노드 정보 출력 함수는 다음과 같다.

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

해당 함수들의 동작 방식은 다음과 같다.
1. 입력받은 노드의 이름을 배열을 훑거나 포인터를 따라가며 노드를 찾는다.
2. 찾은 이후 자식과 부모, 형제 노드를 계산한다.
여기서 2번째 단계는 배열 방식과 포인터 방식의 시간복잡도가 동일하지만, 1번에서 차이가 발생한다.

예를 들어, 노드가 26개인 오른쪽 편향 이진트리의 마지막 노드(A~Z까지의 노드중 Z)의 정보를 출력한다고 했을 때,
포인터 방식의 구현은 비교 후 이동하는 방식으로 26회의 비교가 발생한다.

그러나, 배열 방식의 구현은 편향 이진트리를 만들때 레벨과 레벨 사이에 들어갈 수 있는 노드 개수만큼 배열의 칸 수를 비우기 때문에,
2^(25+1) = 67,108,864칸의 배열이 생성되고, 이로 인해 노드 비교의 횟수 또한 67,108,864번이 발생한다.

따라서, 완전 이진트리와 포화 이진트리의 경우에는 큰 효율 차이가 없지만, 편향 이진트리의 경우 효율 차이가 꽤 발생할 수 있고, 고로 포인터 방식의 구현이 더 효율적이라고 볼 수 있다.