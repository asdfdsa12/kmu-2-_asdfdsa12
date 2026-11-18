사용할 결과는 다음과 같다.

```
Data Size : 1000
Operations: 200

                  Quick Sort      AVL Tree
------------------------------------------------
Total Time        1.0967   ms      1.5110   ms
Comparisons       118673          10745
Data Movements    58011           1098
Memory Usage      11520  Bytes     39936 Bytes

Final Data Count  1100            1100

[Breakdown]                Quick Sort      AVL Tree
  Sort time (ms)           0.1261          1.2332
  Ops time (ms)            0.9706          0.2778
  Sort comparisons         11694           8706
  Ops comparisons          106979          2039
  Sort movements           7776            998
  Ops movements            50235           100
  Data structure (Bytes)   4800            35136
  Max stack (Bytes)        6720            4800
  Inserts / Deletes / Delete misses : 100 / 0 / 100 (AVL: 100 / 0 / 100)
  AVL distinct nodes (final) : 1098

------------------------------------------------

Data Size : 2000
Operations: 400

                  Quick Sort      AVL Tree
------------------------------------------------
Total Time        4.6277   ms      3.6505   ms
Comparisons       450313          24007
Data Movements    223085          2196
Memory Usage      16000  Bytes     75808 Bytes

Final Data Count  2194            2194

[Breakdown]                Quick Sort      AVL Tree
  Sort time (ms)           0.3048          3.0147
  Ops time (ms)            4.3228          0.6359
  Sort comparisons         26448           19507
  Ops comparisons          423865          4500
  Sort movements           17058           2000
  Ops movements            206027          196
  Data structure (Bytes)   9600            70208
  Max stack (Bytes)        6400            5600
  Inserts / Deletes / Delete misses : 196 / 2 / 202 (AVL: 196 / 2 / 202)
  AVL distinct nodes (final) : 2194

------------------------------------------------

Data Size : 4000
Operations: 800

                  Quick Sort      AVL Tree
------------------------------------------------
Total Time        17.6949  ms      7.3641   ms
Comparisons       1752679         52988
Data Movements    818269          4370
Memory Usage      25600  Bytes     145776 Bytes

Final Data Count  4375            4375

[Breakdown]                Quick Sort      AVL Tree
  Sort time (ms)           0.6886          5.9966
  Ops time (ms)            17.0063         1.3675
  Sort comparisons         55838           43129
  Ops comparisons          1696841         9859
  Sort movements           36945           3994
  Ops movements            781324          376
  Data structure (Bytes)   19200           139776
  Max stack (Bytes)        6400            6000
  Inserts / Deletes / Delete misses : 376 / 1 / 423 (AVL: 376 / 1 / 423)
  AVL distinct nodes (final) : 4368

------------------------------------------------

Data Size : 8000
Operations: 1600

                  Quick Sort      AVL Tree
------------------------------------------------
Total Time        66.8630  ms      11.6933  ms
Comparisons       6881840         115552
Data Movements    3399182         8760
Memory Usage      45760  Bytes     286304 Bytes

Final Data Count  8789            8789

[Breakdown]                Quick Sort      AVL Tree
  Sort time (ms)           1.1926          9.6337
  Ops time (ms)            65.6704         2.0596
  Sort comparisons         120974          94228
  Ops comparisons          6760866         21324
  Sort movements           79536           7964
  Ops movements            3319646         796
  Data structure (Bytes)   38400           279904
  Max stack (Bytes)        7360            6400
  Inserts / Deletes / Delete misses : 798 / 9 / 793 (AVL: 798 / 9 / 793)
  AVL distinct nodes (final) : 8747

------------------------------------------------

Data Size : 16000
Operations: 3200

                  Quick Sort      AVL Tree
------------------------------------------------
Total Time        265.5063 ms      25.0094  ms
Comparisons       27301316        250012
Data Movements    14317505        17493
Memory Usage      85440  Bytes     565792 Bytes

Final Data Count  17613           17613

[Breakdown]                Quick Sort      AVL Tree
  Sort time (ms)           2.7150          20.4397
  Ops time (ms)            262.7913        4.5698
  Sort comparisons         259366          204363
  Ops comparisons          27041950        45649
  Sort movements           171867          15866
  Ops movements            14145638        1627
  Data structure (Bytes)   76800           558592
  Max stack (Bytes)        8640            7200
  Inserts / Deletes / Delete misses : 1640 / 27 / 1533 (AVL: 1640 / 27 / 1533)
  AVL distinct nodes (final) : 17456

------------------------------------------------

Data Size : 20000
Operations: 4000

                  Quick Sort      AVL Tree
------------------------------------------------
Total Time        367.7823 ms      34.0807  ms
Comparisons       42264730        320477
Data Movements    21432090        21760
Memory Usage      104000 Bytes     701760 Bytes

Final Data Count  21947           21947

[Breakdown]                Quick Sort      AVL Tree
  Sort time (ms)           3.0960          28.0147
  Ops time (ms)            364.6864        6.0660
  Sort comparisons         328435          262196
  Ops comparisons          41936295        58281
  Sort movements           220125          19804
  Ops movements            21211965        1956
  Data structure (Bytes)   96000           694560
  Max stack (Bytes)        8000            7200
  Inserts / Deletes / Delete misses : 1986 / 39 / 1975 (AVL: 1986 / 39 / 1975)
  AVL distinct nodes (final) : 21705

------------------------------------------------

AVL node size : 32 Bytes
All results verified (sorted, AVL balance, array == inorder) : YES
```

