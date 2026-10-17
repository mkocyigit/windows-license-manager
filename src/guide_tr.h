// Windows Lisans Yöneticisi - kılavuz ve lisans türleri metinleri
#pragma once

static const wchar_t* kGuideText =
LR"LY(WINDOWS LİSANS YÖNETİCİSİ — KULLANIM KILAVUZU
================================================

Bu program, Windows'un yerleşik lisans aracı slmgr.vbs'in yaptığı işleri grafik arayüzle ve
doğrudan C++ ile yapar. Windows 10 ve Windows 11'in tüm sürümlerinde (Home, Pro, Enterprise,
LTSC, Education) çalışır. Komutlar, Windows'un Yazılım Lisans Hizmeti'ne (WMI üzerinden
SoftwareLicensingService / SoftwareLicensingProduct) gönderilir. Program her açılışta
yönetici yetkisi ister; lisans işlemleri bu yetki olmadan çalışmaz.

DİL
  Program ilk açılışta Windows görüntüleme dilinize göre Türkçe veya İngilizce açılır. "Dil / Language"
  menüsünden istediğiniz an değiştirebilirsiniz; seçiminiz hatırlanır.

ETKİNLEŞTİRME KİMLİĞİ (Activation ID) NEDİR?
  Her Windows sürümünün / lisansının bir GUID'i vardır. Çoğu komutta bu kutu boş bırakılırsa
  komut Windows'un birincil lisansına uygulanır. "all" yazarsanız (yalnızca Lisans Bilgisi /
  Ayrıntılı Bilgi komutlarında) anahtarı yüklü tüm ürünler (Office vb. dahil) listelenir.
  Kimlikleri görmek için "Ayrıntılı" düğmesine basın.


1. HIZLI DÜĞMELER (pencerenin üstünde)
------------------------------------------------
Durum ........ Windows lisansının adını, açıklamasını, etkinleştirme kimliğini, anahtarın son
               5 hanesini, lisans durumunu ve varsa kalan tolerans süresini gösterir.
               (slmgr /dli ile aynıdır.) Hiçbir şeyi değiştirmez.
Ayrıntılı .... Durum'un ayrıntılı hâli: uygulama kimliği, genişletilmiş PID, anahtar kanalı
               (Retail / OEM / Volume:MAK ...), çevrimdışı kurulum kimliği, KMS bilgileri ve
               BIOS/OEM anahtarı. (slmgr /dlv)
Etkinleştir .. Yüklü anahtarla Microsoft'un etkinleştirme sunucularına bağlanıp Windows'u
               etkinleştirir. İnternet gerekir. (slmgr /ato)
Yedekle ...... Lisans deposunu seçtiğiniz klasöre yedekler (bkz. bölüm 3).
Geri Yükle ... Daha önce alınmış bir yedeği geri yükler (bkz. bölüm 3).
Kılavuz ...... Bu metin.
Lisans Türleri Retail, OEM, MAK, KMS, dijital lisans vb. tüm lisans türlerinin ayrı bir
               pencerede ayrıntılı anlatımı; sizin lisans türünüz otomatik seçilir.

Pencerenin en üstündeki renkli nokta: yeşil = lisanslı, sarı = tolerans/bildirim süresinde,
kırmızı = lisanssız. Yanında sürüm adı ve anahtar kanalı yazar.


2. LİSANS MENÜSÜ
------------------------------------------------
Lisans bilgisini göster (/dli) .... Durum ile aynı; isterseniz belirli bir etkinleştirme kimliği
                                    ya da "all" yazabilirsiniz.
Ayrıntılı lisans bilgisi (/dlv) ... Ayrıntılı ile aynı; kimlik veya "all" kabul eder.
Bitiş tarihini göster (/xpr) ...... Etkinleştirmenin ne zaman sona ereceğini söyler. Kalıcı
                                    (Retail, OEM, MAK) lisanslarda "kalıcı olarak
                                    etkinleştirildi" yazar; KMS lisanslarında 180 günlük
                                    bitiş tarihini, deneme sürümlerinde deneme bitişini gösterir.
Ürün anahtarı yükle (/ipk) ........ Yeni bir ürün anahtarı girer (XXXXX-XXXXX-XXXXX-XXXXX-XXXXX).
                                    Anahtar yüklenir ama otomatik etkinleşmez; ardından
                                    "Etkinleştir"e basın. Mevcut anahtarın yerini alır.
