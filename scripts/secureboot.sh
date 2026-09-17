#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
KEY_DIR="$ROOT_DIR/keys"
BUILD_DIR="$ROOT_DIR/build"
KEY_BITS=3072
VALID_DAYS=3650

fail() {
    printf 'secureboot: %s\n' "$1" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || fail "required command not found: $1"
}

make_certificate() {
    local name="$1"
    local subject="$2"
    openssl req -new -x509 -newkey "rsa:${KEY_BITS}" \
        -keyout "$KEY_DIR/${name}.key" \
        -out "$KEY_DIR/${name}.crt" \
        -days "$VALID_DAYS" -nodes -sha256 \
        -subj "$subject" \
        -addext "basicConstraints=critical,CA:TRUE" \
        -addext "keyUsage=critical,keyCertSign,digitalSignature"
    chmod 600 "$KEY_DIR/${name}.key"
}

init_keys() {
    require_command openssl
    mkdir -p "$KEY_DIR"
    [[ ! -e "$KEY_DIR/PK.key" ]] || fail "keys already exist; remove them manually before regeneration"
    make_certificate PK "/CN=MyOS Platform Key/"
    make_certificate KEK "/CN=MyOS Key Exchange Key/"
    make_certificate db "/CN=MyOS Allowed Image Signing Key/"
    cp "$KEY_DIR/db.crt" "$KEY_DIR/db.pem"
    chmod 600 "$KEY_DIR/db.pem"
    printf 'secureboot: development PK, KEK, and db keys created in %s\n' "$KEY_DIR"
    printf 'secureboot: private keys are ignored by Git; back them up securely.\n'
}

sign_image() {
    require_command sbsign
    [[ -f "$KEY_DIR/db.key" && -f "$KEY_DIR/db.crt" ]] || fail "run '$0 init' first"
    [[ -f "$BUILD_DIR/BOOTX64.EFI" ]] || fail "run 'make' first"
    sbsign --key "$KEY_DIR/db.key" --cert "$KEY_DIR/db.crt" \
        --output "$BUILD_DIR/BOOTX64.SIGNED.EFI" "$BUILD_DIR/BOOTX64.EFI"
    printf 'secureboot: signed image: %s\n' "$BUILD_DIR/BOOTX64.SIGNED.EFI"
}

verify_image() {
    require_command sbverify
    [[ -f "$BUILD_DIR/BOOTX64.SIGNED.EFI" ]] || fail "run '$0 sign' first"
    sbverify --cert "$KEY_DIR/db.crt" "$BUILD_DIR/BOOTX64.SIGNED.EFI"
}

usage() {
    printf 'usage: %s {init|sign|verify}\n' "$0"
}

case "${1:-}" in
    init) init_keys ;;
    sign) sign_image ;;
    verify) verify_image ;;
    *) usage; exit 2 ;;
esac