# 분석

퀵 정렬의 피벗은 평균적으로 가장 좋은 성능을 보이는 중앙값 선택(median-3) 방식을 사용하여 선택하였다.

데이터의 생성과 삽입 삭제에 사용되는 난수의 값은 0 ~ 1000000의 값이며, 이로 인해 삭제되는 값이 존재하지 않을 가능성이 매우 높다.

## 두 방법의 총 수행시간 및 비교 횟수 차이

| n | 퀵 수행시간(ms) | AVL 수행시간(ms) | 퀵 비교 횟수 | AVL 비교 횟수 |
|---|---|---|---|---|---|
| 1000 | 1.0967 | 1.5110 | 118673 | 10745 |
| 2000 | 4.6277 | 3.6505 | 450313 | 24007 |
| 4000 | 17.6949 | 7.3641 | 1752679 | 52988 |
| 8000 | 66.8630 | 11.6933 | 6881840 | 115552 |
| 16000 | 265.5063 | 25.0094 | 27301316 | 250012 |
| 20000 | 367.7823 | 34.0807 | 42264730 | 320477 |

먼저, 정렬에 걸리는 수행시간의 경우, n = 20000일 때 퀵 정렬은 3ms, AVL 트리 정렬은 28ms가 소요되었다.

그러나, 삽입 삭제에 걸리는 수행시간의 경우, 퀵 정렬은 364ms, AVL 트리 정렬은 6ms가 소요되었다.

수행시간이 극단적으로 차이나는 이유로는, 정렬과 삽입 삭제시의 동작에 대한 차이로 인해 발생한다.

두 정렬 모두 정렬시에 평균적으로 O(n log n)의 시간 복잡도를 보이는 정렬이지만, AVL 트리 정렬의 경우, AVL 트리를 구축하는 과정에서 노드 할당이 필요하고, 트리의 균형을 맞추기 위해 회전을 사용한다.

그로 인해 정렬시에는 단순히 정렬만 하는 퀵 정렬과 달리, 추가적인 조치가 필요한 AVL 트리 정렬의 수행시간이 더 오래 걸린다고 볼 수 있다.

하지만, 삽입 삭제시에 퀵 정렬은 그 위치를 선형 탐색하여 찾고, 동작에 따라 원소들을 뒤로 밀거나 앞으로 당기는 조치가 필요하다.

반면, AVL 트리 정렬은 삽입 삭제시에 모든 원소를 훑을 필요가 없고, 이후 조치는 균형이 깨졌을 때의 회전 뿐이므로 수행 시간이 짧다.

그렇기에, 수행시간의 경우 정렬에서는 퀵 정렬이 이점을 가지지만, 삽입 삭제시의 수행시간까지 포함한다면 AVL 트리 정렬이 우위를 가진다.