Windows'u etkinleştir (/ato) ...... Etkinleştirmeyi dener. MAK/Retail anahtarlarda Microsoft
                                    sunucularıyla, KMS istemcilerinde KMS sunucusuyla görüşür.
                                    Sonucu (başarılı / hata kodu) çıktıda görürsünüz.
Ürün anahtarını kaldır (/upk) ..... Yüklü anahtarı siler. Windows lisanssız duruma düşer.
                                    Onay sorar.
Anahtarı kayıt defterinden sil (/cpky)
                                    Anahtarın düz metin hâlini kayıt defterinden temizler
                                    (güvenlik amaçlı). Lisans geçerli kalır, etkinleştirme
                                    bozulmaz. Onay sorar.


3. YEDEKLEME MENÜSÜ
------------------------------------------------
Lisansı yedekle ... Bir klasör seçersiniz; yedek şu düzende oluşturulur:
                        <seçtiğiniz klasör>\<Bilgisayar Adı>\<Tarih>
                    (örn. D:\Yedek\X870E-APEX\2026-10-18_01-07-47). Bu klasöre şunlar kopyalanır:
                      - C:\Windows\System32\spp\store  (lisans deposu: data.dat, tokens.dat ...)
                      - NetworkService profilindeki SoftwareProtectionPlatform klasörü
                      - kayıt defterindeki SoftwareProtectionPlatform anahtarı (spp.reg)
                      - info.txt (bilgisayar adı, makine kimliği, sürüm, tarih, lisans durumu)
                    Kopyalama sırasında "Yazılım Koruma" hizmeti (sppsvc) birkaç saniye durdurulup
                    yeniden başlatılır. Yedek şifrelenmez; saklarken dikkat edin.
Yedekten geri yükle ... Yedek alırken seçtiğiniz klasörü seçersiniz; program altındaki
                    <Bilgisayar Adı>\<Tarih> yedeklerini bulup listeler (bu bilgisayarın
                    yedekleri ve en yeniler üstte), siz birini seçersiniz. Program önce yedeğin bu
                    bilgisayara ait olup olmadığını kontrol eder (makine kimliği). Sonra
                    geri yüklemeden ÖNCE mevcut durumu otomatik olarak
                    C:\ProgramData\WindowsLicenseManager\<Bilgisayar Adı>\<Tarih>_onceki klasörüne yedekler, ardından
                    dosyaları ve kayıt defterini geri yükler. Sorun çıkarsa o önceki yedeği
                    geri yükleyerek eski duruma dönebilirsiniz.
                    Bu yöntem aynı bilgisayarda, aynı Windows sürümünde işe yarar. Lisans
                    verisi donanıma bağlıdır; başka bilgisayara taşınamaz.


4. KMS İSTEMCİSİ MENÜSÜ  (kuruluşlarda KMS ile etkinleşen bilgisayarlar için)
------------------------------------------------
KMS sunucusu ayarla (/skms) ....... Bilgisayarın etkinleşmek için bağlanacağı KMS sunucusunu
                                    elle belirler: "sunucu" veya "sunucu:port" (IPv6 için
                                    [adres]:port). Port yazılmazsa varsayılan (1688) kullanılır.
KMS sunucusu ayarını temizle (/ckms)  Elle girilen sunucuyu siler; bilgisayar DNS'ten otomatik
                                    bulmaya döner.
KMS arama etki alanı ayarla (/skms-domain)
                                    KMS sunucusunun DNS SRV kaydının aranacağı etki alanını
                                    belirler (ör. sirket.local).
KMS arama etki alanını temizle (/ckms-domain)  Bu ayarı kaldırır.
KMS ana bilgisayar önbelleğini etkinleştir / devre dışı bırak (/skhc, /ckhc)
                                    Bulunan KMS sunucusunun hatırlanıp hatırlanmayacağını
                                    belirler. Önbellek açıkken istemci her seferinde yeniden
                                    aramaz; ağ değişirse kapatmak gerekebilir.
Etkinleştirme türü (/act-type) .... Bilgisayarın hangi toplu etkinleştirme türünü kullanabileceğini
                                    sınırlar: 0 = hepsi, 1 = Active Directory, 2 = KMS,
                                    3 = Token.


5. KMS SUNUCUSU MENÜSÜ  (bilgisayar KMS ana bilgisayarı olarak kurulmuşsa)
------------------------------------------------
Etkinleştirme aralığı (/sai) ...... İstemci etkinleşemediğinde ne kadar sürede bir yeniden
                                    deneyeceğini dakika olarak belirler (varsayılan 120).
