int main() {
  int i;
  i = 0;
  while (i < 10) {
    if (i == 5) {
      break;
    }

    printint(i);
    i = i + 1;
  } else {
    printint(1337);
  }

  printint(69);
  printint(i);

  for (i = 0; i < 10; i = i + 1) {
    if (i == 5) {
      break;
    }
    printint(i);
  } else {
    printint(420);
  }

  printint(69);
  printint(i);
}
