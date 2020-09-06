/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#include "stupidlayers.c"

static int handler(struct input_event* ev) {
  if (ev->code == KEY_CAPSLOCK) ev->code = KEY_ESC;
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 1) {
    fprintf(stderr, "usage: %s /dev/input/eventX\n"
      "you can use use evtest to list devices\n", argv[0]);
    return 1;
  }
  return stupidlayers_simple(argv[1], handler);
}
