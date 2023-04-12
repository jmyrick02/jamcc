void main() {
  int i;
  int j;

  for (i = 0; i < 10; i = i + 1) {
    if (i > 5) {
      break;
    }
    for (j = 0; j < 10; j = j + 1) {
      if (j > 5) {
        break;
      }
      printint(i + j);
    }
  }
}
