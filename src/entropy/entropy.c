#include <immintrin.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "re4/entropy.h"

int re4_hw_rdrand(uint64_t *out) {
#if defined(__x86_64__) || defined(_M_X64)
    unsigned long long tmp = 0ULL;     // тип, якого вимагає _rdrand64_step
    int ok = _rdrand64_step(&tmp);     // потребує флагу компілятора -mrdrnd
    if (ok) { *out = (uint64_t)tmp; return 1; }
    return 0;
#else
    (void)out; return 0;
#endif
}

int re4_hw_rdseed(uint64_t *out) {
#if defined(__x86_64__) || defined(_M_X64)
    unsigned long long tmp = 0ULL;
    int ok = _rdseed64_step(&tmp);     // потребує флагу компілятора -mrdseed
    if (ok) { *out = (uint64_t)tmp; return 1; }
    return 0;
#else
    (void)out; return 0;
#endif
}

static size_t minz(size_t a, size_t b) { return a < b ? a : b; }

int re4_sys_entropy(uint8_t *dst, size_t n) {
    size_t off = 0;
    uint64_t x;

    // 1) RDSEED блоками по 8 байт
    while (off + 8 <= n) {
        if (!re4_hw_rdseed(&x)) break;
        memcpy(dst + off, &x, 8);
        off += 8;
    }
    // 2) RDRAND блоками по 8 байт (якщо ще лишилось)
    while (off + 8 <= n) {
        if (!re4_hw_rdrand(&x)) break;
        memcpy(dst + off, &x, 8);
        off += 8;
    }
    // 3) Дозаповнити /dev/urandom, якщо треба
    if (off < n) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) return -1;
        while (off < n) {
            ssize_t r = read(fd, dst + off, n - off);
            if (r <= 0) { close(fd); return -1; }
            off += (size_t)r;
        }
        close(fd);
    }
    return 0;
}