비교 횟수의 경우, 정렬 과정에서는 퀵 정렬의 비교 횟수가 더 많긴 하지만 두 정렬 방법의 비교 횟수는 비슷한 편이다.

그러나, 삽입 삭제시의 비교 횟수는 퀵 정렬의 비교 횟수가 압도적으로 많은 것을 확인할 수 있고, 이 또한 삽입 삭제에서의 선형 탐색으로 인해 발생하는 비교 횟수이다.

고로, 총 수행시간과 비교 횟수는 삽입 삭제 연산으로 인해 퀵 정렬이 더 많은 값을 가진다.

## 두 방법의 데이터 이동 횟수

| n | 퀵 정렬 | AVL 정렬 | 퀵 삽입 삭제 | AVL 삽입 삭제 |
|---|---|---|---|---|
| 1000 | 7776 | 998 | 50235 | 100 |
| 2000 | 17058 | 2000 | 206027 | 196 |
| 4000 | 36945 | 3994 | 781324 | 376 |
| 8000 | 79536 | 7964 | 3319646 | 796 |
| 16000 | 171867 | 15866 | 14145638 | 1627 |
| 20000 | 220125 | 19804 | 21211965 | 1956 |

데이터 이동 횟수의 경우, 퀵 정렬이 AVL 트리 정렬보다 훨씬 높은 데이터 이동 횟수를 가지며, 특히 삽입 삭제시의 이동 횟수 차이가 매우 큰 것을 확인할 수 있다.

퀵 정렬에서 삽입 삭제시의 데이터 이동 횟수는 위에서 언급한 바와 같이, 데이터의 단체 이동으로 인해 크게 증가한다.

AVL 트리 정렬의 경우, 트리에 없는 값을 삽입하거나, 자식이 2개인 노드를 삭제시에만 1회를 카운트하기에 이동 횟수가 적게 나타난다.

## 배열과 AVL 트리의 메모리 사용량 차이

| n | 퀵(Bytes) | AVL(Bytes) |
|---|---|---|---|---|
| 1000 | 11520 | 39936 |
| 2000 | 16000 | 75808 |
| 4000 | 25600 | 145776 |
| 8000 | 45760 | 286304 |
| 16000 | 85440 | 565792 |
| 20000 | 104000 | 701760 |

해당 메모리 사용량은 각 정렬의 자료구조와, 재귀를 위한 스택의 메모리 사용량을 합친 값이다.

퀵 정렬은 배열을 사용하기 때문에 n이 2배가 되면 배열의 크기 또한 정확히 2배가 된다.

AVL 트리 정렬 또한 n이 2배가 되면 구조체의 크기가 약 2배가 되지만, 32바이트의 구조체 노드를 사용하기 때문에 기본적으로 구조체의 크기가 크다.

그렇기에, n이 배가 될수록 두 정렬의 메모리 사용량은 더 크게 벌어지게 되고, 그 결과 퀵 정렬의 메모리 사용량이 AVL 트리 정렬보다 더 적다는 사실을 알 수 있다.

## 데이터 크기 증가에 따른 성능 변화

n이 2배씩 증가할 때, AVL 트리 정렬의 비교 횟수, 수행시간, 데이터 이동 횟수, 메모리 사용량 또한 약 2배씩 증가함을 보인다.

그러나, 퀵 정렬의 경우, 비교 횟수와 수행시간, 삽입 삭제시의 데이터 이동 횟수에서 약 4배가량 증가하는 것을 볼 수 있다.

그렇기에, n의 크기가 배가 될수록 AVL 트리와의 격차는 점점 커질 것으로 예상된다.

## 삽입 삭제가 반복되는 환경에서 유리한 자료구조와 그 이유

위의 분석들을 종합하여 보았을 때, 퀵 정렬에서 삽입 삭제가 이루어질 경우 선형 탐색으로 모든 데이터를 훑어 위치를 찾고, 수 많은 데이터들이 이동하게 된다.

하지만, AVL 트리 정렬은 노드간의 관계를 따라 특정 원소만 훑어 위치를 찾고, 삽입 삭제시에도 일부 데이터만 이동하게 된다.

그렇기에 비교 횟수 및 수행시간, 데이터 이동 횟수 측면에서 AVL 트리 정렬이 더 유리하다고 볼 수 있다.
