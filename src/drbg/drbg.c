#include "re4/drbg.h"
#include "../health/health.h"
#include "../sp800_90b/estimator.h"
#include "re4/entropy.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  uint64_t s[4]; /* xoshiro256** state */
  uint64_t counter;
} re4_core_t;

struct re4_ctx {
  re4_core_t core;
  re4_health_t health;
  re4_status_t st;
  char tag[32];

  uint64_t reseed_bytes_limit;  /* policy */
  uint64_t reseed_time_limit_s; /* policy */
  time_t last_reseed;

  uint64_t bytes_since_reseed;
};

/*--- helpers ---------------------------------------------------------------*/
static inline uint64_t rotl(const uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}
static uint64_t xoshiro256ss(re4_core_t *c) {
  uint64_t *s = c->s;
  const uint64_t result = rotl(s[1] * 5, 7) * 9;
  const uint64_t t = s[1] << 17;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = rotl(s[3], 45);
  c->counter++;
  return result;
}
static void mix_seed(re4_core_t *c, const void *buf, size_t n) {
  /* absorb with simple xor+splitmix for demo (replace with KDF/hash later) */
  const uint8_t *p = (const uint8_t *)buf;
  uint64_t acc = 0x9e3779b97f4a7c15ull;
  for (size_t i = 0; i < n; ++i) {
    acc ^= ((uint64_t)p[i]) << ((i & 7) * 8);
    acc *= 0xbf58476d1ce4e5b9ull;
    acc ^= acc >> 32;
  }
  c->s[0] ^= acc;
  c->s[1] ^= acc ^ 0x94d049bb133111ebull;
  c->s[2] ^= acc + 0x2545F4914F6CDD1Dull;
  c->s[3] ^= acc ^ 0x1a2b3c4d5e6f7788ull;
  (void)xoshiro256ss(c);
  (void)xoshiro256ss(c); /* diffuse */
}

static uint64_t env_u64(const char *k, uint64_t def) {
  const char *v = getenv(k);
  if (!v || !*v) return def;
  char *end = NULL;
  unsigned long long x = strtoull(v, &end, 10);
  if (end == v) return def;
  return (uint64_t)x;
}

static void policy_init(struct re4_ctx *ctx) {
  ctx->reseed_bytes_limit = env_u64("RE4_RESEED_BYTES", 32ull << 20); /* 32 MiB */
  ctx->reseed_time_limit_s = env_u64("RE4_RESEED_SECS", 600);         /* 10 min */

  uint32_t rct = (uint32_t)env_u64("RE4_HEALTH_RCT_MAXRUN", 33);
  uint32_t apw = (uint32_t)env_u64("RE4_HEALTH_AP_WIN", 1024);
  uint32_t apl = (uint32_t)env_u64("RE4_HEALTH_AP_LOW", 317);
  uint32_t aph = (uint32_t)env_u64("RE4_HEALTH_AP_HIGH", 707);
  re4_health_init(&ctx->health, rct, apw, apl, aph);
}

/*--- public API ------------------------------------------------------------*/
re4_ctx *re4_create(void) {
  re4_ctx *ctx = (re4_ctx *)calloc(1, sizeof(*ctx));
  return ctx;
}
void re4_free(re4_ctx *ctx) {
  if (ctx) {
    memset(ctx, 0, sizeof(*ctx));
    free(ctx);
  }
}

int re4_init(re4_ctx *ctx, const char *tag) {
  if (!ctx) return -1;
  policy_init(ctx);

  unsigned char seed[64];
  size_t got = re4_sys_entropy(seed, sizeof(seed));
  if (got != sizeof(seed)) return -2;

  /* optional: blend RDRAND */
  uint64_t rx;
  if (re4_hw_rdrand(&rx)) memcpy(seed, &rx, sizeof(rx));

  memset(&ctx->core, 0, sizeof(ctx->core));
  mix_seed(&ctx->core, seed, sizeof(seed));
  memset(seed, 0, sizeof(seed));

  if (tag) {
    size_t L = strlen(tag);
    if (L > sizeof(ctx->tag) - 1) L = sizeof(ctx->tag) - 1;
    memcpy(ctx->tag, tag, L);
    ctx->tag[L] = 0;
  }

  /* SP800-90B MCV on a fresh sample */
  unsigned char samp[65536];
  re4_sys_entropy(samp, sizeof(samp));
  ctx->st.est_min_entropy_bits_per_byte = re4_90b_mcv_min_entropy(samp, sizeof(samp));
  memset(samp, 0, sizeof(samp));

  ctx->st.healthy = 1;
  ctx->st.generated_total = 0;
  ctx->st.reseed_count = 1;
  ctx->last_reseed = time(NULL);
  ctx->bytes_since_reseed = 0;
  return 0;
}

int re4_add_entropy(re4_ctx *ctx, const void *buf, size_t n) {
  if (!ctx || !buf || !n) return -1;
  mix_seed(&ctx->core, buf, n);
  return 0;
}

static int need_reseed(re4_ctx *ctx) {
  time_t now = time(NULL);
  if (ctx->bytes_since_reseed >= ctx->reseed_bytes_limit) return 1;
  if ((uint64_t)(now - ctx->last_reseed) >= ctx->reseed_time_limit_s) return 1;
  return 0;
}

int re4_reseed(re4_ctx *ctx) {
  if (!ctx) return -1;
  unsigned char seed[64];
  size_t got = re4_sys_entropy(seed, sizeof(seed));
  if (got != sizeof(seed)) return -2;
  mix_seed(&ctx->core, seed, sizeof(seed));
  memset(seed, 0, sizeof(seed));

  unsigned char samp[65536];
  re4_sys_entropy(samp, sizeof(samp));
  ctx->st.est_min_entropy_bits_per_byte = re4_90b_mcv_min_entropy(samp, sizeof(samp));
  memset(samp, 0, sizeof(samp));

  ctx->bytes_since_reseed = 0;
  ctx->last_reseed = time(NULL);
  ctx->st.reseed_count++;
  return 0;
}

int re4_random(re4_ctx *ctx, void *out, size_t n) {
  if (!ctx || !out) return -1;
  if (!ctx->st.healthy) return -2;

  if (need_reseed(ctx)) {
    if (re4_reseed(ctx) != 0) return -3;
  }

  uint8_t *p = (uint8_t *)out;
  for (size_t i = 0; i < n; ++i) {
    uint64_t w = xoshiro256ss(&ctx->core);
    p[i] = (uint8_t)(w & 0xFF);
  }

  /* health feed on produced bytes */
  if (!re4_health_feed(&ctx->health, p, n)) {
    ctx->st.healthy = 0; /* gate output; caller may decide to reseed & retry */
    return -4;
  }

  ctx->st.generated_total += n;
  ctx->bytes_since_reseed += n;
  return 0;
}

re4_status_t re4_status(const re4_ctx *ctx) {
  return ctx ? ctx->st : (re4_status_t){0};
}
