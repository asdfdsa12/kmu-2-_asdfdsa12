분석 과정에서 사용할 실행 결과는 다음과 같다.

```
sizeof(AdjMatrix) = 40000, sizeof(AdjList) = 808, sizeof(ANode) = 16

########## Sparse Graph (V=100, E=100) ##########

[Connectivity check pairs]
  (64, 87) no edge   matrix=1 list=5
  (76, 40) no edge   matrix=1 list=1
  (37, 57) no edge   matrix=1 list=5
  (30, 15) no edge   matrix=1 list=1
  (43, 19) no edge   matrix=1 list=2
  (85, 32) no edge   matrix=1 list=6
  (26, 88) no edge   matrix=1 list=1
  (27, 25) no edge   matrix=1 list=4
  (24, 57) no edge   matrix=1 list=3
  ( 6, 52) no edge   matrix=1 list=2

[Adjacent vertices]
  Vertex 82 (degree 3): 40 85 92

[Edge insertion pairs]
  (62, 45) matrix=1 list=1
  (31, 34) matrix=1 list=3
  (62, 59) matrix=1 list=2
  (83, 60) matrix=1 list=0
  (48, 21) matrix=1 list=2
  (76, 87) matrix=1 list=1
  (49, 69) matrix=1 list=2
  (24, 59) matrix=1 list=3
  (99, 39) matrix=1 list=1
  ( 2, 71) matrix=1 list=2

[Edge deletion pairs]
  (62, 59) matrix=1 list=3
  ( 2, 92) matrix=1 list=3
  (50, 37) matrix=1 list=4
  (64, 55) matrix=1 list=2
  (64, 70) matrix=1 list=4
  (83, 60) matrix=1 list=2
  (98, 24) matrix=1 list=4
  (64, 96) matrix=1 list=6
  (78, 27) matrix=1 list=7
  (27, 13) matrix=1 list=2

Case 1 : Sparse Graph - Adjacency Matrix
Memory                         : 40000 Bytes
Edge insertion comparisons     : 10 (10 ops, avg 1.00)
Edge deletion comparisons      : 10 (10 ops, avg 1.00)
Connectivity check comparisons : 10 (10 ops, avg 1.00)
Adjacent vertices comparisons  : 100 (1 vertex)

Case 2 : Sparse Graph - Adjacency List
Memory                         : 4008 Bytes
Edge insertion comparisons     : 17 (10 ops, avg 1.70)
Edge deletion comparisons      : 37 (10 ops, avg 3.70)
Connectivity check comparisons : 30 (10 ops, avg 3.00)
Adjacent vertices comparisons  : 3 (1 vertex)

########## Dense Graph (V=100, E=4000) ##########

[Connectivity check pairs]
  (69, 65) connected matrix=1 list=79
  (78, 98) connected matrix=1 list=34
  (82, 64) connected matrix=1 list=67
  (74, 22) connected matrix=1 list=31
  ( 5, 19) connected matrix=1 list=47
  (48, 21) no edge   matrix=1 list=81
  (85,  4) connected matrix=1 list=52
  (62, 93) connected matrix=1 list=47
  (60, 15) connected matrix=1 list=8
  (89, 21) connected matrix=1 list=74

[Adjacent vertices]
  Vertex 94 (degree 78): 2 3 5 6 8 9 10 11 12 14 15 18 19 20 21 24 26 27 28 29 30 31 32 33 34 35 36 38 39 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 62 63 64 65 68 69 71 75 76 77 78 80 81 82 83 84 85 86 88 89 90 91 92 93 95 96 97 98 99

[Edge insertion pairs]
  (52, 24) matrix=1 list=81
  ( 4, 70) matrix=1 list=78
  (76, 74) matrix=1 list=81
  (32, 79) matrix=1 list=78
  (39, 79) matrix=1 list=83
  (55, 35) matrix=1 list=83
  (98, 91) matrix=1 list=81
  (90, 44) matrix=1 list=85
  (12, 69) matrix=1 list=81
  (70, 48) matrix=1 list=83

[Edge deletion pairs]
  (65, 85) matrix=1 list=136
  (15, 32) matrix=1 list=10
  (99, 30) matrix=1 list=112
  (62, 90) matrix=1 list=160
  (85, 23) matrix=1 list=98
  (48,  5) matrix=1 list=135
  (57, 47) matrix=1 list=105
  (97, 99) matrix=1 list=87
  (96, 27) matrix=1 list=37
  (12, 54) matrix=1 list=21

Case 3 : Dense Graph - Adjacency Matrix
Memory                         : 40000 Bytes
Edge insertion comparisons     : 10 (10 ops, avg 1.00)
Edge deletion comparisons      : 10 (10 ops, avg 1.00)
Connectivity check comparisons : 10 (10 ops, avg 1.00)
Adjacent vertices comparisons  : 100 (1 vertex)

Case 4 : Dense Graph - Adjacency List
Memory                         : 128808 Bytes
Edge insertion comparisons     : 814 (10 ops, avg 81.40)
Edge deletion comparisons      : 901 (10 ops, avg 90.10)
Connectivity check comparisons : 520 (10 ops, avg 52.00)
Adjacent vertices comparisons  : 78 (1 vertex)
```

# 결과

## 메모리
희소 그래프와 밀집 그래프의 경우에서 인접 행렬과 인접 리스트를 사용하였을 때, 각 메모리 사용량은 다음과 같다.

