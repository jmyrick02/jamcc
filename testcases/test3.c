void main() {
  int a; int b; int c;
  a = b = c = 3;
  printint(a); printint(b); printint(c);

  int x; int *y;
  y = &x; *y = 5;
  printint(x);

  int n;
  n = 50;
  int **z; z = &y; **z = n;
  printint(x);
}
