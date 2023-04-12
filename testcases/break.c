void main() {
  int i;
  for (i = 0; i < 10; i = i + 1) {
    if (i == 5) {
      break;
    }
    printint(i);
  }

  for (i = 0; i < 10; i = i + 1) {
    if (i == 5) {
      continue;
    }
    printint(i);
  }
}
