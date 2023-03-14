{
  int i;
  for (i = 0; i < 10; i = i + 1) {
    if (i == 5) {
      break;
    }
    print i;
  }

  for (i = 0; i < 10; i = i + 1) {
    if (i == 5) {
      continue;
    }
    print i;
  }
}
