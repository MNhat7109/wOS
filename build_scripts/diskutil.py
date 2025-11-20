import argparse as ap
import subprocess as sproc
import sys, os, json

part_fs = [
    "fat16",
    "fat32"
]

class ProgramError(Exception):
    def __init__(self, message):
        super().__init__(message)

class LoopbackManager:
    STATE_FILE = "/tmp/mytool-loop.json"
    def __init__(self, image=None):
        self.image = image
        self.loop = None

    def get_loop(self):
        return self.loop

    def save_loop(self, image, loop):
        self.loop = loop
        self.image = image
        data = {"loop": loop, "image": image}

        with open(self.STATE_FILE, "w") as f:
            json.dump(data, f)

    def load_loop(self):
        if not os.path.exists(self.STATE_FILE):
            return False
        
        with open(self.STATE_FILE, "r") as f:
            data = json.load(f)

        self.loop = data["loop"]
        self.image = data["image"]

        if sproc.run(["losetup", self.loop], check=False).returncode != 0:
            if sproc.run(["losetup", "-P", self.loop, self.image], check=False).returncode != 0: 
                self.setup_loop(self.image)
            
        return True

    def setup_loop(self, image):
        if self.image is None: self.image = image
        self.loop = sproc.check_output(
            ["losetup", "-fP", "--show", self.image], text=True).strip()
        self.save_loop(self.image, self.loop)
    
    def detach_loop(self):
        sproc.run(["losetup", "-d", self.loop], check=True)

    def reset_temp(self):
        os.remove(self.STATE_FILE)

def ensure_root():
    if os.geteuid() == 0:
        return

    print("This command requires root, retrying with sudo...")
    cmd = ["sudo", sys.executable] + sys.argv
    os.execvp("sudo", cmd)

def cmd_reset_temp(args):
    args.loop_handler.reset_temp()

def cmd_mount_part(args):
    os.makedirs(args.mountpoint, exist_ok=True)

    args.loop_handler.setup_loop(args.image)
    loopname=args.loop_handler.get_loop()
    sproc.run(["mount", f"{loopname}p{args.part}", args.mountpoint], check=True)
    args.loop_handler.save_loop(args.image, loopname)

def cmd_unmount_part(args):
    loopstat = args.loop_handler.load_loop()
    if not loopstat: 
        raise ProgramError("Error: Loop does not exist")

    sproc.run(["umount", args.mountpoint], check=True)
    args.loop_handler.detach_loop()

    if args.auto_delete:
        pass

def cmd_create_part(args):
    loopstat = args.loop_handler.load_loop()
    if not loopstat:
        args.loop_handler.setup_loop(args.image)

    sproc.run(["parted", "-s", args.image, "mkpart", args.mode, 
               args.start, args.end if args.end is not None else "100%"]
              , check=True)
    
    args.loop_handler.detach_loop()
    if args.auto_unmount: return
    args.loop_handler.setup_loop(args.image)

def cmd_format_part(args):
    loopstat = args.loop_handler.load_loop()
    if not loopstat:
        args.loop_handler.setup_loop(args.image)

    cmdlist=[]
    match args.fs:
        case "fat32":
            cmdlist += ["mkfs.fat", "-F32"]
        case "fat16":
            cmdlist += ["mkfs.fat", "-F16"]
        case _:
            raise ProgramError(f"Error: Unrecognized fs: {args.fs}")

    cmdlist.append(f"-R{args.reserved_sectors if args.reserved_sectors else 8}")
    cmdlist.append(f"-n{args.label if args.label != '' else ''}")
    cmdlist.append(f"{args.loop_handler.get_loop()}p{args.part}")

    sproc.run(cmdlist, check=True)
    
    if args.auto_unmount: args.loop_handler.detach_loop()

def get_part_table(loop):
    result = sproc.run(
        ["parted", "-s", "-m", loop, "unit B", "print"],
        capture_output=True, text=True
    )
    for line in result.stdout.splitlines():
        if line and not line.startswith("BYT"):
            fields = line.split(":")
            # field 5 (0-indexed 5) is partition table type
            return fields[5]
    return None

def get_part_info(loop, part_num):
    out = sproc.check_output(
        ["parted", "-s", "-m", loop, "unit B", "print"], text=True
        ).splitlines()
    
    for line in out:
        if line.startswith(f"{part_num}"):
            fields = line.strip().split(":")
            return {
                "number": fields[0],
                "start": fields[1],
                "end": fields[2],
                "size": fields[3],
                "fs": fields[4] or None,
                "name": fields[5] or None,
                "flags": fields[6].split(",") if fields[6] else []
            }
    
    raise ProgramError(f"Error: Partition {part_num} not found")

