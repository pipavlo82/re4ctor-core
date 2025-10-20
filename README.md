# RE4CTOR Core

**RE4CTOR Core** is an open-core, high-assurance deterministic RNG / DRBG engine  
designed for cryptography, security infrastructure, and provable randomness.

It has successfully passed the strongest statistical batteries:

| Test Suite           | Status |
|----------------------|--------|
| NIST STS             | ✅ PASS |
| Dieharder            | ✅ PASS |
| PractRand (2⁴⁰+)     | ✅ PASS (no anomalies) |
| TestU01 BigCrush     | 🏆 **FULL PASS (160/160)** |

---

### 🔍 Key Objectives

- **Open & Auditable Core** — transparent C code for community review and trust  
- **Enterprise Extensions** (coming soon): PQ-seed, reseed policy, FIPS 140 health tests, DRBG API  
- **Post-Quantum Ready** — can integrate Kyber/Dilithium seeds and hybrid entropy

---

### 🧪 Verified on Real Batteries

Rigorous validation over multi-gigabyte streams:

```bash
bash scripts/run_practrand.sh
bash scripts/run_dieharder.sh
bash scripts/run_bigcrush.sh
Logs are stored in proof/ for reproducibility.

🏗 Build
bash
Copy code
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/re4_dump | hexdump -C | head
🚧 Roadmap
Reseed Policy (PQ + classical entropy)

SP800-90B Health Tests (online)

DRBG Interface (CTR-DRBG / ChaCha20-DRBG)

Encrypted State File (AES-GCM / ChaCha20-Poly1305)

📜 License
Apache 2.0 — Open-core. Enterprise layer will remain separate/commercial.

Entropy for a Quantum World.