Yenileme aralığı (/sri) ........... Etkinleşmiş istemcilerin etkinleşmeyi kaç dakikada bir
                                    yenileyeceğini belirler (varsayılan 10080 = 7 gün).
Dinleme portu (/sprt) ............. KMS'in dinleyeceği TCP portu (varsayılan 1688).
DNS yayınlamayı etkinleştir / devre dışı bırak (/sdns, /cdns)
                                    KMS sunucusunun kendini DNS'te (SRV kaydı) duyurup
                                    duyurmayacağını belirler.
Normal öncelik / Düşük öncelik (/spri, /cpri)
                                    KMS hizmetinin işlemci önceliğini belirler. Düşük öncelik,
                                    sunucu başka işler de yapıyorsa kullanılır.


6. ÇEVRİMDIŞI ETKİNLEŞTİRME MENÜSÜ  (internet olmayan makineler için; telefonla etkinleştirme)
------------------------------------------------
Kurulum kimliğini göster (/dti) ... Bilgisayara özgü uzun "Kurulum Kimliği"ni (Installation ID)
                                    gösterir. Bu kimliği Microsoft'un etkinleştirme telefon
                                    hattında / etkinleştirme sayfasında verirsiniz.
Onay kimliği gir (/atp) ........... Karşılığında aldığınız "Onay Kimliği"ni (Confirmation ID)
                                    girersiniz; Windows çevrimdışı etkinleşir.
  Adımlar: (1) Kurulum kimliğini göster, (2) Microsoft'tan onay kimliğini al,
  (3) Onay kimliğini gir. Başarılıysa Durum'da "Lisanslı" görünür.


7. REARM MENÜSÜ
------------------------------------------------
Windows rearm (/rearm) ............ Deneme/tolerans süresini sıfırlar. Sınırlı sayıda
                                    yapılabilir, kalan hak sayısı azalır. Sonrasında bilgisayarı
                                    yeniden başlatmak gerekir. Onay sorar.
Uygulama rearm (/rearm-app) ....... Belirli bir uygulamanın (uygulama kimliğiyle) süresini
                                    sıfırlar. Varsayılan kimlik Windows'a aittir.
SKU rearm (/rearm-sku) ............ Belirli bir lisansın (etkinleştirme kimliği) süresini sıfırlar.
  Rearm, kalıcı olarak etkinleştirilmiş (Retail / OEM / MAK) bir Windows'ta gerekmez.


8. LİSANS DOSYALARI MENÜSÜ
------------------------------------------------
Lisans dosyası yükle (/ilc) ....... Bir .xrm-ms lisans dosyasını sisteme yükler. Genelde
                                    Microsoft'un verdiği ek lisans dosyaları için kullanılır.
Sistem lisanslarını yeniden yükle (/rilc)
                                    Windows'un kendi lisans dosyalarını (C:\Windows\System32\spp\
                                    tokens ve \oem) yeniden yükler. Bozulmuş lisans verisi
                                    onarımında işe yarar.


9. SIK KARŞILAŞILAN SORUNLAR
------------------------------------------------
0xC004F074  KMS sunucusuna ulaşılamadı. Ağı, sunucu adını ve portu kontrol edin.
0xC004C003  Anahtar engellenmiş / geçersiz. Anahtarı doğrulayın.
0xC004F050  Ürün anahtarı geçersiz. Anahtarı ve sürümü kontrol edin.
0xC004C008  Anahtarın etkinleştirme hakkı dolmuş (MAK'ta sık görülür). Kalan hak için
            Microsoft Toplu Lisanslama'ya başvurun veya telefonla etkinleştirin.
0x8007232B  DNS'te KMS kaydı bulunamadı.
0x80070005  Erişim reddedildi. Programın yönetici olarak çalıştığından emin olun.

NOT: Bu program yalnızca sahip olduğunuz / kullanım hakkınız bulunan lisansları yönetmek
içindir. Etkinleştirme, Microsoft'un kendi sunucularına veya kuruluşunuzun KMS sunucusuna
yapılır; üçüncü taraf etkinleştirme sunucularına yönlendirme özelliği yoktur.
)LY";

// ----------------------------------------------------------------------------------------
struct LicType { const wchar_t* title; const wchar_t* text; };

