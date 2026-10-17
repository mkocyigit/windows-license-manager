# Windows License Manager

<p align="center"><img src="docs/icon.png" width="128" alt="icon"></p>

A graphical Windows licensing tool written in C++ (Win32 + WMI). It reimplements what `slmgr.vbs` does,
without VBScript, and adds license-store **backup / restore**, a built-in **user guide** and a
**license types** reference. English and Turkish UI (auto-detected, switchable from the menu).

**Türkçe özet:** `slmgr.vbs` işlevlerinin C++ ile yazılmış menülü GUI karşılığıdır. Lisans deposunu
`<Klasör>\<Bilgisayar Adı>\<Tarih>` düzeninde yedekler/geri yükler; Türkçe ve İngilizce arayüz vardır.
Açık kaynaktır (MIT), istediğiniz gibi kullanabilir ve değiştirebilirsiniz.

## Features

- **License**: show info (`/dli`), detailed info (`/dlv`), expiration (`/xpr`), install key (`/ipk`),
  activate (`/ato`), uninstall key (`/upk`), clear key from registry (`/cpky`)
- **KMS client**: `/skms`, `/ckms`, `/skms-domain`, `/ckms-domain`, `/skhc`, `/ckhc`, `/act-type`
- **KMS server**: `/sai`, `/sri`, `/sprt`, `/sdns`, `/cdns`, `/spri`, `/cpri`
- **Offline activation**: `/dti`, `/atp`
- **Rearm**: `/rearm`, `/rearm-app`, `/rearm-sku`
- **License files**: `/ilc`, `/rilc`
- **Backup / restore** of the license store to `<folder>\<Computer Name>\<Date>`; restore lists the
  backups it finds, warns if a backup is from another machine, and automatically backs up the current
  state first (`%ProgramData%\WindowsLicenseManager\...`)
- User guide and a detailed "License Types" window (Retail, OEM, MAK, KMS, digital license, ...)
- Windows 10 and 11, all editions; x64 and x86 builds; DPI aware

Not implemented: Active Directory activation (`/ad-activation-*`), token activation
(`/lil`, `/ril`, `/stao`, ...) and remote machine targeting.

## Build

Requires Visual Studio or Build Tools with the "Desktop development with C++" workload.

```
build.bat
```

Output: `build\x64\WindowsLicenseManager.exe` and `build\x86\WindowsLicenseManager_x86.exe`.
The exe requests administrator rights on start (needed for licensing operations).

## Project layout

| Path | Purpose |
|---|---|
| `src/main.cpp` | Application (UI, WMI calls, backup/restore) |
| `src/guide_tr.h`, `src/guide_en.h` | User guide and license type texts |
| `src/strings_en.h` | Generated English UI strings |
| `translations/en.tsv` | English translations (source of `strings_en.h`) |
| `tools/gen_strings.py` | Regenerates `strings_en.h` (`--check` lists missing ones) |
| `tools/make_icon.py` | Generates the icon (pure Python) |
| `res/` | Icon, manifest, version resource |

## Translating

UI strings are written in Turkish in `main.cpp` as `T(L"...")`. To add or fix a translation edit
`translations/en.tsv` (`Turkish<TAB>English`) and run `python tools/gen_strings.py`.
Adding another language means adding a table and a language entry in `LoadLanguage()`; PRs welcome.

## Notes and disclaimer

- Use it only for licenses you own or are entitled to use. It talks to Microsoft's Software Licensing
  service / your organization's KMS; it contains no third-party activation servers.
- Backup/restore overwrites system licensing files. License data is bound to the hardware, so a backup
  only works on the same computer and Windows edition. Backups are not encrypted and contain license
  data: do not publish them.
- Written for Windows 11 Enterprise LTSC; behavior on other editions has not been tested. Provided
  "as is", without warranty. See [LICENSE](LICENSE).

## License

[MIT](LICENSE)
