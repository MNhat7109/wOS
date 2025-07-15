# wOS

Another *hobby x86 Operating System* you see on the 'Net.

## About the Name

So - why "wOS", but not "lOS", eh? (Bad joke, nevermind)

To be honest, I don't know. It's just a placeholder name that I whipped out of my head back when I was in grade 10.

You guys can interpret it whatever comes to your mind - maybe "W" here stands for "Win" or for a Twitch slang (W chat kinda thing). Or you guys can even do a mood-based interpretation for this. The OS works seamlessly, then it'll be "Wow!"-OS, or if it's garbage or doesn't work with your devices, you can say it as "Why?"-OS.

But, if my naming here is so arse for some reason, or if you guys want a better name, don't worry. This name is a placeholder, so it'll be changed after I go touch some grass for inspiration.

## Status

**Current Version:** *0.1-canary-1*

- ✔ VGA Text Mode
- ✔ VESA Linear Mode
- ✔ Paging
- ✔ MMU and Heap
- ✔ ACPI Device Discovery
- ✔ Timer
- ✔ PCI Device Discovery
- ✔ AHCI
- ❌ UART

## Features

This OS:

- Boots on QEMU
- Supports VESA video mode up to 720p HD
- Scans PCI devices with both PIO and MMIO
- Sleeps in intervals
- Supports Paging
- Supports kmalloc/kfree
- Lays down the groundwork for multitasking
- Loads files on SATA disks (AHCI only)

## How To Run

### Prerequisites:

- A laptop that runs Linux.
- Latest build of [Git](https://git-scm.com/) (to fork this repo, of course).
- Latest build of [QEMU](https://www.qemu.org/) (for emulation).
- Latest build of [GMP](https://gmplib.org/), [MPFR](https://www.mpfr.org/) and [```texinfo```](https://www.gnu.org/software/texinfo/) (for toolchain building).
- Latest build of Linux dev tools (e.g. [```build-essential```](https://wiki.debian.org/BuildEssential) for Debian-based, or [```base-devel```](https://wiki.archlinux.org/title/DeveloperWiki:Building_in_a_Clean_Chroot#base-devel_group) for Arch-based distros).

> 💡 **Note:**  
> If your laptop runs something other than Linux — don’t worry!  
> - **Windows:** Install [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/en-us/windows/wsl/) or MSYS2 — they give you a Linux-like dev environment.  
> - **macOS:** Install [Xcode Command Line Tools](https://developer.apple.com/xcode/) and [Homebrew](https://brew.sh/) for extra tools (`gcc`, `make`, `qemu`).  

### Run

**1. Install all of the needed packages mentioned above:**

- If you're using Debian-based distros (e.g. Ubuntu, Debian, etc.), or MSYS2:

```
sudo apt update
sudo apt install build-essential qemu gmp mpfr libmpfr-dev texinfo git
```

- For Arch-based distros (e.g. Arch Linux, Manjaro):

```
sudo pacman -Syu
sudo pacman -S base-devel qemu gmp mpfr texinfo git
```

- For Macs:

```
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew if not installed yet
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Then install extra tools
brew install gcc make qemu gmp mpfr texinfo git
```

**2. Fork and clone this GitHub repository:**

```
git clone https://github.com/MNhat7109/wOS.git
```

**3. Run the initial setup:**

```
make setup
make run -C build_scripts/
```
> **Note:** 
> Remember to wait for the commands to complete!

**4. Whenever you want to test the OS out, run:**

```
make
```

Want to see how you can contribute? [Click here](#contributing).

## Development Stage

**Status:** *Early Canary build*

wOS has 5 stages of development: **Canary**, **Developer's Beta**, **Public's Beta**, **Sneak Peek** and **Release**. All of which will be discussed below.

### Canary

There will be 4–5 *Canary* stages. The first 4 cover essential features for the scheduled Release (v1.0), while the last sets groundwork for future versions (e.g., v2.0, 3.0). The goal is to test experimental hardware and features fast, with quick feedback — even if it’s unstable.

**WARNING:** All *Canary* builds contain *cutting-edge features* so sharp they might *chop your PC into smithereens*. Therefore, if you want to test the latest features, using a **Virtual Machine** is strongly recommended. If you still choose to run them on your real PC, do so at your own risk, you stubborn little pookie bear.

**Status:** *Canary 1*

***End-of-stage goals:***

- [ ] Can read/write from many types of disks (e.g. SATA, PATA, ATAPI, USB, NVMe)
- [ ] Can run on bare-metal
- [ ] Can pre-emptively multitask
- [ ] Can run other programs
- [ ] Can run a shell

### Developer's Beta

There will be at most 3 Dev stages. The first one will be the most stable Canary build with app support and features that are stable enough for the developers to contribute. The last two may appear as patches or stabilization updates for the first Developer’s Beta. But even so, expect instabilities and lack of security that are present in these builds.

**Status:** *Coming Soon!*

### Public's Beta

There will be two *Public's Beta* stages. One will be the most stable *Developer's Beta* build, with refined edges that make it friendly to the early adopters. The other one will be built based on the user's feedback to further improve the user experience. There may be rough edges here and there, but they are still more stable than the *Developer's*. 

**Status:** *Coming Soon!*

### Sneak Peek

If you are a user who wants the early immersion of the new features, but are worried about stability and security, then this build is for you. *Sneak Peek* unveils all the latest, stable features from the *Public's Beta* and refined them to perfection, all for the users, by the users.

**Status:** *Coming Soon!*

### Release

For any ordinary users or newcomers who are getting familiar with the OS. It has most of the latest, stable features from *Sneak Peek*, with maximized stability.

**Status:** *Coming Soon!*

## License

This project is licensed under the [MIT License](./LICENSE).

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md) if you want to help, laugh, or find my typos.