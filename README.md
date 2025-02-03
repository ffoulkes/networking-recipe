<!-- markdownlint-disable MD041 -->
[![P4CP build checks](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml)
<!-- markdownlint-enable MD041 -->
# Networking-Recipe (P4 Control Plane)

This repository is a fork of <https://github.com/ipdk-io/networking-recipe>.

In January 2025, Intel decided to roll the IPDK networking-recipe `main`
branch back to their `mev-ts-1.9` release (Sep 30, 2024), effectively
discarding the work I had done since then. This fork allows me to keep
tinkering without stepping on their toes.

## Repositories

`networking-recipe` is a fork of `ipdk-io/networking-recipe`.
Its baseline is
[P4CP v3.3.0.0](https://github.com/ipdk-io/networking-recipe/releases/tag/v3.3.0.0).

`krnlmon/krnlmon` is a fork of `ipdk-io/krnlmon`. Its baseline is
[Krnlmon v3.2.0.0](https://github.com/ipdk-io/krnlmon/releases/tag/v3.2.0.0).

`stratum/stratum` is a fork of `stratum/stratum`, with the addition of
a `ffoulkes` branch imported from `ipdk-io/stratum-dev`.
Its baseline is
[IPDK Stratum v3.3.0.0](https://github.com/ipdk-io/stratum-dev/releases/tag/v3.3.0.0).
