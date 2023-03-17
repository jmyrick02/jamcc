int f() {
  int a;

  a = a + 1;
  return a;
}

int main() {
  a = 1;
  print a + f(0);

  a = 1;
  print f(0) + a;
}
