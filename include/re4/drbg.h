#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int healthy;                          // 1 = ok, 0 = fail/paused
  uint64_t generated_total;             // bytes produced since init
  uint64_t reseed_count;                // how many reseeds
  double est_min_entropy_bits_per_byte; // SP800-90B (MCV) on last seed
} re4_status_t;

typedef struct re4_ctx re4_ctx;

/** Create / destroy **/
re4_ctx *re4_create(void);
void re4_free(re4_ctx *ctx);

/** Init from system entropy (urandom + optional RDRAND/jitter later) */
int re4_init(re4_ctx *ctx, const char *domain_tag);

/** Reseed (PQ + класика) з причиною для логів/тегів */
int re4_reseed(re4_ctx *ctx);

/** Generate n bytes; 0 = ok, <0 = error/health-stop */
int re4_generate(re4_ctx *ctx, void *out, size_t n);

/** Snapshot health/metrics */
re4_status_t re4_status(const re4_ctx *ctx);

#ifdef __cplusplus
}
#endif
