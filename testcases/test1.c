// 5 factorial
int fred() {
  int x;
  int y;
  x = 5;
  y = x - 1;

  while (y > 0) {
    x = x * y;
    y = y - 1;
  }

  return x;
}

void nice() {
  print 69;
}

void main() {
  print fred();
  factorial 5;
  nice();
}
