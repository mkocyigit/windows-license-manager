# Windows Lisans Yöneticisi

[English](README.md) | **Türkçe**

<p align="center"><img src="docs/icon.png" width="128" alt="simge"></p>

C++ (Win32 + WMI) ile yazılmış, grafik arayüzlü bir Windows lisans yönetim aracı. `slmgr.vbs` dosyasının
yaptığı işleri VBScript kullanmadan, doğrudan Windows Yazılım Lisans Hizmeti'ne (WMI) bağlanarak yapar.
Ek olarak lisans deposunu **yedekleme / geri yükleme**, yerleşik **kullanım kılavuzu** ve ayrıntılı bir
**lisans türleri** rehberi içerir. Arayüz Türkçe ve İngilizcedir; Windows diline göre otomatik seçilir,
menüden değiştirilebilir.

Açık kaynaktır (MIT). İstediğiniz gibi kullanabilir, değiştirebilir ve dağıtabilirsiniz.

## Neden?

`slmgr.vbs` komut satırında çalışır, komutları ezbere bilmeyi gerektirir ve VBScript'in kullanımdan
kalkmasıyla ilerde sorun çıkarabilir. Bu program aynı işleri menülerle ve tek bir exe ile yapar.

## Özellikler

- **Lisans:** bilgi göster (`/dli`), ayrıntılı bilgi (`/dlv`), bitiş tarihi (`/xpr`), ürün anahtarı yükle
  (`/ipk`), etkinleştir (`/ato`), anahtarı kaldır (`/upk`), anahtarı kayıt defterinden sil (`/cpky`)
- **KMS istemcisi:** `/skms`, `/ckms`, `/skms-domain`, `/ckms-domain`, `/skhc`, `/ckhc`, `/act-type`
- **KMS sunucusu:** `/sai`, `/sri`, `/sprt`, `/sdns`, `/cdns`, `/spri`, `/cpri`
- **Çevrimdışı etkinleştirme:** kurulum kimliği (`/dti`), onay kimliği gir (`/atp`)
- **Rearm:** `/rearm`, `/rearm-app`, `/rearm-sku`
- **Lisans dosyaları:** `/ilc`, `/rilc`
- **Yedekle / geri yükle:** lisans deposu `<klasör>\<Bilgisayar Adı>\<Tarih>` düzeninde yedeklenir.
  Geri yüklerken bulunan yedekler listelenir, başka bilgisayara ait yedek için uyarı verilir ve
  geri yüklemeden önce mevcut durum otomatik yedeklenir (`%ProgramData%\WindowsLicenseManager\...`).
- Kullanım kılavuzu ve "Lisans Türleri" penceresi (Retail, OEM, MAK, KMS, dijital lisans, ESU, LTSC...).
  Sizin lisans kanalınız otomatik işaretlenir.
- Windows 10 ve 11, tüm sürümler; x64 ve x86; yüksek DPI desteği.

Henüz yok: Active Directory etkinleştirme (`/ad-activation-*`), token etkinleştirme
(`/lil`, `/ril`, `/stao` vb.) ve uzak bilgisayara bağlanma.

## İndirme ve kullanım

Derlenmiş dosyalar için [Releases](../../releases) sayfasına bakın (varsa) ya da aşağıdaki gibi kendiniz
derleyin. Program açılırken yönetici izni ister; lisans işlemleri bunsuz çalışmaz.

- `WindowsLicenseManager.exe`: 64-bit Windows 10/11
- `WindowsLicenseManager_x86.exe`: 32-bit Windows 10

Pencerenin üstündeki hızlı düğmelerle (Durum, Ayrıntılı, Etkinleştir, Yedekle, Geri Yükle, Kılavuz,
Lisans Türleri) çoğu işi yapabilirsiniz; tüm komutlar menülerdedir. Başlık şeridindeki renkli nokta
lisans durumunu gösterir (yeşil lisanslı, sarı tolerans süresinde, kırmızı lisanssız).

## Derleme

Visual Studio veya "Build Tools for Visual Studio" ("C++ ile masaüstü geliştirme" iş yükü) gerekir.

```
build.bat
```

Çıktı: `build\x64\WindowsLicenseManager.exe` ve `build\x86\WindowsLicenseManager_x86.exe`.

## Proje yapısı

| Yol | Görev |
|---|---|
| `src/main.cpp` | Uygulama (arayüz, WMI çağrıları, yedekleme/geri yükleme) |
| `src/guide_tr.h`, `src/guide_en.h` | Kılavuz ve lisans türleri metinleri |
| `src/strings_en.h` | Üretilmiş İngilizce arayüz metinleri |
| `translations/en.tsv` | İngilizce çeviriler (`strings_en.h` kaynağı) |
| `tools/gen_strings.py` | `strings_en.h` dosyasını üretir (`--check` eksikleri listeler) |
| `tools/make_icon.py` | Simgeyi üretir (saf Python) |
| `res/` | Simge, manifest, sürüm bilgisi |

## Çeviri ve katkı

Arayüz metinleri `main.cpp` içinde Türkçe olarak `T(L"...")` biçiminde yazılır. Çeviri eklemek veya
düzeltmek için `translations/en.tsv` dosyasını (`Türkçe<TAB>İngilizce`) düzenleyip
`python tools/gen_strings.py` çalıştırın. Yeni dil eklemek için tablo ve `LoadLanguage()` içine bir satır
yeterlidir. Pull request'ler memnuniyetle karşılanır.

## Önemli notlar

- Yalnızca sahibi olduğunuz veya kullanım hakkınız bulunan lisanslar için kullanın. Program Microsoft'un
  lisans hizmetiyle ve kuruluşunuzun KMS sunucusuyla konuşur; üçüncü taraf etkinleştirme sunucusu içermez.
- Yedekleme/geri yükleme sistem lisans dosyalarının üzerine yazar. Lisans verisi donanıma bağlıdır; yedek
  yalnızca aynı bilgisayarda ve aynı Windows sürümünde işe yarar. Yedekler şifrelenmez ve lisans verisi
  içerir, herkese açık yerlere yüklemeyin.
- Windows 11 Enterprise LTSC üzerinde geliştirildi; diğer sürümlerde denenmedi. Yedekleme/geri yükleme
  özelliği henüz yaygın olarak test edilmedi, önce kendi sisteminizde dikkatle deneyin. Garanti verilmez.

## Lisans

[MIT](LICENSE)
