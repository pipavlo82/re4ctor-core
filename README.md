🧬 re4ctor-core

Status: Public Drop v0.1
Build type: Verified core + signed user binary
Includes: FastAPI /random service, usage docs, systemd unit, and SBOM proof
License: Restricted public binary — internal entropy / DRBG core is not disclosed

This release provides a reproducible entropy tap with verifiable statistical quality and an operational API layer.
You get a callable RNG service with proofs.
You do not get the private DRBG/entropy internals.

1. Quick Verify (supply chain integrity)

Download and verify:

sha256sum -c re4_release.sha256
gpg --verify re4_release.tar.gz.asc re4_release.tar.gz

If both pass — you have exactly what we built.

Extract and test:

mkdir -p ~/re4ctor-local
tar -C ~/re4ctor-local -xzf re4_release.tar.gz
~/re4ctor-local/bin/re4_dump | head -c 32 | hexdump -C

You should see non-zero, non-repeating hex on every run.

2. API Service

A FastAPI app (re4ctor-api) exposes:

GET /health → "ok"
GET /version → build info + git rev
GET /info → usage help
GET /random → random bytes (requires API key)

Auth

Dev/default:
header: x-api-key: local-demo
or query: ?key=local-demo

Prod:
Set API_KEY in .env and restart the service.

Examples

16 bytes in hex:
curl -s -H "x-api-key: local-demo" "http://127.0.0.1:8080/random?n=16&fmt=hex
"

64 raw bytes:
curl -s -H "x-api-key: local-demo" "http://127.0.0.1:8080/random?n=64
" | hexdump -C

Invalid key:
curl -i -s -H "x-api-key: WRONG" "http://127.0.0.1:8080/random?n=16&fmt=hex
"

→ 401 Unauthorized {"detail": "invalid key"}

Limits

10 requests/sec per IP
Max 1,000,000 bytes per request

Example log:
[re4/random] ip=127.0.0.1 n=16 fmt=hex head4=7eca4753

3. Local Run (Developer Mode)

cd ~/re4ctor-local/re4ctor-api

Create .env:
API_HOST=127.0.0.1
API_PORT=8080
API_KEY=local-demo
API_GIT=local-api
CORE_GIT=release-core

Start the API:
./api_start.sh start

Check:
curl -s http://127.0.0.1:8080/health

curl -s http://127.0.0.1:8080/version

curl -s -H "x-api-key: local-demo" "http://127.0.0.1:8080/random?n=32&fmt=hex
"

Manage:
./api_start.sh status
./api_start.sh log
./api_start.sh stop

Logs: ~/re4ctor-local/re4ctor-api/logs/api.log

4. Production Deployment (systemd)

Unit file: docs/re4ctor-api.service.example

Main features:

Dedicated service user

.env for API_KEY and metadata

Sandbox (ProtectSystem, ProtectHome, MemoryDenyWriteExecute)

Auto-restart on failure

Steps:

Copy bundle to /opt/re4ctor

Set API_KEY in .env

Enable and start with systemctl

5. Build & Self-Test (WSL / Ubuntu)

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
./build/re4_dump | head -c 32 | hexdump -C

cd build
ctest --output-on-failure

Runs internal self-tests and short PractRand/Dieharder bursts.

6. SBOM & Signed Release

syft packages dir:release -o spdx-json > release/SBOM.spdx.json
tar -C release -czf re4_release.tar.gz .
sha256sum re4_release.tar.gz > re4_release.sha256
gpg --armor --detach-sign re4_release.tar.gz

Verify:
sha256sum -c re4_release.sha256
gpg --verify re4_release.tar.gz.asc re4_release.tar.gz

7. Threat Model

Not published:
src/entropy/.c
src/drbg/.c
internal heuristics / seed path logic

Treat shipped binary as a hardware RNG:

auditable via PractRand / Dieharder / BigCrush

verifiable SBOM & GPG signature

not clonable

We prove the water is clean — without showing the plumbing.

8. Statistical Proof

proof/ includes summaries:

dieharder

PractRand

BigCrush

All tests passed without critical bias.
Raw logs available on request.

9. CI / Roadmap

CI builds and tests re4_dump on multiple platforms.
Plans: appliance mode, internal network /random service.

10. TL;DR

✅ re4ctor-core → C binary for entropy bytes
✅ re4ctor-api → REST with key auth + rate limit
✅ Signed bundle + SBOM + GPG sig
✅ systemd hardened service
❌ DRBG internals remain private
