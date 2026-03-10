/*
 * iohttpparser — Scalar scanner backend (Layer 1)
 * Baseline implementation, always available on all platforms.
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.1-or-later
 */

#include "ihtp_internal.h"

#include <iohttpparser/ihtp_scanner.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* ─── Token character table (RFC 9110 Section 5.6.2) ──────────────────── */

/* clang-format off */
const uint8_t ihtp_token_table[256] = {
    /*   0 nul  1 soh  2 stx  3 etx  4 eot  5 enq  6 ack  7 bel */
            0,     0,     0,     0,     0,     0,     0,     0,
    /*   8 bs   9 ht  10 nl  11 vt  12 np  13 cr  14 so  15 si  */
            0,     0,     0,     0,     0,     0,     0,     0,
    /*  16 dle 17 dc1 18 dc2 19 dc3 20 dc4 21 nak 22 syn 23 etb */
            0,     0,     0,     0,     0,     0,     0,     0,
    /*  24 can 25 em  26 sub 27 esc 28 fs  29 gs  30 rs  31 us  */
            0,     0,     0,     0,     0,     0,     0,     0,
    /*  32 sp  33  !  34  "  35  #  36  $  37  %  38  &  39  '  */
            0,   '!',    0,   '#',  '$',  '%',  '&', '\'',
    /*  40  (  41  )  42  *  43  +  44  ,  45  -  46  .  47  /  */
            0,     0,   '*',  '+',    0,   '-',  '.',    0,
    /*  48  0  49  1  50  2  51  3  52  4  53  5  54  6  55  7  */
           '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
    /*  56  8  57  9  58  :  59  ;  60  <  61  =  62  >  63  ?  */
           '8',  '9',    0,     0,    0,     0,    0,     0,
    /*  64  @  65  A  66  B  67  C  68  D  69  E  70  F  71  G  */
            0,   'A',  'B',  'C',  'D',  'E',  'F',  'G',
    /*  72  H  73  I  74  J  75  K  76  L  77  M  78  N  79  O  */
           'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
    /*  80  P  81  Q  82  R  83  S  84  T  85  U  86  V  87  W  */
           'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',
    /*  88  X  89  Y  90  Z  91  [  92  \  93  ]  94  ^  95  _  */
           'X',  'Y',  'Z',    0,     0,    0,  '^',  '_',
    /*  96  `  97  a  98  b  99  c 100  d 101  e 102  f 103  g  */
           '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
    /* 104  h 105  i 106  j 107  k 108  l 109  m 110  n 111  o  */
           'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
    /* 112  p 113  q 114  r 115  s 116  t 117  u 118  v 119  w  */
           'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
    /* 120  x 121  y 122  z 123  { 124  | 125  } 126  ~ 127 del */
           'x',  'y',  'z',    0,   '|',    0,  '~',    0,
    /* 128-255: high bytes — not valid in tokens */
};
/* clang-format on */

/* ─── Scanner runtime dispatch ────────────────────────────────────────── */

static ihtp_scanner_vtable_t scanner_vtable;
static bool scanner_initialized = false;

void ihtp_scanner_select_vtable(ihtp_scanner_vtable_t *vtable, int simd_level)
{
    /* Start with scalar baseline */
    vtable->find_char = ihtp_scan_find_char_scalar;
    vtable->is_token = ihtp_scan_is_token_scalar;

#ifdef IOHTTPPARSER_HAVE_AVX2
    if ((simd_level & 0x02) != 0) {
        vtable->find_char = ihtp_scan_find_char_avx2;
        vtable->is_token = ihtp_scan_is_token_avx2;
        return;
    }
#endif

#ifdef IOHTTPPARSER_HAVE_SSE42
    if ((simd_level & 0x01) != 0) {
        vtable->find_char = ihtp_scan_find_char_sse42;
        vtable->is_token = ihtp_scan_is_token_sse42;
    }
#endif
}

static void ihtp_scanner_init(void)
{
    if (scanner_initialized) {
        return;
    }

    ihtp_scanner_select_vtable(&scanner_vtable, ihtp_scanner_simd_level());

    scanner_initialized = true;
}

const ihtp_scanner_vtable_t *ihtp_scanner_get(void)
{
    if (!scanner_initialized) {
        ihtp_scanner_init();
    }
    return &scanner_vtable;
}

/* ─── Public scanner API (delegates to active backend) ────────────────── */

const char *ihtp_scan_find_char(const char *buf, size_t len, const char *delims)
{
    return ihtp_scanner_get()->find_char(buf, len, delims);
}

bool ihtp_scan_is_token(const char *buf, size_t len)
{
    return ihtp_scanner_get()->is_token(buf, len);
}

size_t ihtp_scan_skip_lws(const char *buf, size_t len)
{
    size_t i = 0;
    while (i < len && ihtp_is_lws((uint8_t)buf[i])) {
        i++;
    }
    return i;
}

int ihtp_scanner_simd_level(void)
{
    int level = 0;
#ifdef IOHTTPPARSER_HAVE_SSE42
    if (__builtin_cpu_supports("sse4.2")) {
        level |= 0x01;
    }
#endif
#ifdef IOHTTPPARSER_HAVE_AVX2
    if (__builtin_cpu_supports("avx2")) {
        level |= 0x02;
    }
#endif
    return level;
}

/* ─── Scalar implementations ──────────────────────────────────────────── */

const char *ihtp_scan_find_char_scalar(const char *buf, size_t len, const char *delims)
{
    /* Build a 256-bit lookup table for delimiter set */
    uint8_t table[256] = {0};
    for (const char *d = delims; *d != '\0'; d++) {
        table[(uint8_t)*d] = 1;
    }

    for (size_t i = 0; i < len; i++) {
        if (table[(uint8_t)buf[i]]) {
            return buf + i;
        }
    }
    return buf + len;
}

bool ihtp_scan_is_token_scalar(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (!ihtp_is_token_char((uint8_t)buf[i])) {
            return false;
        }
    }
    return true;
}
