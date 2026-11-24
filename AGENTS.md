# RunLog — instructions de développement

Fork public de [beppi/pebble-runlog](https://codeberg.org/beppi/pebble-runlog) (MIT), pour Pebble Time 2 (emery). `origin` est le fork GitHub, `upstream` le dépôt Codeberg.

## Règles

- Travailler uniquement dans ce checkout : pas d'autre clone ni worktree pour modifier le code.
- Committer à la fin de chaque modification.
- Dépôt public : aucun secret, adresse privée, chemin local ou réglage personnel dans les fichiers ou les commits.
- Ne pas publier sur le Pebble Appstore : l'UUID est celui de l'app d'origine.

## Commandes

- Tests : `bash tools/test.sh` (nécessite `node` et `rg`).
- Build : `pebble build`.
- Émulateur : `pebble install --emulator emery --vnc`, puis `pebble screenshot --emulator emery --vnc --no-correction --no-open fichier.png` et `pebble emu-button --emulator emery --vnc click select`.
- Montre : activer **Dev Connection** sur la montre, puis `pebble install --cloudpebble` et `pebble logs --cloudpebble`.

## Pièges

- Le GPS vient du téléphone (`src/pkjs/index.js`) ; la montre ne fait que l'afficher et tient un journal capteurs hors ligne.
- Avec le SDK 4.33, `pebble emu-bt-connection --connected no` n'est pas vu par l'app dans l'émulateur, et les logs et captures passent par la même liaison. Pour tester un comportement hors ligne, forcer temporairement `s_phone_connected = false` à l'init sans le committer, puis valider sur la montre.
- `tools/test_watch_screenshots.sh` utilise `stat -c` (GNU) et échoue tel quel sur macOS.
- Les clés `messageKeys` de `package.json` sont append-only : ne jamais réordonner.
