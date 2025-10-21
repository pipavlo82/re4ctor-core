// src/drbg/drbg.c
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "re4/drbg.h"
#include "re4/entropy.h"
#include "sp800_90b/estimator.h"

// --------------------- внутрішні межі/параметри ---------------------

#define RE4_RESEED_BYTES_LIMIT (32ULL * 1024ULL * 1024ULL) // кожні ~32 MiB
#define RE4_RESEED_TIME_SECONDS 300                        // або кожні 5 хв
#define RE4_90B_SAMPLE_LEN 65536                           // ~64 KiB для MCV

// --------------------- внутрішній стан DRBG -------------------------

struct re4_core_state {
  // Це «ядро» з src/core/re4ctor_core.c.
  // Ми не знаємо його структури тут, лише передаємо в core-функції.
  uint8_t opaque[256];
};

struct re4_ctx {
  struct re4_core_state core;
  re4_status_t st;

  time_t last_reseed;
  uint64_t bytes_since_reseed;
};

// Ці функції реалізовані у ядрі (src/core/re4ctor_core.c).
// Не маємо заголовка — викликаємо напряму. Якщо у тебе є хедер, підключи його
// і видали ці extern (вони тут лише щоб уникнути implicit-decl warnings).
extern void mix_seed(void *core, const void *seed, size_t seedlen);
extern void re4_stream(void *core, uint8_t *out, size_t n);

// --------------------- допоміжні функції ----------------------------

static int need_reseed(re4_ctx *ctx) {
  if (!ctx) return 0;
  if (ctx->bytes_since_reseed >= RE4_RESEED_BYTES_LIMIT) return 1;
  time_t now = time(NULL);
  if (now - ctx->last_reseed >= RE4_RESEED_TIME_SECONDS) return 1;
  return 0;
}

// Швидка оцінка мін-ентропії за SP800-90B (Most Common Value) для сід-матеріалу.
static void update_90b_estimate(re4_ctx *ctx) {
  if (!ctx) return;

  unsigned char *samp = (unsigned char *)malloc(RE4_90B_SAMPLE_LEN);
  if (!samp) return;

  // NB: re4_sys_entropy повертає int — кількість байтів або 0 при помилці
  int got = re4_sys_entropy(samp, RE4_90B_SAMPLE_LEN);
  if (got > 0) {
    ctx->st.est_min_entropy_bits_per_byte = re4_90b_mcv_min_entropy(samp, (size_t)got);
  }

  memset(samp, 0, RE4_90B_SAMPLE_LEN);
  free(samp);
}

// --------------------- публічний API --------------------------------

re4_ctx *re4_create(void) {
  re4_ctx *ctx = (re4_ctx *)calloc(1, sizeof(*ctx));
  return ctx;
}

void re4_free(re4_ctx *ctx) {
  if (!ctx) return;
  // Нульовимо чутливий стан
  memset(ctx, 0, sizeof(*ctx));
  free(ctx);
}

int re4_init(re4_ctx *ctx, const char *domain_tag) {
  if (!ctx) return -1;
  (void)domain_tag;

  // 1) Початковий сід із системної ентропії (64 байти)
  unsigned char seed[64];
  int got = re4_sys_entropy(seed, sizeof(seed));
  if (got != (int)sizeof(seed)) {
    memset(seed, 0, sizeof(seed));
    return -2;
  }

  mix_seed(&ctx->core, seed, sizeof(seed));
  memset(seed, 0, sizeof(seed));

  // 2) Оцінимо мін-ентропію поточного середовища (90B MCV)
  update_90b_estimate(ctx);

  // 3) Базовий health-статус
  ctx->st.healthy = 1;
  ctx->st.generated_total = 0;
  ctx->st.reseed_count = 1; // первинний сід уже зроблено
  ctx->last_reseed = time(NULL);
  ctx->bytes_since_reseed = 0;
  return 0;
}

int re4_reseed(re4_ctx *ctx) {
  if (!ctx) return -1;

  unsigned char seed[64];
  int got = re4_sys_entropy(seed, sizeof(seed));
  if (got != (int)sizeof(seed)) {
    memset(seed, 0, sizeof(seed));
    return -2;
  }

  mix_seed(&ctx->core, seed, sizeof(seed));
  memset(seed, 0, sizeof(seed));

  update_90b_estimate(ctx);

  ctx->st.reseed_count++;
  ctx->last_reseed = time(NULL);
  ctx->bytes_since_reseed = 0;
  return 0;
}

int re4_random(re4_ctx *ctx, void *out, size_t n) {
  if (!ctx || !out) return -1;
  if (!ctx->st.healthy) return -2;

  if (need_reseed(ctx)) {
    if (re4_reseed(ctx) != 0) return -3;
  }

  // re4_stream заповнює байтами з ядра
  re4_stream(&ctx->core, (uint8_t *)out, n);

  ctx->st.generated_total += (uint64_t)n;
  ctx->bytes_since_reseed += (uint64_t)n;
  return 0;
}

re4_status_t re4_status(const re4_ctx *ctx) {
  // зручно повертати знімок структури статусу
  re4_status_t z = {0};
  if (!ctx) return z;
  return ctx->st;
}
