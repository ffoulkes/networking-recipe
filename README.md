<!-- markdownlint-disable MD041 -->
[![Build checks](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml)
[![Lint checks](https://github.com/ffoulkes/networking-recipe/actions/workflows/linters.yml/badge.svg)](https://github.com/ffoulkes/networking-recipe/actions/workflows/linters.yml)
<!-- markdownlint-enable MD041 -->
# ffoulkes/networking-recipe

## Background

In January 2025, Intel decided to roll the IPDK networking-recipe `main`
branch back to their `mev-ts-1.9` release (Sep 30, 2024), effectively
discarding the work I had done since then.
This fork allows me to tinker without stepping on their toes.

## Contents

This repository is a fork of <https://github.com/ipdk-io/networking-recipe>.
Its baseline is
[P4CP v3.3.0](https://github.com/ipdk-io/networking-recipe/releases/tag/v3.3.0).

Ongoing development is done in the `ffoulkes` branch.

## Submodules

- [`krnlmon/krnlmon`](https://github.com/ffoulkes/krnlmon)
is a fork of <https://github.com/ipdk-io/krnlmon>.

- [`stratum/stratum`](https://github.com/ffoulkes/stratum) is a fork of
<https://github.com/stratum/stratum> (the mothership).</br>
The `ffoulkes` branch was imported from <https://ipdk-io/stratum-dev>.

## Tags

- The `v3.3.0` tag corresponds to [IPDK Release v3.3.0](https://github.com/ipdk-io/networking-recipe/releases/tag/v3.3.0).
- The `v3.2.0.0` tag corresponds to [IPDK Release v3.2.0.0](https://github.com/ipdk-io/networking-recipe/releases/tag/v3.2.0.0).
- The `mev-ts-1.9` tag marks the point at which the `ffoulkes` branch diverges from the ipdk-io `main` branch.
