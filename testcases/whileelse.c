int main() {
  int i;
  i = 0;
  while (i < 10) {
    if (i == 5) {
      break;
    }

    print i;
    i = i + 1;
  } else {
    print 1337;
  }

  print 69;
  print i;

  for (i = 0; i < 10; i = i + 1) {
    if (i == 5) {
      break;
    }
    print i;
  } else {
    print 420;
  }

  print 69;
  print i;
}
