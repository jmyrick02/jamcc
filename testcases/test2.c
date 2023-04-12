int main() {
  int a;
  a = 18;
  printint(a);

  int *b;
  b = &a;
  printint(b);

  int c;
  c = 12;
  printint(c);
  int *f;
  f = &c;
  int d;
  d = *f;
  printint(d);

  int g; int *h; int **j; int ***k;
  g = 5; h = &g; j = &h; k = &j;
  int n; int *m; int **l;
  l = *k; m = *l; n = *m;
  printint(n);
}
