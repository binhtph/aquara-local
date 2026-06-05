# Brand icons

Home Assistant / HACS show an integration's icon from the central
**[home-assistant/brands](https://github.com/home-assistant/brands)** repo — it cannot be served
from this repo directly. These files are pre-sized and ready to submit.

## How to make the icon appear

1. Fork `home-assistant/brands`.
2. Copy this folder to `custom_integrations/aquara_local/` in that fork:
   ```
   custom_integrations/aquara_local/icon.png      (256×256)
   custom_integrations/aquara_local/icon@2x.png   (512×512)
   custom_integrations/aquara_local/logo.png      (256×256)   # optional
   custom_integrations/aquara_local/logo@2x.png   (512×512)   # optional
   ```
3. Open a PR. Once merged, the icon shows in HA's integrations list and in HACS.

Source: derived from `app/assets/icon.png`. If the brands bot complains about whitespace, trim
the transparent margins so the artwork fills the square.