def cmd_set_boot_part(args):
    # Get loop information
    loopstat = args.loop_handler.load_loop()
    if not loopstat:
        args.loop_handler.setup_loop(args.image)
    
    loopname = args.loop_handler.get_loop()
    info = get_part_info(loopname, args.part)
    sproc.run(["parted", "-s", args.image, "set", args.part, "esp" if get_part_table(loopname) == "gpt" else "boot", "on"])

    args.loop_handler.detach_loop()
    
    # Copy the boot record to the desired partition.
    # This requires the partition to be FAT, or else it'll break.
    if info['fs'] not in ["fat16", "fat32"]:
        raise ProgramError("Error: setboot requires partition to be FAT16 or FAT32")

    # Now to copy to the right place, we'll add the partition's start offset to 62 (in bytes), as this is
    # where the boot code starts
    size_bytes = int(info['start'].replace("B", ""))+62
    sproc.run(["dd", f"if={args.boot_file}", f"of={args.image}", "bs=1","skip=62", f"seek={size_bytes}", "conv=notrunc"], check=True)


def cmd_convert(args):  
    #sproc.run(["dd", "if=/dev/zero", f"of={args.image}", "bs=512", "conv=notrunc"], check=True)
    match args.mode:
        case "mbr":
            sproc.run(["dd", f"if={args.boot_file}", f"of={args.image}", "conv=notrunc"], check=True)
        case "gpt":
            print("Not implemented. Sorry!")
        case _:
            raise ProgramError(f"Error: Unknown mode '{args.mode}'")

def build_parser():
    parser = ap.ArgumentParser(
        prog="diskutil",
    )

    sub = parser.add_subparsers(dest="command", required=True)

    parse_create = sub.add_parser("create-part", )
    parse_create.add_argument("image")
    parse_create.add_argument("--mode", required=True, choices=["primary", "extended"])
    parse_create.add_argument("--start", required=True)
    parse_create.add_argument("--end")
    parse_create.add_argument("--auto-unmount", action="store_true")
    parse_create.set_defaults(func=cmd_create_part, loop_handler=LoopbackManager())

    parse_format = sub.add_parser("format", )
    parse_format.add_argument("image")
    parse_format.add_argument("--part", required=True)
    parse_format.add_argument("--fs", required=True, choices=part_fs)
    parse_format.add_argument("--reserved-sectors")
    parse_format.add_argument("--label")
    parse_format.add_argument("--auto-unmount", action="store_true")
    parse_format.set_defaults(func=cmd_format_part, loop_handler=LoopbackManager())

    parse_mount = sub.add_parser("mount", )
    parse_mount.add_argument("image")
    parse_mount.add_argument("mountpoint")
    parse_mount.add_argument("--part", required=True)
    parse_mount.set_defaults(func=cmd_mount_part, loop_handler=LoopbackManager())

    parse_umount = sub.add_parser("unmount", )
    parse_umount.add_argument("image")
    parse_umount.add_argument("mountpoint")
    parse_umount.add_argument("--auto-delete", action="store_true")
    parse_umount.set_defaults(func=cmd_unmount_part, loop_handler=LoopbackManager())

    parse_setboot = sub.add_parser("setboot", )
    parse_setboot.add_argument("image")
    parse_setboot.add_argument("--part", required=True)
    parse_setboot.add_argument("--boot-file")
    parse_setboot.set_defaults(func=cmd_set_boot_part, loop_handler=LoopbackManager())

    parse_convert = sub.add_parser("convert")
    parse_convert.add_argument("image")
    parse_convert.add_argument("mode", choices=["mbr", "gpt"])
    parse_convert.add_argument("boot_file")
    parse_convert.set_defaults(func=cmd_convert, loop_handler=None)

    parse_reset = sub.add_parser("reset")
    parse_reset.set_defaults(func=cmd_reset_temp, loop_handler=LoopbackManager())

    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()
    ensure_root()

    try:
        args.func(args)
    except Exception as ex:
        print(f"Error: diskutil failed with exception: {ex}")
        if args.loop_handler: args.loop_handler.detach_loop()
        sys.exit(1)
        

if __name__ == "__main__":
    main()