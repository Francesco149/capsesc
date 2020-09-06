remaps caps lock to escape through a user-space keyboard driver. this should make it behave as
escape even in programs that use the keyboard scan codes to handle key presses such as games.

I made this because I got tired of having to press actual ESC in some games and virtual machines
which would then trigger caps lock because of `setxkbmap -option caps:escape` .

based on my [stupidlayers](https://github.com/Francesco149/stupidlayers) project.

# usage

use evtest to figure out which /dev/input/event* device is your keyboard.
for me it's `/dev/input/event3`

```sh
gcc main.c -o capsesc
sudo ./capsesc /dev/input/event3
```

if you run this by hand on the keyboard that gets captured you might
end up with the enter key getting stuck from missing the release event.
just press enter again to stop it

ideally you want a udev rule that automatically runs capsesc when
the keyboard is plugged in, here's an example for void linux (assumes you
copied capsesc to /usr/bin)

note that the rules.d directory might be different in other distros

```sh
sudo tee /usr/lib/udev/rules.d/30-capsesc.rules << "EOF"
ACTION=="add", \
KERNEL=="event[0-9]*", \
ATTRS{name}=="SONiX USB DEVICE", \
RUN+="/bin/sh -c 'echo /usr/bin/capsesc /dev/input/%k | at now'"
EOF

sudo xbps-install at
sudo ln -s /etc/sv/at /var/service
sudo sv start at
sudo udevadm control --reload
sudo udevadm control --reload-rules
```

you can also just make a script that finds the device and run it in
your xinitrc or something, but it won't stick if you unplug the keyboard

example of finding a device by name (what I have in my xinitrc):

```sh
sudo killall capsesc
for x in /dev/input/event*; do
  devname=$(cat $(echo $x | sed 's|dev|sys/class|g')/device/name)
  if [ "$devname" = "SONiX USB DEVICE" ]; then
    sudo capsesc $x &
    break
  fi
done
```

keep in mind that if you put this in .xinitrc you must have no password
required on sudo
