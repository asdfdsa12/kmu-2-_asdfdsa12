사용할 결과는 다음과 같다.

```
Data Size : 1000

Insertion Sort       : 254898 comparisons
Shell Sort (Original): 15225 comparisons
Shell Sort (Knuth)   : 13742 comparisons

Data Size : 2000

Insertion Sort       : 1015055 comparisons
Shell Sort (Original): 36120 comparisons
Shell Sort (Knuth)   : 32684 comparisons

Data Size : 4000

Insertion Sort       : 4015402 comparisons
Shell Sort (Original): 84990 comparisons
Shell Sort (Knuth)   : 76062 comparisons

Data Size : 8000

Insertion Sort       : 16038622 comparisons
Shell Sort (Original): 200825 comparisons
Shell Sort (Knuth)   : 186864 comparisons

Data Size : 16000

Insertion Sort       : 64507719 comparisons
Shell Sort (Original): 479528 comparisons
Shell Sort (Knuth)   : 413666 comparisons

Data Size : 20000

Insertion Sort       : 99595985 comparisons
Shell Sort (Original): 622823 comparisons
Shell Sort (Knuth)   : 527689 comparisons

All results sorted correctly : YES
```

# 비교 횟수 변화 및 정렬의 성능 차이

각 정렬에서 데이터의 크기 증가에 따른 비교 횟수 변화는 다음과 같다.

| 데이터 개수 | 1000 | 2000 | 4000 | 8000 | 16000 | 20000 |
| --- |
| 삽입 정렬 | 254898 | 1015055 | 4015402 | 16038622 | 64507719 | 99595985 |
| 쉘 정렬(기본 간격) | 15225 | 36120 | 84990 | 200825 | 479528 | 622823 |
| 쉘 정렬(Knuth 수열 간격) | 13742 | 32684 | 76062 | 186864 | 413666 | 527689 |

데이터의 크기가 2배가 되면 16000 ~ 20000 구간을 제외한 삽입 정렬의 비교 횟수는 약 4배가 되는 것을 확인할 수 있다.

삽입 정렬은 인접한 데이터와 비교를 거치며 정렬을 하기에, 데이터가 1000개에서 2000개로 늘어날 경우 뒤의 데이터 1000개는 최악의 경우 앞의 데이터 1000개와 모두 비교를 할 수 있다.

삽입 정렬의 경우, 데이터의 삽입이 대략 앞에 있는 데이터 개수의 절반을 지난 뒤 삽입되므로, 비교 횟수는 최대 비교 횟수인 n(n+1)/2 의 절반, 약 n^2/4 번의 비교 횟수가 발생하게 된다.

이는 1000 ^ 2 / 4 = 250000, 2000 ^ 2 / 4 = 1000000의 값을 위의 비교 횟수와 비교하여 검증할 수 있다.

반면, 쉘 정렬의 비교 횟수는 약 2.3 ~ 2.4배가 되는 것을 확인할 수 있다.

쉘 정렬의 경우, 특정한 간격(기본 간격 = n/2)을 가지는 원소들을 하나의 집합으로 묶어 그 안에서 삽입 정렬을 하는 방식으로, 모든 집합의 정렬이 끝나면 간격을 더 촘촘하게 하여 간격이 1이 되도록 하는 방식이다.

그렇기에 데이터의 수가 2배가 되면, 간격을 줄이는 단계의 수가 1단계 추가되고, 그 결과 첫 단계의 집합 수 또한 2배가 된다.

고로 비교 횟수가 배가 되는 것은 맞지만, 간격이 1/2배가 되어 집합의 원소 개수가 2배가 되었을 땐, 이미 일부 데이터는 정렬이 되어있는 상태이므로 삽입 정렬보다는 비교횟수가 덜 증가하게 된다.

## 쉘 정렬의 간격에 따른 성능 차이

위의 데이터를 확인했을 때, 기본 간격보다 Knuth 수열을 간격으로 사용했을 때의 비교 횟수가 약 1.1배 더 적은 것을 확인할 수 있다.

Knuth 수열이란 h(1) = 1, h(k+1) = 3h(k) + 1의 값을 갖는 수열로, 이 수열을 간격으로 사용할 땐 수열의 값을 역순으로 정렬 후 사용한다.

예시로, 데이터가 1000개일 때 역순 정렬된 수열의 값은 (364, 121, 40, 13, 4, 1)로, 기본 간격인 (500, 250, 125, 62, 31, 15, 7, 3, 1)에 비해 단계 수가 3단계 적은 것을 확인할 수 있다.

하지만, 정렬의 단계 수가 적어지며 데이터가 일부 정렬된 부분 또한 감소하였고, 이러한 점으로 인해 기본 간격과 Knuth 수열 간격의 성능 차이는 약 10% 정도로 나타나게 된다.