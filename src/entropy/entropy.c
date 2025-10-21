#include "re4/entropy.h"
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

// ---------- System entropy ----------
size_t re4_sys_entropy(void *buf, size_t n) {
#if defined(_WIN32)
  // CNG: BCryptGenRandom
  if (!buf || n == 0) return 0;
  NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
  return st == 0 ? n : 0;
#else
  if (!buf || n == 0) return 0;
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) return 0;
  size_t off = 0;
  while (off < n) {
    ssize_t r = read(fd, (uint8_t *)buf + off, n - off);
    if (r <= 0) {
      if (errno == EINTR) continue;
      break;
    }
    off += (size_t)r;
  }
  close(fd);
  return off;
#endif
}

// ---------- HW intrinsics (x86-64) ----------
#if (defined(__x86_64__) || defined(_M_X64))
// GCC/Clang/MSVC: immintrin provides rdrand/rdseed intrinsics
#if defined(__has_include)
#if __has_include(<immintrin.h>)
#include <immintrin.h>
#endif
#else
#include <immintrin.h>
#endif
#endif

int re4_hw_rdrand(uint64_t *out) {
#if (defined(__x86_64__) || defined(_M_X64))
  unsigned long long tmp = 0ULL;
// Some compilers need the CPU feature flag at compile-time; CI adds it where supported.
#if defined(__RDRND__) || defined(_MSC_VER)
  int ok = _rdrand64_step(&tmp);
  if (ok) {
    *out = (uint64_t)tmp;
    return 1;
  }
  return 0;
#else
  (void)tmp;
  (void)out;
  return 0;
#endif
#else
  (void)out;
  return 0;
#endif
}

int re4_hw_rdseed(uint64_t *out) {
#if (defined(__x86_64__) || defined(_M_X64))
  unsigned long long tmp = 0ULL;
#if defined(__RDSEED__) || defined(_MSC_VER)
  int ok = _rdseed64_step(&tmp);
  if (ok) {
    *out = (uint64_t)tmp;
    return 1;
  }
  return 0;
#else
  (void)tmp;
  (void)out;
  return 0;
#endif
#else
  (void)out;
  return 0;
#endif
}
