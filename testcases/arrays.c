int main() {
  int a[10];
  int i;
  int x;

  for (i = 0; i < 10; i = i + 1) {
    a[i] = 2 * i;
    x = a[i];

    printint(i);
    printint(x);
  }

  printint(a[8] = 2 * (3 + 5));
  printint(a[8]);
}
