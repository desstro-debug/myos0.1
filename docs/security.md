# MyOS security model

MyOS uses UID/GID metadata on VFS entries:

- UID/GID `0/0` is `root`.
- UID/GID `1/1` is `system`.
- UID/GID `1000/1000` is the default user.

Root can read, write, and remove ordinary VFS entries. The `/boot` records are the exception: they carry protected and hidden flags, have mode `0000`, are omitted from listings, and cannot be opened or removed through the VFS API, including by UID 0.

This is an in-OS policy boundary. It cannot hide the repository's physical `boot/` source directory from the host operating system, and it cannot prevent a host root user from reading or replacing the boot sector. Real disk encryption, measured boot, or hardware write protection would be needed for that boundary.

## UEFI Secure Boot preparation

The repository contains [keys/](../keys/) and [scripts/secureboot.sh](../scripts/secureboot.sh). The script creates separate PK, KEK, and db development certificates, keeps private material ignored by Git, signs `BOOTX64.EFI` with `sbsign`, and verifies it with `sbverify`.

OpenSSL alone is used for certificate generation. It is intentionally not treated as an EFI image signer: Authenticode/EFI signatures must be produced by a tool such as `sbsign` and then enrolled into firmware or an isolated OVMF variable store.

The loader also embeds a SHA-256 digest generated from `kernel.elf` during `make`. Before parsing ELF segments, UEFI recomputes the digest and rejects a modified kernel. This creates a measured chain from the signed EFI image to the kernel payload; changing the kernel requires rebuilding and re-signing the EFI loader.

The current container has OpenSSL but not `sbsign`, so `make secureboot-sign` stops with an explicit missing-tool error. No unsigned CMS or fake EFI signature is produced.
