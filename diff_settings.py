import os


def add_custom_arguments(parser):
    parser.add_argument("--version", choices=("us", "jp"), default=os.environ.get("VERSION", "us"))


def apply(config, args):
    version = args.version
    if version not in ("us", "jp"):
        raise ValueError("VERSION must be us or jp")
    suffix = ".jp" if version == "jp" else ""
    build_dir = "build/jp" if version == "jp" else "build"
    config["baseimg"] = f"snowboardkids{suffix}.z64"
    config["myimg"] = f"{build_dir}/snowboardkids.z64"
    config["mapfile"] = f"{build_dir}/snowboardkids.map"
    config["makeflags"] = [f"VERSION={version}"]
    config["source_directories"] = ["."]
