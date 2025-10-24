# 🧬 re4ctor-core

Status: Public Drop v0.1
Build type: Verified core + signed user binary
Includes: FastAPI /random service, usage docs, systemd unit, and SBOM proof
License: Restricted public binary, internal entropy core not disclosed

This release provides a reproducible entropy tap with verifiable statistical quality and an operational API layer.

---

API service

There's a small FastAPI app (re4ctor-api) that exposes:

- GET /health  → "ok"
- GET /version → build info + git rev
- GET /info    → usage help
- GET /random  → random bytes, requires API key

Auth:
- header: x-api-key: <KEY>
- or query: ?key=<KEY>

Example (hex):
curl -s -H "x-api-key: local-demo" \
  "http://127.0.0.1:8080/random?n=16&fmt=hex"

Example (raw bytes → hexdump):
curl -s -H "x-api-key: local-demo" \
  "http://127.0.0.1:8080/random?n=64" | hexdump -C

Rate limiting:
- default 10/second per client IP
- max 1,000,000 bytes per request

---

For detailed operator and deployment instructions see:
docs/USAGE.md
docs/re4ctor-api.service.example

---

[![tests](https://github.com/pipavlo82/re4ctor-core/actions/workflows/tests.yml/badge.svg)](https://github.com/pipavlo82/re4ctor-core/actions/workflows/tests.yml) [![build-matrix](https://github.com/pipavlo82/re4ctor-core/actions/workflows/build-matrix.yml/badge.svg)](https://github.com/pipavlo82/re4ctor-core/actions/workflows/build-matrix.yml)

# RE4CTOR Core

[![Build](https://github.com/pipavlo82/re4ctor-core/actions/workflows/build.yml/badge.svg)](https://github.com/pipavlo82/re4ctor-core/actions/workflows/build.yml)
[![License: Apache-2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![BigCrush](https://img.shields.io/badge/TestU01-BigCrush%20PASS-brightgreen)](PROOF.md)
[![PractRand](https://img.shields.io/badge/PractRand-PASS-brightgreen)](PROOF.md)

## CI status

| Check | Status | Logs / Artifacts |
|------|:------:|-------------------|
| Build matrix | ![build-matrix](https://github.com/pipavlo82/re4ctor-core/actions/workflows/build-matrix.yml/badge.svg?branch=main) | [latest runs](https://github.com/pipavlo82/re4ctor-core/actions/workflows/build-matrix.yml?query=branch%3Amain) |
| Tests (Ubuntu smoke) | ![tests](https://github.com/pipavlo82/re4ctor-core/actions/workflows/tests.yml/badge.svg?branch=main) | [latest runs](https://github.com/pipavlo82/re4ctor-core/actions/workflows/tests.yml?query=branch%3Amain) · artifacts: `dieharder_ci.txt`, `RNG_test.log` |

> **Note.** Логи тестів (Dieharder, PractRand) прикріплюються як *Artifacts* на сторінці відповідного run’а.

RE4CTOR Core & API

RE4CTOR Core is a minimal, deterministic, high-entropy byte generator (DRBG + system entropy + SP800-90B minimal entropy layer).
RE4CTOR API is an HTTP service built on top of re4ctor-core, providing cryptographically secure random bytes via REST API.

1️⃣ Build RE4CTOR Core
Dependencies
sudo apt update
sudo apt install -y build-essential cmake ninja-build

Build & Test
cd ~/re4ctor-core
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure


Executables:

build/re4_dump — infinite stream of random bytes

build/re4_tests — internal DRBG self-tests

Quick check:

./build/re4_dump | head -c 32 | hexdump -C


Example output (always different):

00000000  91 c1 a3 e0 3d ca 91 b9  58 86 a0 81 a6 91 84 f1  |....=...X.......|

2️⃣ RE4CTOR API (FastAPI + uvicorn)
Folder layout
/home/pavlo/re4ctor-api/main.py        # FastAPI app
/home/pavlo/re4ctor-api/.venv/         # Python virtual env
/home/pavlo/re4ctor-core/build/re4_dump # Core backend binary
/etc/systemd/system/re4ctor-api.service # Systemd service

2.1 Virtual environment setup
cd ~/re4ctor-api
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt


requirements.txt

fastapi==0.115.0
uvicorn[standard]==0.30.6
slowapi==0.1.9
python-dotenv==1.0.1
pydantic==2.9.2

2.2 Environment config

Create or edit:

nano /home/pavlo/re4ctor-api/.env


Add:

API_HOST=0.0.0.0
API_PORT=8080
API_KEY=change-me
API_GIT=pavlo-lab

2.3 Dev mode run
cd ~/re4ctor-api
source .venv/bin/activate
API_KEY=change-me uvicorn main:app --host 0.0.0.0 --port 8080 --workers 2

2.4 Systemd service (production)

File /etc/systemd/system/re4ctor-api.service

[Unit]
Description=RE4CTOR API service
After=network-online.target
Wants=network-online.target

[Service]
WorkingDirectory=/home/pavlo/re4ctor-api
EnvironmentFile=/home/pavlo/re4ctor-api/.env
User=pavlo
Group=pavlo
ExecStart=/home/pavlo/re4ctor-api/.venv/bin/uvicorn main:app \
    --host ${API_HOST} \
    --port ${API_PORT} \
    --workers 2
Restart=on-failure

[Install]
WantedBy=multi-user.target


Enable and start:

sudo systemctl daemon-reload
sudo systemctl enable --now re4ctor-api
systemctl status re4ctor-api --no-pager


Logs:

journalctl -u re4ctor-api -f

3️⃣ HTTP API Reference
Public Endpoints

GET /health
→ returns "ok"

GET /version
Example:

{
  "name": "re4ctor-api",
  "version": "0.1.0",
  "api_git": "pavlo-lab",
  "core_git": "91b1494",
  "limits": {
    "max_bytes_per_request": 1000000,
    "rate_limit": "10/second"
  }
}


GET /info
→ prints help documentation

Protected Endpoint

GET /random?n=<bytes>[&fmt=hex]

n — number of bytes (required)

fmt — optional (hex or omitted)

Auth required (API key)

Auth methods

Header
x-api-key: change-me

Query
?key=change-me

Examples
16 bytes (hex)
curl -s -H 'x-api-key: change-me' \
  'http://127.0.0.1:8080/random?n=16&fmt=hex'


→ 63968169edffa881f6a4f6d0adc406eb

64 raw bytes
curl -s -H 'x-api-key: change-me' \
  'http://127.0.0.1:8080/random?n=64' --output sample.bin
hexdump -C sample.bin | sed -n '1,6p'

Invalid key
curl -i -s -H 'x-api-key: WRONG' \
  'http://127.0.0.1:8080/random?n=16&fmt=hex'


→ 401 Unauthorized {"detail": "invalid api key"}

4️⃣ Security / Limits

Rate limit: 10/second per client IP

Max bytes per request: 1,000,000

Logs to journalctl:

[re4/random] ip=127.0.0.1 n=16 fmt=hex head4=7eca4753


Change API key or limits in .env then restart:

sudo systemctl restart re4ctor-api

5️⃣ CI / Future Plans

Ideas for GitHub Actions:

Build re4ctor-core (cmake + ninja)

Run ./build/re4_dump | head -c 32

Test /version endpoint schema

Optional: PractRand / dieharder statistical tests

6️⃣ TL;DR

✅ re4ctor-core → generates true entropy bytes (C)
✅ re4ctor-api → REST interface with key auth & rate limit
✅ Runs as systemd service on port 8080

# re4ctor-core (public drop)

## Quick Verify

Download and verify the release in 2 commands:

```bash
sha256sum -c re4_release.sha256
gpg --verify re4_release.tar.gz.asc re4_release.tar.gz
```

If both pass — you have exactly what we built.

---

## Overview

This repo publishes:
- a minimal RNG binary (`re4_dump`) built from a private core,
- a test harness and CI proof,
- an HTTP API wrapper (FastAPI + systemd),
- SBOM and signed release bundles.

What is intentionally **not** published:
- the low-level DRBG/entropy core implementation in `src/`
- internal health/entropy logic
- reproducible seed path / heuristics

The model is similar to Apple Secure Enclave / Intel SGX:
you can call it, you can test it, but you don't get the guts.

---

## How to build locally (WSL / Ubuntu)

```bash
# clone this repo
cd re4ctor-core

# configure + build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"

# run smoke test
./build/re4_dump | head -c 32 | hexdump -C
```

You should see different hex on every run.

---

## Basic self-tests

```bash
cd build
ctest --output-on-failure
```

This runs a tiny `re4_tests` binary that:

- initializes the generator,
- pulls 32 bytes twice,
- checks they are not all-zero and not identical.

CI also runs:
- `dieharder` on the stream
- `PractRand` on stdin64 (short mode for GitHub Action runtime limits)

---

## SBOM / Release bundle

We generate an SBOM (SPDX 2.3) from only what we ship:

```bash
syft packages dir:release -o spdx-json > release/SBOM.spdx.json
```

Then we pack + sign:

```bash
tar -C release -czf re4_release.tar.gz .
sha256sum re4_release.tar.gz > re4_release.sha256
gpg --armor --detach-sign re4_release.tar.gz  # -> re4_release.tar.gz.asc
```

Deliverables:

- `re4_release.tar.gz` : binaries + helper scripts + SBOM  
- `re4_release.sha256` : hash for integrity  
- `re4_release.tar.gz.asc` : detached GPG signature  

Consumers verify:

```bash
sha256sum -c re4_release.sha256
gpg --verify re4_release.tar.gz.asc re4_release.tar.gz
```

If both pass, they got exactly what we built.

---

## API service

There's a small FastAPI app (`re4ctor-api`) that exposes:

- `GET /health` → `"ok"`
- `GET /version` → build info + git rev
- `GET /info` → usage help
- `GET /random` → random bytes, requires API key

Auth:
- header: `x-api-key: <KEY>`
- or query: `?key=<KEY>`

Example (hex):

```bash
curl -s -H "x-api-key: change-me" \
  "http://127.0.0.1:8080/random?n=16&fmt=hex"
```

Example (raw bytes → hexdump):

```bash
curl -s -H "x-api-key: change-me" \
  "http://127.0.0.1:8080/random?n=64" | hexdump -C
```

Rate limiting:
- default 10/second per client IP
- max 1,000,000 bytes per request

---

### systemd unit (simplified)

```ini
[Service]
WorkingDirectory=/home/pavlo/re4ctor-api
EnvironmentFile=/home/pavlo/re4ctor-api/.env
ExecStart=/home/pavlo/re4ctor-api/.venv/bin/uvicorn main:app \
  --host ${API_HOST} --port ${API_PORT} --workers 2
Restart=on-failure
User=pavlo
Group=pavlo
```

`.env` looks like:

```bash
API_HOST=0.0.0.0
API_PORT=8080
API_KEY=change-me
API_GIT=my-lab-tag
```

We expose `core_git` (commit of generator core) and `api_git`
(commit/tag for the API) via `/version`.

---

## Threat model

We intentionally do not publish `src/entropy/*.c`, `src/drbg/*.c`,
or other internals. The shipped binary is treated like a hardware RNG:

- You can audit behavior via statistical suites (`dieharder`, `PractRand`).
- You can continuously poll `/random` and monitor.
- You cannot trivially clone the core logic.

This gives:

- Transparency of output quality.
- Traceability (hash/sig/SBOM).
- Controlled IP leakage (the "secret sauce" is not open).

In other words:  
**we prove the tap water is clean, without giving you the plumbing diagram.**