- 희소, 행렬 = 40000 bytes
- 희소, 리스트 = 4008 bytes
- 밀집, 행렬 = 40000 bytes
- 밀집, 리스트 = 128808 bytes

인접 행렬의 경우, 간선의 존재 유무를 저장하기 위한 2차원 배열의 행과 열의 개수가 100개, 즉 1만 칸의 배열이 생성되고, int 배열이기에 4 * 10000 = 40000 bytes가 사용되게 된다.

인접 리스트의 경우, 정점 리스트 크기(노드 구조체(정점 개수*8) + 노드 개수 int(4+4(패딩))) = 808 bytes, 노드 1개 크기(next 포인터 구조체(8) + int(4+4)) = 16 bytes이고, 간선은 각 정점마다 저장되므로 2번 저장된다.

고로, 인접 리스트의 메모리 사용량은 808 + 2 * 간선 개수 * 16 가 되고, 그 결과 희소 그래프의 경우는 808 + 2 * 100 * 16 = 4008 bytes, 밀집 그래프의 경우는 808 + 2 * 4000 * 16 = 128808 bytes가 사용되었다.

## 비교 횟수
간선의 삽입과 삭제에 발생한 비교 횟수는 다음과 같다.

간선의 삽입과 삭제는 모두 10회씩 이루어졌다.

- 희소, 행렬 = 10회, 10회
- 희소, 리스트 = 17회, 37회
- 밀집, 행렬 = 10회, 10회
- 밀집, 리스트 = 814회, 901회

행렬의 두 정점 연결 여부 확인의 비교 횟수는 삽입, 삭제시에 행렬의 인덱스를 직접 방문하므로 비교 횟수가 고정 1회 발생한다.

리스트의 두 정점 연결 여부 확인의 비교 횟수는 삽입, 삭제시에 인접 정점을 따라가며 연결 여부를 확인하기에, 비교 횟수가 일정량(0(삭제시 1) ~ 인접 노드 개수)만큼 발생하게 되고, 삭제시에는 두 정점 각각의 연결 여부를 비교하기 때문에 비교가 추가로 발생하게 된다.

특정 정점의 인접 정점 출력에 필요한 비교 횟수는 다음과 같다. 

- 희소, 행렬 = 100회
- 희소, 리스트 = 3회
- 밀집, 행렬 = 100회
- 밀집, 리스트 = 78회

행렬의 경우 선택한 정점의 모든 행 또는 열의 연결 여부를 확인해야 하므로 비교 횟수가 고정 100회 발생하게 된다.

리스트의 경우 인접한 정점이 포인터로 연결되어 있기 때문에, 인접한 정점의 개수만큼 비교 횟수가 발생하게 된다.

## 분석

### 희소 그래프

희소 그래프에서의 인접 행렬과 리스트의 차이는 다음과 같다.

- 메모리: 40000 bytes, 4008 bytes
- 삽입 삭제 비교 횟수: 10/10회, 17/37회
- 특정 정점의 인접 정점 출력 비교 횟수: 100회, 3회

행렬은 100*100의 2차원 배열을 사용하여 고정적으로 40000 bytes를 사용하지만, 리스트는 간선의 개수만큼 메모리를 사용하기에 행렬보다 적은 메모리 사용량을 보이고 있다.

삽입 삭제시 비교 횟수의 경우, 행렬은 인덱스를 지정해 직접 찾아가지만(=1회), 리스트는 인접한 정점을 따라가며 비교하기에 비교 횟수가 고정되지 않기에 행렬보다 더 많은 비교 횟수를 보이고 있다.

특정 정점의 인접 정점 출력시 비교 횟수의 경우, 정점의 모든 행 또는 열을 비교하는 행렬과 달리 인접한 정점만 확인하는 리스트의 비교 횟수가 훨씬 적은 것을 알 수 있다.

### 밀집 그래프 

밀집 그래프에서의 인접 행렬과 리스트의 차이는 다음과 같다.

- 메모리: 40000 bytes, 128808 bytes
- 삽입 삭제 비교 횟수: 10/10회, 814/901회
- 특정 정점의 인접 정점 출력 비교 횟수: 100회, 78회

행렬의 경우 희소 그래프의 결과와 동일하나, 리스트의 경우 희소 그래프의 결과와 차이가 크게 발생하였다.

리스트의 메모리 사용량은 희소 그래프의 인접 리스트와 달리, 간선의 개수가 증가하며 오히려 행렬보다 더 많은 메모리 사용량을 보이고 있다.

삽입 삭제시 비교 횟수의 경우, 간선의 개수가 증가하며 정점과 인접한 정점의 개수 또한 증가하였고, 연결 여부 확인에서 발생한 비교 횟수 또한 크게 증가하였다.

특정 정점의 인접 정점 출력시 비교 횟수의 경우, 인접 정점의 개수가 증가하며 비교 횟수도 증가하였지만, 여전히 고정적으로 100회의 비교가 발생하는 행렬보다는 비교 횟수가 적은 것을 확인할 수 있다.

### 종합

이러한 희소, 밀집 그래프에서의 인접 행렬, 인접 리스트의 성능 분석을 보았을 때, 희소 그래프에서는 인접 리스트를, 밀집 그래프에서는 인접 행렬을 사용하는 것이 적절하다고 볼 수 있다.   
