사용할 결과는 다음과 같다.

```
Data Size : 1000

Quick Sort (First)    : 13636 comparisons
Quick Sort (Random)   : 12540 comparisons
Quick Sort (Median-3) : 11694 comparisons

Data Size : 2000

Quick Sort (First)    : 25349 comparisons
Quick Sort (Random)   : 25715 comparisons
Quick Sort (Median-3) : 25376 comparisons

Data Size : 4000

Quick Sort (First)    : 58980 comparisons
Quick Sort (Random)   : 63399 comparisons
Quick Sort (Median-3) : 57116 comparisons

Data Size : 8000

Quick Sort (First)    : 135489 comparisons
Quick Sort (Random)   : 135234 comparisons
Quick Sort (Median-3) : 120885 comparisons

Data Size : 16000

Quick Sort (First)    : 280908 comparisons
Quick Sort (Random)   : 275411 comparisons
Quick Sort (Median-3) : 257196 comparisons

Data Size : 20000

Quick Sort (First)    : 381312 comparisons
Quick Sort (Random)   : 342362 comparisons
Quick Sort (Median-3) : 328849 comparisons

All results sorted correctly : YES

```

# 분석

| 데이터 크기 | 1000 | 2000 | 4000 | 8000 | 16000 | 20000 |
| --- | --- | --- | --- | --- | --- | --- |
| 첫 번째 원소 선택 | 13636 | 25349 | 58980 | 135489 | 280908 | 381312 |
| 무작위 원소 선택 | 12540 | 25715 | 63399 | 135234 | 275411 | 342362 |
| 세 원소의 중앙값 선택 | 11694 | 25376 | 57116 | 120885 | 257196 | 328849 |

해당 결과에서, 세 원소의 중앙값을 피벗 값으로 선택하였을 때의 비교 횟수는 첫 번째 원소를 선택했을 때보다 적고(n=2000 제외),
무작위 원소 선택의 경우, 선택되는 원소에 따라 비교 횟수가 더 적거나 많음을 보인다.

정렬 구간에서 피벗 값을 첫 번째 원소 또는 무작위 원소를 사용할 때, 그 원소 값이 정렬 구간의 중앙값이라면 원소보다 크거나 작은 값을 가진 원소가 대체로 일정하게 존재하므로 적은 분할 및 비교 횟수가 발생하게 된다.

그러나, 양극단에 위치하는 값이라면 원소보다 크거나 작은 값을 가진 원소가 적으므로 분할되는 집합의 원소 개수가 극단적으로 나뉘게 된다.

예를 들어, 1 ~ 10까지의 원소가 들어있는 집합에서 가장 큰 원소를 선택할 때, 퀵 정렬에 의해 10은 가장 오른쪽에, 1 ~ 9까지의 원소는 그 왼쪽에 위치되고, 이후 새로운 두 집합으로 분할되게 된다.

이렇게 되면, 원소가 n-1개인 집합과 1개뿐인 집합으로 계속해서 나뉘게 되어 많은 정렬의 단계가 발생하게 되고, 그로 인해 비교횟수 또한 증가하게 된다.

이러한 최악의 상황을 방지하기 위해, 세 원소의 중앙값을 선택하는 방법을 사용할 수 있고, 위의 결과로 이것이 안정적인 방법임을 검증하였다.

고로, 첫 번째 원소 선택과 무작위 원소 선택은 선택된 원소가 가장 크거나 작을 때 성능 저하가 발생하며 그 가능성이 대체로 높고,
중앙값 선택 방식은 퀵 정렬의 특성에 의해 가장 효과적인 방법이며, 비교 횟수 또한 가장 적은 것으로 나타났다.

-------------

반복적 구현에 의해 발생한 결과는 다음과 같다.

```
Data Size : 10000
Pivot     : Median-of-Three
Repetitions : 1000 (average per run)

Recursive Quick Sort : 1.2588 ms
Iterative Quick Sort : 1.1536 ms

Results sorted correctly and identical : YES
```

# 분석

각 방법에 사용된 데이터의 크기는 1만개이고, 모두 중앙값 선택 방식을 사용하였다. 정렬 횟수는 실행당 1천번이다.

재귀적 구현을 사용한 퀵 정렬의 경우 평균 1.2588ms, 반복적 구현을 사용한 퀵 정렬의 경우 평균 1.1536ms가 소요되었다.

두 방법의 실행시간 차이는 그렇게 크지 않은데, 이는 실행 시간의 대부분을 차지하는 비교, 교환은 두 방법이 똑같이 수행하기 때문이다.

그러나, 반복적 구현의 경우는 최악의 상황인 극단적인 원소 개수의 두 집합으로 분할되는 상황에 재귀적 구현에 비해 유연하다.

이는, 분할된 집합을 스택에 넣어 관리하고, 각 집합을 적절하게 push, pop하여 스택의 깊이를 log n으로 유지, 그로 인해 재귀적 구현보다 안정적인 실행 환경을 보인다.
