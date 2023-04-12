void main() {
  if (3 > 4) {
    printint(1);
  } else {
    printint(2);
  }

  int i;
  i = 1;

  while (i <= 10) {
    printint(i);
    i = i + 1;
  }

  int j;
  for (j = 1; j <= 10; j = j + 1) {
    printint(j);
  }
}
