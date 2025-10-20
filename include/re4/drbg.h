#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int healthy;              // 1 = ok, 0 = fail/paused
  uint64_t generated_total; // bytes produced since init
  uint64_t reseed_count;    // how many reseeds
  double   est_min_entropy_bits_per_byte; // SP800-90B (MCV) on last seed
} re4_status_t;

typedef struct re4_ctx re4_ctx;

/** Create / destroy **/
re4_ctx *re4_create(void);
void     re4_free(re4_ctx *ctx);

/** Init from system entropy (urandom + optional RDRAND/jitter later) */
int  re4_init(re4_ctx *ctx, const char *domain_tag);

/** Generate random bytes (auto-reseed per policy; health-checked). */
int  re4_random(re4_ctx *ctx, void *out, size_t n);

/** Force reseed now (e.g., after adding external entropy). */
int  re4_reseed(re4_ctx *ctx);

/** Add external entropy (does not reveal it back). */
int  re4_add_entropy(re4_ctx *ctx, const void *buf, size_t n);

/** Status snapshot (no secrets). */
re4_status_t re4_status(const re4_ctx *ctx);

/** Tunables via env:
 *  RE4_RESEED_BYTES (default 33554432 = 32 MiB)
 *  RE4_RESEED_SECS  (default 600 = 10 min)
 *  RE4_HEALTH_RCT_MAXRUN (default 33)
 *  RE4_HEALTH_AP_WIN (default 1024), RE4_HEALTH_AP_LOW/HIGH (default 317/707 for LSB)
 */
#ifdef __cplusplus
}
#endif
