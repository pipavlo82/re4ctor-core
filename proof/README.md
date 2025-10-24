# re4ctor-core: Statistical Proofs

This directory contains summary evidence of statistical validation for the `re4ctor-core` random generator.

## Available Proofs

| Test Suite              | File                        | Notes                                       |
|-------------------------|----------------------------|---------------------------------------------|
| Dieharder               | dieharder_summary.txt      | 0 failed / 0 weak across 100+ subtests      |
| PractRand               | practrand_summary.txt      | No anomalies up to 256MB                    |
| TestU01 BigCrush        | bigcrush_summary.txt       | 160/160 tests passed                        |

All proofs correspond to the same build of `re4_dump` shipped in the signed release bundle.

To verify supply-chain integrity of that bundle:

```bash
sha256sum -c re4_release.sha256
gpg --verify re4_release.tar.gz.asc re4_release.tar.gz
