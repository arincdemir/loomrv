# LoomRV Documentation

The authoritative LoomRV manual is stored in [`wiki/Home.md`](wiki/Home.md).

This source travels with the repository and archival releases. The public
[GitHub Wiki](https://github.com/arincdemir/loomrv/wiki) is generated from it
with GitHub-compatible internal links.

Validate the manual and its built command examples:

```bash
python3 docs/wiki_tool.py validate --build-dir build
```

Check or update a local GitHub Wiki checkout:

```bash
python3 docs/wiki_tool.py sync --check --destination loomrv.wiki
python3 docs/wiki_tool.py sync --write --destination loomrv.wiki
```
