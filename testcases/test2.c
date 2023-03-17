int main() {
  int a;
  a = 18;
  print a;

  int *b;
  b = &a;
  print b;

  int c;
  c = 12;
  print c;
  int *f;
  f = &c;
  int d;
  d = *f;
  print d;

  int g; int *h; int **j; int ***k;
  g = 5; h = &g; j = &h; k = &j;
  int n; int *m; int **l;
  l = *k; m = *l; n = *m;
  print n;
}
