void main() {
  int a; int b; int c;
  a = b = c = 3;
  print a; print b; print c;

  int x; int *y;
  y = &x; *y = 5;
  print x;

  int n;
  n = 50;
  int **z; z = &y; **z = n;
  print x;
}
