#!/usr/bin/env bash
set -euo pipefail

input="$1"
output="$2"
read -r digest _ < <(sha256sum "$input")
{
    printf '#ifndef MYOS_KERNEL_DIGEST_H\n#define MYOS_KERNEL_DIGEST_H\n\n'
    printf 'static const unsigned char myos_kernel_sha256[32] = {'
    for ((index = 0; index < 64; index += 2)); do
        printf '0x%s' "${digest:index:2}"
        if (( index < 62 )); then printf ', '; fi
    done
    printf '};\n\n#endif\n'
} > "$output"
