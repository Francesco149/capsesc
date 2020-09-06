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

/* stripped down version of https://github.com/Francesco149/stupidlayers */

#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef struct stupidlayers {
  char* errstr;
  int fd, uinput;
} stupidlayers_t;

typedef int input_handler_t(struct input_event* ev);

static stupidlayers_t* sl_perror(stupidlayers_t* sl, char* syscall, char* errstr) {
  perror(syscall);
  if (sl) { sl->errstr = errstr; }
  return sl;
}

stupidlayers_t* new_stupidlayers(char* device) {
  stupidlayers_t* sl = calloc(sizeof(stupidlayers_t), 1);
  struct uinput_user_dev uidev;
  int i;
  if (!sl) {
    return sl_perror(sl, "calloc", 0);
  }
  /* capture all input from the device */
  sl->fd = open(device, O_RDONLY);
  if (sl->fd < 0) {
    return sl_perror(sl, "open", "failed to open device");
  }
  if (ioctl(sl->fd, EVIOCGRAB, (void*)1) < 0) {
    return sl_perror(sl, "ioctl", "failed to grab device");
  }
  /* create a virtual device that will forward keystrokes to evdev */
  sl->uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (sl->uinput < 0) {
    return sl_perror(sl, "open", "failed to open /dev/uinput");
  }
  #define evbit(x) \
  if (ioctl(sl->uinput, UI_SET_EVBIT, x) < 0) { \
    return sl_perror(sl, "ioctl", "failed to set " #x); \
  }
  evbit(EV_KEY)
  #undef evbit
  for (i = 1; i < 255; ++i) {
    if (ioctl(sl->uinput, UI_SET_KEYBIT, i) < 0) {
      return sl_perror(sl, "ioctl", "failed to set keybit");
    }
  }
  memset(&uidev, 0, sizeof(uidev));
  strcpy(uidev.name, "stupidlayers");
  if (write(sl->uinput, &uidev, sizeof(uidev)) < 0) {
    return sl_perror(sl, "write", "failed to write to uinput");
  }
  if (ioctl(sl->uinput, UI_DEV_CREATE) < 0) {
    return sl_perror(sl, "ioctl", "UI_DEV_CREATE failed");
  }
  return sl;
}

void free_stupidlayers(stupidlayers_t* sl) {
  if (sl->fd >= 0) {
    ioctl(sl->fd, EVIOCGRAB, 0);
    close(sl->fd);
  }
  if (sl->uinput) { close(sl->uinput); }
  free(sl);
}

int stupidlayers_send(stupidlayers_t* sl, struct input_event* ev) {
  if (write(sl->uinput, ev, sizeof(struct input_event)) < 0) {
    sl_perror(sl, "write", "failed to write to uinput");
    return 0;
  }
  return 1;
}

void stupidlayers_run(stupidlayers_t* sl, input_handler_t* handler) {
  struct input_event ev;
  while (1) {
    if (read(sl->fd, &ev, sizeof(ev)) != sizeof(ev)) {
      sl_perror(sl, "read", "failed to read from device");
      return;
    }
    if (ev.type == EV_KEY) {
      /* modify event here if desired */
      if (handler && handler(&ev)) continue;
    }
    /* forward event to the virtual device */
    if (!stupidlayers_send(sl, &ev)) break;
  }
}

#define die(x) { fprintf(stderr, x); return 1; }

int stupidlayers_simple(char* device, input_handler_t* handler) {
  stupidlayers_t* sl;
  sl = new_stupidlayers(device);
  if (!sl) return 1;
  if (sl->errstr) die(sl->errstr);
  stupidlayers_run(sl, handler);
  if (sl->errstr) die(sl->errstr);
  return 0;
}
