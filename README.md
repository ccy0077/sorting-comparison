# Sorting Algorithm Comparison

고급알고리즘 과제용 정렬 알고리즘 비교 프로젝트입니다.

## 비교 대상

- 수업에서 배운 정렬: 삽입 정렬(Insertion Sort), 퀵 정렬(Quick Sort)
- AI를 활용해 추가 학습한 정렬: 병합 정렬(Merge Sort)

## 비교 항목

- 실행 시간
- 비교 및 이동 횟수
- 추가 메모리와 재귀 깊이
- 안정성(stability)
- 입력 형태별 특성: 무작위, 정렬, 역정렬, 중복값

## 빌드 및 실행

Linux/macOS:

```bash
gcc -std=c11 -O2 -Wall -Wextra -pedantic src/sort_compare.c -o sort_compare
./sort_compare > results/results.csv
```

Windows(MinGW):

```bash
gcc -std=c11 -O2 -Wall -Wextra src/sort_compare.c -o sort_compare.exe
sort_compare.exe > results\results.csv
```

출력의 `RESULT` 행은 성능 비교 결과이고, `STABILITY` 행의 위반 횟수가 0이면 안정 정렬입니다.

## 폴더 구성

```text
sorting-comparison/
├── src/sort_compare.c
├── results/results.csv
├── report/정렬_알고리즘_비교_보고서.pdf
├── README.md
└── .gitignore
```

## 제출 전 확인

1. 이 폴더를 GitHub 저장소에 업로드합니다.
2. 보고서 첫 페이지의 GitHub URL을 실제 주소로 교체합니다.
3. GitHub의 `Code > Download ZIP`으로 내려받은 ZIP과 PDF 보고서를 제출합니다.
