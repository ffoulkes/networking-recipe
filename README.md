<!-- markdownlint-disable MD041 -->
[![P4CP build checks](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml/badge.svg)](https://github.com/ipdk-io/networking-recipe/actions/workflows/pipeline.yml)
<!-- markdownlint-enable MD041 -->
# Networking-Recipe (P4 Control Plane)

This repository is a fork of <https://github.com/ipdk-io/networking-recipe>.

In January 2025, Intel decided to roll the IPDK networking-recipe `main`
branch back to their `mev-ts-1.9` release (Sep 30, 2024), effectively
discarding the work I had done since then. This fork allows me to keep
tinkering without stepping on their toes.

The main branch of the fork is `ffoulkes`, to provide a clear indication
of the point where it deviates from the IPDK `main` branch. The branch
point corresponds to [P4CP Release v3.3.0.0](https://github.com/ipdk-io/networking-recipe/releases/tag/v3.3.0.0).

Note that the submodules in this repository still reference repositories
on ipdk-io. They will be updated to local forks as time allows.
