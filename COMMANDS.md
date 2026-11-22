# OmniQ-OS System Commands & Tool Reference

This document tracks all discovered binaries, system tool paths, and command invocations to avoid redundant discovery commands and save credits.

## Discovered System Binaries (macOS Host)
- **VirtualBox CLI**: `/usr/local/bin/VBoxManage` (Present)
- **ISO Tools**:
  - `xorriso`: `/opt/homebrew/bin/xorriso` (v1.5.8.pl02)
  - `mksquashfs` / `unsquashfs`: `/opt/homebrew/bin/mksquashfs`, `/opt/homebrew/bin/unsquashfs`
- **Compression**:
  - `zstd`: `/opt/homebrew/bin/zstd`
  - `gzip`: `/usr/bin/gzip`
- **Archive & Linker**:
  - `cpio`: `/usr/bin/cpio` (standard BSD/GNU cpio on macOS)
  - `python3`: `/usr/bin/python3` or Homebrew Python 3.12
- **Image & OCR**:
  - `tesseract`: `/opt/homebrew/bin/tesseract`
  - `sips`: `/usr/bin/sips`
- **Compilers & Build**:
  - `cmake`: `/opt/homebrew/bin/cmake`
  - `clang` / `clang++`: `/usr/bin/clang++`

## VM & Build Artifact Paths
- **VM Name**: `OmniQ-OS`
  - UUID: `9d3fb4d1-2af7-48f8-bda8-f7c036856972`
  - Storage Controller: `USB_Storage` (Port 0)
- **Distro Path**: `/Users/gouthamarcot/Documents/personal/quantum/omniqos-linux`
- **Source Files**:
  - `SOURCE_ISO`: `/Users/gouthamarcot/Documents/personal/quantum/omniqos-linux/OmniQ-OS-24.04-RT-arm64.iso`
  - `SOURCE_INITRD`: `/tmp/casper/initrd`
  - `SOURCE_SQUASHFS`: `/tmp/casper/minimal.squashfs`
- **Build Roots**:
  - `BUILD_ROOT`: `/tmp/omniq_pure_distro_build`
  - `STAGING_ROOTFS`: `/tmp/omniq_pure_distro_build/staging_rootfs`
  - `INITRD_ROOT`: `/tmp/omniq_pure_distro_build/initrd_root`
  - `ISO_ROOT`: `/tmp/omniq_pure_distro_build/iso_root`
  - `OUTPUT_ISO`: `/Users/gouthamarcot/Documents/personal/quantum/omniqos-linux/OmniQ-OS-Pure-24.04-RT-arm64.iso`
  - `OUTPUT_SHA256`: `2006107713a8802e6cd3f3caff43bfb1073ef7d36f26ba55e3a7a433c12c2c51` (165.25 MB)
- **Screenshots Directory**: `/Users/gouthamarcot/.gemini/antigravity/brain/a89ace88-0c02-4bdb-b246-b97558019cd9/`

## CPIO Archive Format Rules (Kernel Initramfs)
- Format: SVR4 newc without CRC (`070701`).
- All directories MUST be present as CPIO entries (mode `040755`, filesize `0`) BEFORE any child files.
- Symlinks must have mode `0120777` with target pathname as content and length.
- Alignment: 4-byte boundary for headers, filenames, and data.
- Padding: 512-byte boundary after `TRAILER!!!\0`.