static const LicType kLicTypes[] = {
{ L"Lisans türü nedir? (Başlarken)",
LR"LY(LİSANS TÜRÜ ≠ WINDOWS SÜRÜMÜ

"Sürüm" (Home, Pro, Enterprise, Education...) hangi özelliklere sahip olduğunuzu söyler.
"Lisans türü" ise o sürümü NASIL ve KİMDEN aldığınızı, yani lisansın haklarını ve
etkinleştirme biçimini belirler. Aynı sürüm (örn. Pro) Retail, OEM, MAK veya KMS lisansıyla
gelebilir; bu tür taşınabilirliği, etkinleştirme şeklini ve geçerlilik süresini değiştirir.

Kendi lisans türünüzü anlamak için programda "Ayrıntılı" düğmesine basın ve "Ürün Anahtarı
Kanalı" satırına bakın. Bu pencerede sizin kanalınıza uyan tür otomatik seçilir.

KANAL ADLARI (Ürün Anahtarı Kanalı):
  Retail ............ Perakende
  OEM:DM / OEM:SLP / OEM:NONSLP ... OEM türleri
  Volume:MAK ........ Çoklu Etkinleştirme Anahtarı (MAK)
  Volume:GVLK ....... KMS istemci anahtarı
  Volume:CSVLK ...... KMS ana bilgisayar anahtarı

GENEL KURAL: Lisans yalnızca Microsoft'un lisans koşullarına uygun kullanılabilir. Her türün
kullanım hakkı (kaç bilgisayar, taşınabilir mi, kimler kullanabilir) farklıdır.)LY" },

{ L"Retail (Perakende / FPP)",
LR"LY(RETAIL — PERAKENDE (Full Packaged Product)

NEDİR
  Mağazadan, Microsoft Store'dan veya bir bayiden kutulu / dijital olarak satın alınan
  bireysel lisanstır. Anahtar kanalı "Retail" olarak görünür.

NASIL ETKİNLEŞİR
  25 karakterlik ürün anahtarı girilir, Microsoft'un sunucularıyla (internetten veya
  telefonla) etkinleştirilir. Windows 10/11'de etkinleştirme çoğunlukla "dijital lisans"a
  dönüşür ve donanım kimliğine bağlanır (bkz. Dijital Lisans).

TAŞINABİLİRLİK
  Bir bilgisayarda kullanımdan kaldırılıp başka bir bilgisayara aktarılabilir; aynı anda
  yalnızca bir bilgisayarda kullanılabilir. En esnek bireysel lisans türüdür.

GEÇERLİLİK
  Kalıcıdır; süre sınırı yoktur. Windows'un destek ömrü boyunca güncelleme alır.

KİMLER İÇİN
  Kendi bilgisayarını kuran, parça değiştiren, lisansı ileride taşımak isteyen bireysel
  kullanıcılar.

BU PROGRAMDA
  Anahtar Yükle → Etkinleştir. Durum'da kanal "Retail" görünür.)LY" },

{ L"OEM (Bilgisayarla birlikte gelen lisans)",
LR"LY(OEM — ORIGINAL EQUIPMENT MANUFACTURER

NEDİR
  Bilgisayar üreticisinin (Dell, HP, Lenovo, ASUS vb.) veya sistem toplayıcının cihazla birlikte
  sattığı lisanstır. Daha ucuzdur, çünkü o cihaza bağlıdır.

ALT TÜRLERİ
  OEM:DM ....... "Digital Marker". Anahtar anakartın BIOS/UEFI'sine (MSDM tablosu) gömülüdür.
                 Windows 8 ve sonrasında yaygındır. Temiz kurulumda anahtar sormadan otomatik
                 bulunur ve etkinleşir. Programda "Ayrıntılı" ile BIOS/OEM anahtarı görülür.
  OEM:SLP ...... "System Locked Preinstallation". Windows 7 dönemi yöntemi; BIOS'taki SLIC
                 tablosu ve üretici sertifikasıyla çevrimdışı etkinleşir.
  OEM:NONSLP ... Üreticinin BIOS'una gömülü olmayan, ancak OEM'e verilen anahtarlar; çoğunlukla
                 çevrimiçi etkinleşir.

TAŞINABİLİRLİK
  Lisans koşullarına göre cihaza bağlıdır; başka bir bilgisayara aktarılamaz. Anakart
  değişimi lisansı geçersiz kılabilir.

GEÇERLİLİK
  Kalıcıdır; cihazın ömrüyle sınırlıdır.

KİMLER İÇİN
  Hazır bilgisayar alanlar; marka dizüstü / masaüstü kullanıcıları.

BU PROGRAMDA
  Genellikle yapacak bir şey yoktur, Windows kendiliğinden etkinleşir. "Ayrıntılı" ile BIOS'taki
  anahtarı görebilirsiniz.)LY" },

{ L"Dijital Lisans (Hak / HWID)",
LR"LY(DİJİTAL LİSANS (Digital Entitlement / HWID)

NEDİR
  Windows 10 ve 11'de ürün anahtarı yerine, donanımınızın "parmak izine" (Hardware ID) bağlı,
  Microsoft sunucusunda tutulan bir etkinleştirme hakkıdır. Yeni ürün anahtarı yazmanız
  gerekmez.

NASIL OLUŞUR
  - Windows 7/8.1'den ücretsiz yükseltme sonrasında,
  - Microsoft Store'dan Windows satın alındığında,
  - Retail anahtarla etkinleştirme sonrasında (otomatik),
  - Insider Preview programında etkin bir Windows'tan.

TEMİZ KURULUMDA
  Aynı donanımda Windows'u yeniden kurup "Ürün anahtarım yok" derseniz, internete bağlanınca
  kendiliğinden etkinleşir. Doğru sürümü seçmeniz gerekir (Home/Pro).

MİCROSOFT HESABI
  Lisansı Microsoft hesabınıza bağlarsanız, büyük donanım değişikliğinden (özellikle anakart)
  sonra Ayarlar → Etkinleştirme → "Sorun Gider" → "Bu cihazda donanımı yakın zamanda
  değiştirdim" ile lisansı yeni donanıma taşıyabilirsiniz.

ANAHTAR GÖRÜNÜR MÜ?
  Hayır, yalnızca son 5 hane görünür. Bu yüzden yedekleme programı "anahtar" değil, lisans
  deposunu yedekler.)LY" },

{ L"Volume: MAK (Çoklu Etkinleştirme Anahtarı)",
LR"LY(VOLUME: MAK — MULTIPLE ACTIVATION KEY

NEDİR
  Toplu Lisanslama (Volume Licensing) anlaşmasıyla verilen, kuruluşa özel bir anahtardır. Tek
  bir anahtar, Microsoft'un tanımladığı belirli sayıda bilgisayarda etkinleştirme yapabilir.
  Her etkinleştirme bu sayıdan düşer.

NASIL ETKİNLEŞİR
  Anahtar yüklenir, bilgisayar Microsoft'un etkinleştirme sunucularına bir kez bağlanır
  (internetten ya da telefonla). Etkinleştirmeden sonra KALICIDIR: sürekli bağlantı, sunucu
  veya yenileme gerekmez.
  İnternet olmayan makinelerde: Çevrimdışı menüsü → Kurulum kimliğini göster, Onay kimliği gir.

HAK SAYISI
  Anahtarın kaç kez kullanılabileceği anlaşmaya bağlıdır. Haklar bittiğinde 0xC004C008 hatası
  alınır; ek hak için Toplu Lisanslama sağlayıcınıza başvurulur. Aynı bilgisayarda yeniden
  kurulum bazı durumlarda yeni bir hak harcayabilir.

TAŞINABİLİRLİK
  Anahtar, anlaşma kapsamındaki cihazlarda kullanılabilir; kuruluşa aittir.

GEÇERLİLİK
  Kalıcıdır. KMS gibi 180 günlük yenileme yoktur.

KİMLER İÇİN
  Küçük/orta ölçekli işletmeler, ağdan bağımsız çalışan veya az sayıda bilgisayarı olan
  kurumlar. Yedekleme/geri yükleme MAK bilgisayarlarında özellikle faydalıdır.

BU PROGRAMDA
  Ürün anahtarı yükle → Windows'u etkinleştir. "Ayrıntılı"da kanal "Volume:MAK" görünür.)LY" },

{ L"Volume: GVLK (KMS İstemcisi)",
LR"LY(VOLUME: GVLK — GENERIC VOLUME LICENSE KEY (KMS İSTEMCİSİ)

NEDİR
  Toplu Lisanslamada, bilgisayarın bir kuruluş içi KMS sunucusuna bağlanarak etkinleşmesini
  sağlayan istemci anahtarıdır. Windows'un Volume sürümlerinde (Pro Volume, Enterprise,
  Education) çoğunlukla zaten yüklü gelir; anahtar girmeniz gerekmez.

NASIL ÇALIŞIR
  İstemci, ağda DNS "SRV" kaydıyla (_vlmcs._tcp) KMS sunucusunu bulur ve etkinleşir. KMS
  sunucusu yerel ağdadır; Microsoft'a gidilmez.

SÜRE
  Etkinleşme 180 gün geçerlidir. İstemci varsayılan olarak 7 günde bir yenilenir (varsayılan
  yenileme aralığı 10080 dk). Ağa 180 gün erişemezse lisans bildirim moduna geçer.

EŞİK
  KMS sunucusunun etkinleştirme yapabilmesi için en az sayıda istemci görmesi gerekir:
  Windows istemcileri için 25, sunucu işletim sistemleri için 5 bilgisayar.

KİMLER İÇİN
  Alan adı (domain) ağındaki yüzlerce bilgisayara sahip büyük kuruluşlar, okullar.

BU PROGRAMDA
  KMS İstemcisi menüsü: sunucu ayarla, temizle, arama etki alanı, önbellek, etkinleştirme türü.
  Etkinleştir ile sunucuya bağlanılır. /xpr ile bitiş tarihini görebilirsiniz.)LY" },

{ L"Volume: CSVLK (KMS Ana Bilgisayar Anahtarı)",
LR"LY(VOLUME: CSVLK — CUSTOMER SPECIFIC VOLUME LICENSE KEY (KMS ANA BİLGİSAYARI)

NEDİR
  Kuruluşunuzda bir KMS sunucusu (ana bilgisayar) kurmak için kullanılan anahtardır. Bu anahtarla
  etkinleşen makine, ağdaki istemcilerin etkinleşmesini yönetir.

NASIL ÇALIŞIR
  KMS ana bilgisayarı, bu anahtarla bir kez Microsoft'la etkinleşir. Sonra istemcilerden gelen
  etkinleştirme isteklerini (TCP 1688) yanıtlar ve DNS'te kendini SRV kaydıyla duyurur.

SINIRLAR
  Anahtarla sınırlı sayıda ana bilgisayar etkinleştirilebilir (anlaşmaya bağlıdır).

KİMLER İÇİN
  Yalnızca BT yöneticileri; ağda KMS sunucusu kuracak kişiler.

BU PROGRAMDA
  KMS Sunucusu menüsü: etkinleştirme / yenileme aralığı, dinleme portu, DNS yayınlama, öncelik.)LY" },

{ L"Active Directory Tabanlı Etkinleştirme (ADBA)",
LR"LY(ACTIVE DIRECTORY TABANLI ETKİNLEŞTİRME (ADBA)

NEDİR
  Windows Server 2012 ve sonrasında, bir Active Directory etki alanındaki bilgisayarların, ayrı bir
  KMS sunucusuna gerek kalmadan etki alanı üzerinden etkinleşmesidir. Etkinleştirme nesnesi
  Active Directory'de tutulur.

NASIL ÇALIŞIR
  Etki alanına katılan bilgisayarlar, etki alanı denetleyicisinden etkinleştirme bilgisini otomatik
  alır. Aynı KMS istemci anahtarları (GVLK) kullanılır ve süre 180 gündür; etki alanıyla bağlantı
  olduğu sürece yenilenir.

KİMLER İÇİN
  Etki alanı ortamına sahip kuruluşlar.

BU PROGRAMDA
  ADBA'ya özel komutlar (/ad-activation-*) bu sürümde YOKTUR. Etkinleştirme türü ayarı
  (/act-type 1) ile bu yöntem açılıp kapatılabilir.)LY" },

{ L"Token Tabanlı Etkinleştirme",
LR"LY(TOKEN TABANLI ETKİNLEŞTİRME (Token-based Activation)

NEDİR
  Windows 8.1 / Server 2012 R2 ile gelen, yüksek güvenlikli ortamlar için yöntemdir. Etkinleştirme,
  akıllı kartla (smart card) saklanan, kuruluşa ait bir sertifika ile yapılan bir "token" ile olur.
  Bilgisayarların internete veya KMS'e erişmesine gerek yoktur.

KİMLER İÇİN
  İnternete kapalı, ağdan yalıtılmış (isolated) kurumlar: savunma, laboratuvar, kapalı ağlar.

BU PROGRAMDA
  Token etkinleştirme komutları (/lil, /ril, /stao vb.) bu sürümde YOKTUR. /act-type 3 ile
  token türü açılıp kapatılabilir.)LY" },

{ L"Abonelik Etkinleştirme (Subscription)",
LR"LY(ABONELİK ETKİNLEŞTİRME (Windows Subscription Activation)

NEDİR
  Windows 10/11 Pro bilgisayarın, kuruluşun aboneliği (Microsoft 365 / Windows Enterprise E3, E5
  veya Education) ile otomatik olarak Enterprise / Education sürümüne yükselmesidir. Ürün anahtarı
  girilmez; kullanıcı kuruluş hesabıyla (Microsoft Entra ID) oturum açar.

SÜRE
  Abonelik süresince geçerlidir. Abonelik biterse bilgisayar Pro sürümüne döner; veri kaybı olmaz.

KİMLER İÇİN
  Microsoft 365 / Windows Enterprise aboneliği olan kuruluşlar.

BU PROGRAMDA
  Anahtar yönetimi gerekmez. Durum'da lisansın durumu görülür.)LY" },

{ L"Deneme Sürümü (Evaluation)",
LR"LY(DENEME SÜRÜMÜ (Evaluation)

NEDİR
  Windows Enterprise veya Server'ın sınırlı süreli ücretsiz sürümüdür. Test ve değerlendirme içindir.

SÜRE
  Genellikle 90 gün (Enterprise) veya 180 gün (Server). Süre, "Bitiş tarihini göster (/xpr)" ile
  görülür.

SÜRE BİTİNCE
  Windows bildirim verir, masaüstüne filigran koyar ve belirli aralıklarla kapanabilir. Veri
  silinmez; geçerli bir anahtar girip etkinleştirirseniz tam sürüme dönüşür (sürümler uyumluysa).

REARM
  Deneme süresi sınırlı sayıda uzatılabilir (rearm). Uzatma hakkı ve koşulları sürüme bağlıdır.

BU PROGRAMDA
  /xpr ile süreyi gör, /rearm ile uzat (hak varsa), ya da anahtar girip etkinleştir.)LY" },

{ L"LTSC / IoT / Gömülü (Embedded)",
LR"LY(LTSC — LONG-TERM SERVICING CHANNEL, IoT, GÖMÜLÜ SÜRÜMLER

NEDİR
  Windows 10/11 Enterprise LTSC ve IoT Enterprise LTSC, özellik güncellemesi almayan; yalnızca güvenlik
  ve kararlılık güncellemesi alan, uzun ömürlü sürümlerdir. Kiosk, tıbbi cihaz, endüstriyel
  makineler, ATM gibi değişmesi istenmeyen sistemler içindir.

ÖMÜR
  LTSC 2024: 5 yıl; IoT Enterprise LTSC: 10 yıl destek.

LİSANS TÜRLERİ
  - LTSC genellikle Toplu Lisanslama (MAK veya KMS) ile gelir.
  - IoT Enterprise çoğunlukla OEM kanalıyla (gömülü cihaz üreticisi üzerinden) satılır.

NOTLAR
  Microsoft Store, Cortana ve bazı varsayılan uygulamalar LTSC'de bulunmaz. Etkinleştirme ve yönetim
  diğer Enterprise sürümleriyle aynıdır.

BU PROGRAMDA
  Tüm komutlar LTSC'de aynı şekilde çalışır.)LY" },

{ L"Eğitim / Academic",
LR"LY(EĞİTİM (Education / Academic)

NEDİR
  Okul, üniversite ve eğitim kurumları için Windows Education / Pro Education sürümleridir. Eğitim
  fiyatlandırmasıyla Toplu Lisanslama (MAK/KMS) ya da abonelik etkinleştirmesiyle verilir.

ÖZELLİKLER
  Enterprise'ın birçok güvenlik ve yönetim özelliğini içerir; eğlence odaklı bazı özellikler kısıtlıdır.

ÖĞRENCİ / PERSONEL
  Bazı kurumlar öğrencilere dijital lisans (indirim ya da ücretsiz) sağlar; bunun koşulları kurumdan
  kuruma değişir.

BU PROGRAMDA
  Kurum KMS kullanıyorsa KMS İstemcisi menüsü, MAK kullanıyorsa Anahtar Yükle + Etkinleştir.)LY" },

{ L"ESU (Genişletilmiş Güvenlik Güncelleştirmeleri)",
LR"LY(ESU — EXTENDED SECURITY UPDATES

NEDİR
  Normal destek süresi biten bir Windows sürümü için, Microsoft'un sınırlı süreyle ücretli
  olarak sunduğu ek güvenlik güncelleştirmeleridir. Özellik güncellemesi veya teknik destek
  içermez; yalnızca kritik/önemli güvenlik düzeltmeleridir.

NASIL ETKİNLEŞİR
  ESU için ayrı bir lisans anahtarı (MAK türünde) yüklenir ve ayrı bir etkinleştirme kimliğiyle
  kaydolur. Windows'un normal lisansından bağımsızdır.

NASIL GÖRÜLÜR
  Lisans → Ayrıntılı lisans bilgisi (/dlv) komutunda kimlik alanına "all"
  yazın; ESU ayrı bir ürün olarak listelenir.

NOT
  ESU programının kapsamı, süresi ve fiyatı Microsoft tarafından belirlenir; güncel koşulları
  Microsoft'un resmî belgelerinden kontrol edin.)LY" },

{ L"Insider Preview (Önizleme)",
LR"LY(INSIDER PREVIEW (Windows Insider Programı)

NEDİR
  Yeni Windows özelliklerini erken test etmeniz için Microsoft'un önizleme derlemeleridir. Dev,
  Beta, Release Preview gibi kanalları vardır.

LİSANS
  Ayrı bir anahtar yoktur: halihazırda etkinleştirilmiş bir Windows'tan (genellikle bir
  dijital lisans) Insider derlemesine geçilir. Etkinleştirme önceki lisanstan devam eder.

DİKKAT
  Önizleme derlemeleri kararsız olabilir; üretim ortamında kullanılmaması önerilir.

BU PROGRAMDA
  Lisans durumu normal şekilde görülür. Etkinleştirme sorunu yaşarsanız Windows'u asıl lisanslı
  sürüme döndürmeniz gerekebilir.)LY" },

{ L"Genel / varsayılan kurulum anahtarları",
LR"LY(GENEL (GENERIC) KURULUM ANAHTARLARI

NEDİR
  Kurulum sırasında Windows sürümünü (Home, Pro, Enterprise...) seçebilmek için kullanılan,
  herkese açık ve etkinleştirme YAPMAYAN anahtarlardır. Microsoft bunları belgelerinde yayınlar.

NE İŞE YARAR
  Bir bilgisayarda kurulumu tamamlamak veya sürümü değiştirmek (örn. Pro → Enterprise) için
  kullanılır. Anahtar, kurulumdan sonra bilgisayarı etkinleştirmez; ayrı bir etkinleştirme gerekir
  (dijital lisans, MAK, KMS vb.).

DİKKAT
  Bu tür anahtarlarla etkinleşmiş gibi görünen bilgisayar, aslında KMS veya başka bir yöntemle
  etkinleşiyor olabilir. Etkinleştirmenin kaynağını Durum / Ayrıntılı ile doğrulayın.)LY" },

{ L"Lisans durumları ve tolerans süreleri",
LR"LY(LİSANS DURUMLARI

Programın "Lisans Durumu" satırında görebileceğiniz değerler:

  Lisanslı ....................... Etkinleşmiş ve geçerli.
  Lisanssız ...................... Geçerli lisans yok. Anahtar yükleyin / etkinleştirin.
  İlk tolerans süresi ............ Yeni kurulmuş Windows, henüz etkinleşmemiş (kısa süre).
  Ek tolerans süresi ............. Donanım değişikliği veya rearm sonrası ek süre.
  Orijinal olmayan tolerans ....... Windows'un orijinalliği doğrulanamıyor; süre dolmadan düzeltin.
  Bildirim ....................... Süre bitti. Windows sürekli etkinleştirme uyarısı verir, bazı
                                    özellikleri (kişiselleştirme gibi) kısıtlar.
  Uzatılmış tolerans süresi ...... Ek süre. Yalnızca özel durumlarda görülür.

TOLERANS SÜRESİ
  Süre dakika olarak gösterilir (örn. 259200 dk = 180 gün). KMS ve deneme sürümlerinde bu süre
  Bitiş tarihi (/xpr) olarak da görülür.

KALICI vs SÜRELİ
  Retail, OEM, MAK ve dijital lisanslar kalıcıdır. KMS / ADBA lisansları 180 günlüktür ve
  yenilenmelidir. Deneme sürümleri sabit bir süre sonunda sona erer.)LY" },
};

static const int kLicTypeCount = (int)(sizeof(kLicTypes) / sizeof(kLicTypes[0]));
