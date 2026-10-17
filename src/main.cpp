// Windows Lisans Yöneticisi - slmgr.vbs işlevlerinin WMI üzerinden doğrudan C++ ile yeniden yazımı (menülü GUI),
// lisans deposu yedekleme / geri yükleme dahil.
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <wbemidl.h>
#include <comdef.h>
#include <cstdint>
#include <string>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

using std::wstring;

#include <algorithm>
#include <unordered_map>
#include "guide_tr.h"
#include "guide_en.h"

// ---------------------------------------------------------------- Dil / i18n
// Kaynak metinler Türkçedir; T() İngilizce seçiliyse lang_en.h tablosundan karşılığını döndürür
// (bulunamazsa Türkçe metin gösterilir).
enum { LANG_TR_ID = 0, LANG_EN_ID = 1 };
static int g_lang = LANG_TR_ID;

struct LangPair { const wchar_t* tr; const wchar_t* en; };
#include "strings_en.h"

static const wchar_t* T(const wchar_t* tr) {
    if (g_lang != LANG_EN_ID) return tr;
    static std::unordered_map<wstring, const wchar_t*> map;
    if (map.empty()) for (const LangPair& p : kEnPairs) map[p.tr] = p.en;
    auto it = map.find(tr);
    return it != map.end() ? it->second : tr;
}

static const wchar_t* GuideText() { return g_lang == LANG_EN_ID ? kGuideTextEn : kGuideText; }
static const LicType* LicTypes() { return g_lang == LANG_EN_ID ? kLicTypesEn : kLicTypes; }

static const wchar_t* kLangRegKey = L"Software\\WindowsLicenseManager";

static void LoadLanguage() {
    wchar_t b[16]; DWORD n = sizeof b;
    if (RegGetValueW(HKEY_CURRENT_USER, kLangRegKey, L"Language", RRF_RT_REG_SZ, nullptr, b, &n) == ERROR_SUCCESS) {
        if (wcscmp(b, L"en") == 0) { g_lang = LANG_EN_ID; return; }
        if (wcscmp(b, L"tr") == 0) { g_lang = LANG_TR_ID; return; }
    }
    // Kayıtlı seçim yok: Windows görüntüleme diline göre otomatik (Türkçe ise Türkçe, değilse İngilizce)
    g_lang = (PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_TURKISH) ? LANG_TR_ID : LANG_EN_ID;
}

static void SaveLanguage() {
    HKEY k;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kLangRegKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &k, nullptr) == ERROR_SUCCESS) {
        const wchar_t* v = g_lang == LANG_EN_ID ? L"en" : L"tr";
        RegSetValueExW(k, L"Language", 0, REG_SZ, (const BYTE*)v, (DWORD)((wcslen(v) + 1) * sizeof(wchar_t)));
        RegCloseKey(k);
    }
}

static const wchar_t* kWindowsAppId = L"55c92734-d682-4d71-983e-d6ec3f16059f";
static const wchar_t* kSppRegKey = L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform";

// Menü / düğme komut kimlikleri
enum Cmd {
    C_DLI = 1000, C_DLV, C_XPR, C_IPK, C_ATO, C_UPK, C_CPKY,
    C_SKMS, C_CKMS, C_SKMSD, C_CKMSD, C_SKHC, C_CKHC, C_ACTTYPE,
    C_SAI, C_SRI, C_SPRT, C_SDNS, C_CDNS, C_SPRI, C_CPRI,
    C_DTI, C_ATP,
    C_REARM, C_REARMAPP, C_REARMSKU,
    C_ILC, C_RILC,
    C_BACKUP, C_RESTORE,
    C_QDLI, C_QDLV, C_QATO, C_GUIDE, C_TYPES, C_LANG_TR, C_LANG_EN,
    C_CLEAR, C_COPY, C_EXIT, C_ABOUT
};

static HWND g_main, g_out;
static HFONT g_font, g_mono, g_title, g_small;
static HBRUSH g_bgBrush, g_outBrush, g_hdrBrush;
static IWbemServices* g_svc = nullptr;
static int g_dpi = 96;
static wstring g_channel;
static wstring g_statText;
static COLORREF g_statColor = RGB(156, 163, 175);

static const COLORREF kBg = RGB(243, 244, 246), kHdr = RGB(17, 24, 39), kAccent = RGB(59, 130, 246), kText = RGB(31, 41, 55);

static int S(int x) { return MulDiv(x, g_dpi, 96); }

// ---------------------------------------------------------------- Genel yardımcılar

static wstring Trim(wstring s) {
    size_t a = s.find_first_not_of(L" \t\r\n"), b = s.find_last_not_of(L" \t\r\n");
    return a == wstring::npos ? L"" : s.substr(a, b - a + 1);
}

static wstring Hex(HRESULT hr) {
    wchar_t b[32];
    wsprintfW(b, L"0x%08X", (unsigned)hr);
    wstring s = b;
    wchar_t* msg = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, (DWORD)hr, 0, (LPWSTR)&msg, 0, nullptr) && msg) {
        s += L" " + Trim(msg);
        LocalFree(msg);
    }
    return s;
}

static void Append(const wstring& s) {
    wstring t;
    for (wchar_t c : s) { if (c == L'\n') t += L'\r'; if (c != L'\r') t += c; }
    int len = GetWindowTextLengthW(g_out);
    SendMessageW(g_out, EM_SETSEL, len, len);
    SendMessageW(g_out, EM_REPLACESEL, FALSE, (LPARAM)t.c_str());
}

static void SetOut(const wstring& s) {
    SetWindowTextW(g_out, L"");
    Append(s);
}

static void ShowText(const wstring& s) {
    SetOut(s);
    SendMessageW(g_out, EM_SETSEL, 0, 0);
    SendMessageW(g_out, EM_SCROLLCARET, 0, 0);
}

static wstring Env(const wchar_t* name) {
    wchar_t b[MAX_PATH * 2];
    DWORD n = GetEnvironmentVariableW(name, b, MAX_PATH * 2);
    return n ? wstring(b, n) : L"";
}

static wstring Stamp() {
    SYSTEMTIME t; GetLocalTime(&t);
    wchar_t b[32];
    wsprintfW(b, L"%04d-%02d-%02d_%02d-%02d-%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return b;
}

// ---------------------------------------------------------------- WMI

static bool ConnectWmi(wstring& err) {
    if (g_svc) { g_svc->Release(); g_svc = nullptr; }
    IWbemLocator* loc = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&loc);
    if (FAILED(hr)) { err = Hex(hr); return false; }
    hr = loc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &g_svc);
    loc->Release();
    if (FAILED(hr)) { err = Hex(hr); return false; }
    hr = CoSetProxyBlanket(g_svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL,
                           RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hr)) { err = Hex(hr); return false; }
    return true;
}

static bool InitCom(wstring& err) {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) { err = Hex(hr); return false; }
    hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    if (FAILED(hr) && hr != RPC_E_TOO_LATE) { err = Hex(hr); return false; }
    return ConnectWmi(err);
}

typedef std::vector<IWbemClassObject*> ObjList;

static ObjList Query(const wstring& wql) {
    ObjList r;
    IEnumWbemClassObject* en = nullptr;
    HRESULT hr = g_svc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(wql.c_str()),
                                  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &en);
    if (FAILED(hr)) return r;
    for (;;) {
        IWbemClassObject* o = nullptr;
        ULONG n = 0;
        if (en->Next(WBEM_INFINITE, 1, &o, &n) != S_OK || !n) break;
        r.push_back(o);
    }
    en->Release();
    return r;
}

static wstring Str(IWbemClassObject* o, const wchar_t* prop) {
    VARIANT v; VariantInit(&v);
    wstring s;
    if (SUCCEEDED(o->Get(prop, 0, &v, nullptr, nullptr)) && v.vt != VT_NULL && v.vt != VT_EMPTY)
        if (SUCCEEDED(VariantChangeType(&v, &v, 0, VT_BSTR)) && v.bstrVal) s = v.bstrVal;
    VariantClear(&v);
    return s;
}

static long long Num(IWbemClassObject* o, const wchar_t* prop) {
    wstring s = Str(o, prop);
    return s.empty() ? 0 : _wtoi64(s.c_str());
}

static void ReleaseAll(ObjList& v) { for (auto* o : v) o->Release(); v.clear(); }

static IWbemClassObject* GetService() {
    ObjList v = Query(L"SELECT * FROM SoftwareLicensingService");
    if (v.empty()) return nullptr;
    for (size_t i = 1; i < v.size(); ++i) v[i]->Release();
    return v[0];
}

// actId boş: anahtarı yüklü Windows ürünleri; "all": anahtarı yüklü tüm ürünler; diğeri: o etkinleştirme kimliği.
static ObjList GetProducts(const wstring& actId) {
    wstring a = Trim(actId);
    if (a.empty())
        return Query(wstring(L"SELECT * FROM SoftwareLicensingProduct WHERE ApplicationId='") + kWindowsAppId +
                     L"' AND PartialProductKey IS NOT NULL");
    if (_wcsicmp(a.c_str(), L"all") == 0)
        return Query(L"SELECT * FROM SoftwareLicensingProduct WHERE PartialProductKey IS NOT NULL");
    return Query(L"SELECT * FROM SoftwareLicensingProduct WHERE ID='" + a + L"'");
}

struct Arg { const wchar_t* name; wstring val; };   // name == nullptr: yöntemin tek/ilk parametresi

static HRESULT CallMethod(IWbemClassObject* obj, const wchar_t* method, std::vector<Arg> args = {}) {
    wstring path = Str(obj, L"__PATH");
    IWbemClassObject* in = nullptr;
    if (!args.empty()) {
        wstring cls = Str(obj, L"__CLASS");
        IWbemClassObject* clsObj = nullptr;
        HRESULT hr = g_svc->GetObject(_bstr_t(cls.c_str()), 0, nullptr, &clsObj, nullptr);
        if (FAILED(hr)) return hr;
        IWbemClassObject* inDef = nullptr;
        hr = clsObj->GetMethod(method, 0, &inDef, nullptr);
        clsObj->Release();
        if (FAILED(hr) || !inDef) return FAILED(hr) ? hr : WBEM_E_INVALID_METHOD_PARAMETERS;

        struct P { wstring name; CIMTYPE type; };
        std::vector<P> props;
        inDef->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
        for (;;) {
            BSTR n = nullptr; CIMTYPE t = 0;
            if (inDef->Next(0, &n, nullptr, &t, nullptr) != WBEM_S_NO_ERROR) break;
            props.push_back({ n, t });
            SysFreeString(n);
        }
        inDef->EndEnumeration();
        hr = inDef->SpawnInstance(0, &in);
        inDef->Release();
        if (FAILED(hr)) return hr;

        for (auto& a : args) {
            const P* p = nullptr;
            for (auto& q : props) {
                if (a.name ? _wcsicmp(q.name.c_str(), a.name) == 0 : true) { p = &q; break; }
            }
            if (!p) { in->Release(); return WBEM_E_INVALID_METHOD_PARAMETERS; }
            VARIANT v; VariantInit(&v);
            switch (p->type & ~CIM_FLAG_ARRAY) {
            case CIM_BOOLEAN:
                v.vt = VT_BOOL;
                v.boolVal = (a.val == L"1" || _wcsicmp(a.val.c_str(), L"true") == 0) ? VARIANT_TRUE : VARIANT_FALSE;
                break;
            case CIM_SINT8: case CIM_UINT8: case CIM_SINT16: case CIM_UINT16: case CIM_SINT32: case CIM_UINT32:
                v.vt = VT_I4; v.lVal = _wtol(a.val.c_str());
                break;
            default:
                v.vt = VT_BSTR; v.bstrVal = SysAllocString(a.val.c_str());
            }
            in->Put(p->name.c_str(), 0, &v, 0);
            VariantClear(&v);
        }
    }
    IWbemClassObject* out = nullptr;
    HRESULT hr = g_svc->ExecMethod(_bstr_t(path.c_str()), _bstr_t(method), 0, nullptr, in, &out, nullptr);
    if (in) in->Release();
    if (out) out->Release();
    return hr;
}

static HRESULT Refresh() {
    IWbemClassObject* svc = GetService();
    if (!svc) return E_FAIL;
    HRESULT hr = CallMethod(svc, L"RefreshLicenseStatus");
    svc->Release();
    return hr;
}

// Etkinleştirme kimliği boş ise servis nesnesi, doluysa ilgili ürün (KMS istemci ayarları için).
static IWbemClassObject* GetKmsTarget(const wstring& actId, wstring& err) {
    wstring a = Trim(actId);
    if (a.empty()) {
        IWbemClassObject* s = GetService();
        if (!s) err = T(L"SoftwareLicensingService bulunamadı.");
        return s;
    }
    ObjList v = Query(L"SELECT * FROM SoftwareLicensingProduct WHERE ID='" + a + L"'");
    if (v.empty()) { err = T(L"Etkinleştirme kimliği bulunamadı: ") + a; return nullptr; }
    for (size_t i = 1; i < v.size(); ++i) v[i]->Release();
    return v[0];
}

static void Report(HRESULT hr, const wstring& okMsg) {
    Append(SUCCEEDED(hr) ? okMsg + L"\n" : T(L"Başarısız: ") + Hex(hr) + L"\n");
}

// ---------------------------------------------------------------- Bilgi gösterimi

static const wchar_t* StatusName(long long s) {
    switch (s) {
        case 0: return T(L"Lisanssız");
        case 1: return T(L"Lisanslı");
        case 2: return T(L"İlk tolerans süresi");
        case 3: return T(L"Ek tolerans süresi");
        case 4: return T(L"Orijinal olmayan tolerans süresi");
        case 5: return T(L"Bildirim");
        case 6: return T(L"Uzatılmış tolerans süresi");
    }
    return T(L"Bilinmiyor");
}

static bool ParseWmiDate(const wstring& d, SYSTEMTIME& st) {
    if (d.size() < 14) return false;
    auto n = [&](int o, int l) { return _wtoi(d.substr(o, l).c_str()); };
    ZeroMemory(&st, sizeof st);
    st.wYear = (WORD)n(0, 4); st.wMonth = (WORD)n(4, 2); st.wDay = (WORD)n(6, 2);
    st.wHour = (WORD)n(8, 2); st.wMinute = (WORD)n(10, 2); st.wSecond = (WORD)n(12, 2);
    return st.wYear >= 1601;
}

static wstring FmtDate(const SYSTEMTIME& st) {
    wchar_t b[64];
    wsprintfW(b, L"%02d.%02d.%04d %02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
    return b;
}

static wstring ExpiryText(IWbemClassObject* p, long long status) {
    long long grace = Num(p, L"GracePeriodRemaining");
    if (status == 1 && grace == 0) return T(L"Makine kalıcı olarak etkinleştirildi.");
    if (grace > 0) {
        FILETIME ft; GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER u; u.LowPart = ft.dwLowDateTime; u.HighPart = ft.dwHighDateTime;
        u.QuadPart += (ULONGLONG)grace * 60ULL * 10000000ULL;
        ft.dwLowDateTime = u.LowPart; ft.dwHighDateTime = u.HighPart;
        SYSTEMTIME utc, loc;
        FileTimeToSystemTime(&ft, &utc);
        SystemTimeToTzSpecificLocalTime(nullptr, &utc, &loc);
        return (status == 1 ? T(L"Etkinleştirme sona eriyor: ") : T(L"Tolerans süresi bitiyor: ")) + FmtDate(loc);
    }
    SYSTEMTIME st;
    if (ParseWmiDate(Str(p, L"EvaluationEndDate"), st) && st.wYear < 9999) return T(L"Deneme sürümü bitiyor: ") + FmtDate(st);
    return T(L"Lisanslı değil.");
}

static void ShowInfo(bool verbose, const wstring& actId) {
    wstring o;
    ObjList prods = GetProducts(actId);
    IWbemClassObject* svc = GetService();
    if (svc) o += T(L"Yazılım Lisans Servisi sürümü: ") + Str(svc, L"Version") + L"\n\n";
    if (prods.empty()) o += T(L"Eşleşen lisans bulunamadı.\n");
    for (auto* p : prods) {
        long long st = Num(p, L"LicenseStatus");
        o += T(L"Ad: ") + Str(p, L"Name") + L"\n";
        o += T(L"Açıklama: ") + Str(p, L"Description") + L"\n";
        o += T(L"Etkinleştirme Kimliği: ") + Str(p, L"ID") + L"\n";
        if (verbose) {
            o += T(L"Uygulama Kimliği: ") + Str(p, L"ApplicationId") + L"\n";
            o += T(L"Genişletilmiş PID: ") + Str(p, L"ProductKeyID2") + L"\n";
            o += T(L"Ürün Anahtarı Kanalı: ") + Str(p, L"ProductKeyChannel") + L"\n";
            o += T(L"Kurulum Kimliği (IID): ") + Str(p, L"OfflineInstallationId") + L"\n";
        }
        o += T(L"Kısmi Ürün Anahtarı: ") + Str(p, L"PartialProductKey") + L"\n";
        o += wstring(T(L"Lisans Durumu: ")) + StatusName(st) + L"\n";
        if (st != 1) {
            wchar_t b[32]; wsprintfW(b, L"0x%08X", (unsigned)Num(p, L"LicenseStatusReason"));
            o += wstring(T(L"Bildirim Nedeni: ")) + b + L"\n";
        }
        long long grace = Num(p, L"GracePeriodRemaining");
        if (grace > 0) {
            wchar_t b[128]; wsprintfW(b, T(L"%lld dakika (%lld gün)"), grace, grace / 1440);
            o += wstring(T(L"Kalan Tolerans: ")) + b + L"\n";
        }
        if (verbose) {
            wstring kms = Str(p, L"DiscoveredKeyManagementServiceMachineName");
            if (!kms.empty()) o += L"Bulunan KMS Sunucusu: " + kms + L":" + Str(p, L"DiscoveredKeyManagementServiceMachinePort") + L"\n";
            wstring ms = Str(p, L"KeyManagementServiceMachine");
            if (!ms.empty()) o += T(L"Yapılandırılmış KMS: ") + ms + L"\n";
        }
        o += L"\n";
    }
    if (svc && verbose) {
        wstring oem = Str(svc, L"OA3xOriginalProductKey");
        if (!oem.empty()) o += T(L"BIOS/OEM Ürün Anahtarı: ") + oem + L"\n";
        wstring cid = Str(svc, L"ClientMachineID");
        if (!cid.empty()) o += T(L"İstemci Makine Kimliği: ") + cid + L"\n";
    }
    if (svc) svc->Release();
    ReleaseAll(prods);
    ShowText(o);
}

// Üstteki başlık şeridindeki durum özeti
static void UpdateHeader() {
    ObjList prods = GetProducts(L"");
    IWbemClassObject* best = nullptr;
    for (auto* p : prods)
        if (!best || (Num(p, L"LicenseStatus") == 1 && Num(best, L"LicenseStatus") != 1)) best = p;
    if (best) {
        long long st = Num(best, L"LicenseStatus");
        wstring ch = Str(best, L"ProductKeyChannel");
        g_channel = ch;
        g_statText = Str(best, L"Name") + L"   •   " + StatusName(st) + (ch.empty() ? L"" : L"   •   " + ch);
        g_statColor = st == 1 ? RGB(34, 197, 94) : st == 0 ? RGB(239, 68, 68) : RGB(245, 158, 11);
    } else {
        g_channel.clear();
        g_statText = T(L"Anahtarı yüklü bir Windows lisansı bulunamadı");
        g_statColor = RGB(239, 68, 68);
    }
    ReleaseAll(prods);
    RECT r = { 0, 0, 5000, S(80) };
    InvalidateRect(g_main, &r, FALSE);
}

// ---------------------------------------------------------------- Giriş penceresi

struct PromptState {
    const wchar_t* label[2];
    wstring val[2];
    HWND edit[2];
    bool ok, done;
};

static LRESULT CALLBACK PromptProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    PromptState* s = (PromptState*)GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (msg) {
    case WM_CREATE: {
        s = (PromptState*)((CREATESTRUCTW*)lp)->lpCreateParams;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)s);
        int y = S(14);
        for (int i = 0; i < 2; ++i) {
            if (!s->label[i]) { s->edit[i] = nullptr; continue; }
            HWND l = CreateWindowW(L"STATIC", s->label[i], WS_CHILD | WS_VISIBLE, S(16), y, S(440), S(20), h, nullptr, nullptr, nullptr);
            SendMessageW(l, WM_SETFONT, (WPARAM)g_font, TRUE);
            s->edit[i] = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", s->val[i].c_str(),
                                         WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, S(16), y + S(24), S(440), S(28), h, nullptr, nullptr, nullptr);
            SendMessageW(s->edit[i], WM_SETFONT, (WPARAM)g_font, TRUE);
            y += S(62);
        }
        HWND ok = CreateWindowW(L"BUTTON", T(L"Tamam"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, S(262), y + S(2), S(92), S(30), h, (HMENU)IDOK, nullptr, nullptr);
        HWND ca = CreateWindowW(L"BUTTON", T(L"İptal"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, S(364), y + S(2), S(92), S(30), h, (HMENU)IDCANCEL, nullptr, nullptr);
        SendMessageW(ok, WM_SETFONT, (WPARAM)g_font, TRUE);
        SendMessageW(ca, WM_SETFONT, (WPARAM)g_font, TRUE);
        SetFocus(s->edit[0]);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK) {
            for (int i = 0; i < 2; ++i) if (s->edit[i]) {
                wchar_t b[2048]; GetWindowTextW(s->edit[i], b, 2048); s->val[i] = Trim(b);
            }
            s->ok = true; s->done = true;
        } else if (LOWORD(wp) == IDCANCEL) s->done = true;
        return 0;
    case WM_CLOSE: s->done = true; return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

// label2 nullptr ise tek alanlı. İptalde false döner.
static bool Prompt(const wchar_t* title, const wchar_t* label1, wstring& v1, const wchar_t* label2 = nullptr, wstring* v2 = nullptr) {
    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = PromptProc; wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = L"LYPrompt";
        RegisterClassW(&wc); reg = true;
    }
    PromptState s = {};
    s.label[0] = label1; s.label[1] = label2; s.val[0] = v1; if (v2) s.val[1] = *v2;
    RECT cr = { 0, 0, S(472), S(14) + S(62) * (label2 ? 2 : 1) + S(46) };
    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRectEx(&cr, style, FALSE, WS_EX_DLGMODALFRAME);
    RECT pr; GetWindowRect(g_main, &pr);
    HWND w = CreateWindowExW(WS_EX_DLGMODALFRAME, L"LYPrompt", title, style,
                             pr.left + S(60), pr.top + S(110), cr.right - cr.left, cr.bottom - cr.top, g_main, nullptr, GetModuleHandleW(nullptr), &s);
    EnableWindow(g_main, FALSE);
    ShowWindow(w, SW_SHOW);
    MSG m;
    while (!s.done && GetMessageW(&m, nullptr, 0, 0)) {
        if (!IsDialogMessageW(w, &m)) { TranslateMessage(&m); DispatchMessageW(&m); }
    }
    EnableWindow(g_main, TRUE);
    DestroyWindow(w);
    SetForegroundWindow(g_main);
    if (s.ok) { v1 = s.val[0]; if (v2) *v2 = s.val[1]; }
    return s.ok;
}

#define kActLabel T(T(L"Etkinleştirme Kimliği (isteğe bağlı, boş = varsayılan):"))

static bool AskActId(const wchar_t* title, wstring& id) { return Prompt(title, kActLabel, id); }

// ---------------------------------------------------------------- Lisans komutları

static void CmdXpr() {
    wstring id; if (!AskActId(T(L"Bitiş Tarihi (/xpr)"), id)) return;
    ObjList prods = GetProducts(id);
    wstring o;
    if (prods.empty()) o = T(L"Eşleşen lisans bulunamadı.\n");
    for (auto* p : prods) o += Str(p, L"Name") + L"\n  " + ExpiryText(p, Num(p, L"LicenseStatus")) + L"\n\n";
    ReleaseAll(prods);
    SetOut(o);
}

static void CmdIpk() {
    wstring key;
    if (!Prompt(T(L"Ürün Anahtarı Yükle (/ipk)"), T(L"Ürün anahtarı (XXXXX-XXXXX-XXXXX-XXXXX-XXXXX):"), key) || key.empty()) return;
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    HRESULT hr = CallMethod(svc, L"InstallProductKey", { { nullptr, key } });
    svc->Release();
    SetOut(L"");
    if (SUCCEEDED(hr)) Refresh();
    Report(hr, T(L"Ürün anahtarı başarıyla yüklendi."));
}

static void CmdAto(bool ask) {
    wstring id;
    if (ask && !AskActId(T(L"Etkinleştir (/ato)"), id)) return;
    ObjList prods = GetProducts(id);
    SetOut(prods.empty() ? T(L"Eşleşen lisans bulunamadı.\n") : L"");
    for (auto* p : prods) {
        Append(T(L"Etkinleştiriliyor: ") + Str(p, L"Name") + L"\n");
        HRESULT hr = CallMethod(p, L"Activate");
        Report(hr, T(L"  Başarılı."));
    }
    Refresh();
    ReleaseAll(prods);
}

static void CmdUpk() {
    wstring id; if (!AskActId(T(L"Anahtarı Kaldır (/upk)"), id)) return;
    if (MessageBoxW(g_main, T(L"Yüklü ürün anahtarı kaldırılacak. Devam edilsin mi?"), T(L"Onay"), MB_YESNO | MB_ICONWARNING) != IDYES) return;
    ObjList prods = GetProducts(id);
    SetOut(prods.empty() ? T(L"Eşleşen lisans bulunamadı.\n") : L"");
    for (auto* p : prods) {
        HRESULT hr = CallMethod(p, L"UninstallProductKey");
        Append(Str(p, L"Name") + L": ");
        Report(hr, T(L"ürün anahtarı kaldırıldı."));
    }
    Refresh();
    ReleaseAll(prods);
}

static void CmdCpky() {
    if (MessageBoxW(g_main, T(L"Ürün anahtarı kayıt defterinden silinecek (lisans kalır). Devam edilsin mi?"), T(L"Onay"), MB_YESNO | MB_ICONWARNING) != IDYES) return;
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    HRESULT hr = CallMethod(svc, L"ClearProductKeyFromRegistry");
    svc->Release();
    SetOut(L""); Report(hr, T(L"Ürün anahtarı kayıt defterinden temizlendi."));
}

static void CmdSkms() {
    wstring name, id;
    if (!Prompt(T(L"KMS Sunucusu Ayarla (/skms)"), T(L"KMS sunucusu (ad[:port]):"), name, kActLabel, &id) || name.empty()) return;
    wstring host = name, port;
    if (!name.empty() && name[0] == L'[') {                    // [IPv6]:port
        size_t e = name.find(L']');
        if (e != wstring::npos) { host = name.substr(1, e - 1); if (e + 2 <= name.size() && name[e + 1] == L':') port = name.substr(e + 2); }
    } else {
        size_t c = name.find(L':');
        if (c != wstring::npos && name.find(L':', c + 1) == wstring::npos) { host = name.substr(0, c); port = name.substr(c + 1); }
    }
    wstring err;
    IWbemClassObject* t = GetKmsTarget(id, err);
    if (!t) { SetOut(err); return; }
    HRESULT hr = CallMethod(t, L"SetKeyManagementServiceMachine", { { nullptr, host } });
    if (SUCCEEDED(hr)) hr = port.empty() ? CallMethod(t, L"ClearKeyManagementServicePort")
                                         : CallMethod(t, L"SetKeyManagementServicePort", { { nullptr, port } });
    t->Release();
    SetOut(L""); Report(hr, T(L"KMS makine adı ") + host + (port.empty() ? L"" : L":" + port) + T(L" olarak ayarlandı."));
}

static void CmdCkms() {
    wstring id; if (!AskActId(T(L"KMS Ayarını Temizle (/ckms)"), id)) return;
    wstring err;
    IWbemClassObject* t = GetKmsTarget(id, err);
    if (!t) { SetOut(err); return; }
    HRESULT hr = CallMethod(t, L"ClearKeyManagementServiceMachine");
    if (SUCCEEDED(hr)) hr = CallMethod(t, L"ClearKeyManagementServicePort");
    t->Release();
    SetOut(L""); Report(hr, T(L"KMS makine adı ve bağlantı noktası temizlendi."));
}

static void CmdSkmsDomain() {
    wstring fqdn, id;
    if (!Prompt(T(L"KMS Arama Etki Alanı (/skms-domain)"), T(L"Etki alanı (FQDN):"), fqdn, kActLabel, &id) || fqdn.empty()) return;
    wstring err;
    IWbemClassObject* t = GetKmsTarget(id, err);
    if (!t) { SetOut(err); return; }
    HRESULT hr = CallMethod(t, L"SetKeyManagementServiceLookupDomain", { { nullptr, fqdn } });
    t->Release();
    SetOut(L""); Report(hr, T(L"KMS arama etki alanı ") + fqdn + T(L" olarak ayarlandı."));
}

static void CmdCkmsDomain() {
    wstring id; if (!AskActId(T(L"KMS Arama Etki Alanını Temizle (/ckms-domain)"), id)) return;
    wstring err;
    IWbemClassObject* t = GetKmsTarget(id, err);
    if (!t) { SetOut(err); return; }
    HRESULT hr = CallMethod(t, L"ClearKeyManagementServiceLookupDomain");
    t->Release();
    SetOut(L""); Report(hr, T(L"KMS arama etki alanı temizlendi."));
}

static void ServiceBool(const wchar_t* method, bool v, const wchar_t* ok) {
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    HRESULT hr = CallMethod(svc, method, { { nullptr, v ? L"1" : L"0" } });
    svc->Release();
    SetOut(L""); Report(hr, ok);
}

static void ServiceNumber(const wchar_t* title, const wchar_t* label, const wchar_t* method, const wchar_t* ok) {
    wstring v;
    if (!Prompt(title, label, v) || v.empty()) return;
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    HRESULT hr = CallMethod(svc, method, { { nullptr, v } });
    svc->Release();
    SetOut(L""); Report(hr, wstring(ok) + L" " + v);
}

static void CmdActType() {
    wstring type, id;
    if (!Prompt(T(L"Etkinleştirme Türü (/act-type)"), T(L"Tür (0 = hepsi, 1 = AD, 2 = KMS, 3 = Token):"), type, kActLabel, &id) || type.empty()) return;
    int n = _wtoi(type.c_str());
    if (n < 0 || n > 3) { SetOut(T(L"Geçersiz değer: 0-3 arası olmalı.")); return; }
    wstring err;
    IWbemClassObject* t = GetKmsTarget(id, err);
    if (!t) { SetOut(err); return; }
    HRESULT hr = n ? CallMethod(t, L"SetVLActivationTypeEnabled", { { nullptr, type } })
                   : CallMethod(t, L"ClearVLActivationTypeEnabled");
    t->Release();
    SetOut(L""); Report(hr, T(L"Toplu lisans etkinleştirme türü ayarlandı."));
}

static void CmdDti() {
    wstring id; if (!AskActId(T(L"Kurulum Kimliği (/dti)"), id)) return;
    ObjList prods = GetProducts(id);
    wstring o;
    if (prods.empty()) o = T(L"Eşleşen lisans bulunamadı.\n");
    for (auto* p : prods) {
        o += Str(p, L"Name") + L"\n";
        o += T(L"  Etkinleştirme Kimliği: ") + Str(p, L"ID") + L"\n";
        o += T(L"  Kurulum Kimliği: ") + Str(p, L"OfflineInstallationId") + L"\n\n";
    }
    ReleaseAll(prods);
    SetOut(o);
}

static void CmdAtp() {
    wstring cid, id;
    if (!Prompt(T(L"Onay Kimliği Gir (/atp)"), T(L"Onay kimliği (Confirmation ID):"), cid, kActLabel, &id) || cid.empty()) return;
    ObjList prods = GetProducts(id);
    SetOut(prods.empty() ? T(L"Eşleşen lisans bulunamadı.\n") : L"");
    for (auto* p : prods) {
        Append(Str(p, L"Name") + L": ");
        HRESULT hr = CallMethod(p, L"DepositOfflineConfirmationId",
                                { { L"InstallationId", Str(p, L"OfflineInstallationId") }, { L"ConfirmationId", cid } });
        Report(hr, T(L"onay kimliği kabul edildi."));
    }
    Refresh();
    ReleaseAll(prods);
}

static void CmdRearm() {
    if (MessageBoxW(g_main, T(L"Windows için değerlendirme/tolerans süresi sıfırlanacak (rearm). Devam edilsin mi?"), T(L"Onay"), MB_YESNO | MB_ICONWARNING) != IDYES) return;
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    HRESULT hr = CallMethod(svc, L"ReArmWindows");
    svc->Release();
    SetOut(L""); Report(hr, T(L"Komut başarıyla tamamlandı. Değişikliğin etkili olması için bilgisayarı yeniden başlatın."));
}

static void CmdRearmApp() {
    wstring app = kWindowsAppId;
    if (!Prompt(T(L"Uygulama Rearm (/rearm-app)"), T(L"Uygulama kimliği:"), app) || app.empty()) return;
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    HRESULT hr = CallMethod(svc, L"ReArmApp", { { nullptr, app } });
    svc->Release();
    SetOut(L""); Report(hr, T(L"Komut başarıyla tamamlandı. Değişikliğin etkili olması için yeniden başlatın."));
}

static void CmdRearmSku() {
    wstring id;
    if (!Prompt(T(L"SKU Rearm (/rearm-sku)"), T(L"Etkinleştirme kimliği (SKU):"), id) || id.empty()) return;
    ObjList prods = GetProducts(id);
    SetOut(prods.empty() ? T(L"Eşleşen lisans bulunamadı.\n") : L"");
    for (auto* p : prods) Report(CallMethod(p, L"ReArmsku"), T(L"Komut başarıyla tamamlandı. Yeniden başlatın."));
    ReleaseAll(prods);
}

static wstring ReadTextFile(const wstring& path, bool& ok) {
    ok = false;
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return L"";
    DWORD sz = GetFileSize(f, nullptr), rd = 0;
    std::string buf(sz, '\0');
    BOOL r = ReadFile(f, &buf[0], sz, &rd, nullptr);
    CloseHandle(f);
    if (!r) return L"";
    ok = true;
    if (rd >= 2 && (uint8_t)buf[0] == 0xFF && (uint8_t)buf[1] == 0xFE)
        return wstring((const wchar_t*)(buf.data() + 2), (rd - 2) / 2);
    UINT cp = CP_ACP; size_t off = 0;
    if (rd >= 3 && (uint8_t)buf[0] == 0xEF && (uint8_t)buf[1] == 0xBB && (uint8_t)buf[2] == 0xBF) { cp = CP_UTF8; off = 3; }
    int n = MultiByteToWideChar(cp, 0, buf.data() + off, (int)(rd - off), nullptr, 0);
    wstring w(n, L'\0');
    MultiByteToWideChar(cp, 0, buf.data() + off, (int)(rd - off), &w[0], n);
    return w;
}

static bool WriteUtf16File(const wstring& path, const wstring& text) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD w; WORD bom = 0xFEFF;
    WriteFile(f, &bom, 2, &w, nullptr);
    BOOL ok = WriteFile(f, text.data(), (DWORD)(text.size() * 2), &w, nullptr);
    CloseHandle(f);
    return ok;
}

static HRESULT InstallLicenseFile(IWbemClassObject* svc, const wstring& path, wstring& err) {
    bool ok;
    wstring data = ReadTextFile(path, ok);
    if (!ok) { err = T(L"Dosya okunamadı."); return HRESULT_FROM_WIN32(GetLastError()); }
    return CallMethod(svc, L"InstallLicense", { { nullptr, data } });
}

static void CmdIlc() {
    wchar_t file[MAX_PATH] = L"";
    OPENFILENAMEW ofn = { sizeof ofn };
    ofn.hwndOwner = g_main;
    wstring filter = wstring(T(L"Lisans dosyaları (*.xrm-ms)")) + L'\0' + L"*.xrm-ms" + L'\0' + T(L"Tüm dosyalar") + L'\0' + L"*.*" + L'\0';
    ofn.lpstrFilter = filter.c_str();
    ofn.lpstrFile = file; ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return;
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    wstring err;
    HRESULT hr = InstallLicenseFile(svc, file, err);
    svc->Release();
    SetOut(L""); Report(hr, wstring(T(L"Lisans dosyası yüklendi: ")) + file);
}

static void InstallTree(IWbemClassObject* svc, const wstring& dir, int& okCount, int& failCount) {
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        wstring n = fd.cFileName;
        if (n == L"." || n == L"..") continue;
        wstring full = dir + L"\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) InstallTree(svc, full, okCount, failCount);
        else if (n.size() > 7 && _wcsicmp(n.c_str() + n.size() - 7, L".xrm-ms") == 0) {
            wstring err;
            if (SUCCEEDED(InstallLicenseFile(svc, full, err))) ++okCount; else ++failCount;
        }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

static void CmdRilc() {
    wstring root = Env(L"SystemRoot");
    IWbemClassObject* svc = GetService();
    if (!svc) { SetOut(T(L"SoftwareLicensingService bulunamadı.")); return; }
    SetOut(T(L"Sistem lisans dosyaları yeniden yükleniyor...\n"));
    int ok = 0, fail = 0;
    InstallTree(svc, root + L"\\system32\\spp\\tokens", ok, fail);
    InstallTree(svc, root + L"\\system32\\oem", ok, fail);
    svc->Release();
    wchar_t b[128]; wsprintfW(b, T(L"Tamamlandı. Başarılı: %d, başarısız: %d\n"), ok, fail);
    Append(b);
}

// ---------------------------------------------------------------- Yedekle / Geri yükle

static bool SvcWait(SC_HANDLE s, DWORD want, DWORD ms) {
    DWORD t0 = GetTickCount();
    for (;;) {
        SERVICE_STATUS_PROCESS ssp; DWORD need;
        if (!QueryServiceStatusEx(s, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof ssp, &need)) return false;
        if (ssp.dwCurrentState == want) return true;
        if (GetTickCount() - t0 > ms) return false;
        Sleep(200);
    }
}

static bool SvcControl(const wchar_t* name, bool start, wstring& err) {
    SC_HANDLE m = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!m) { err = Hex(HRESULT_FROM_WIN32(GetLastError())); return false; }
    SC_HANDLE s = OpenServiceW(m, name, SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!s) { err = Hex(HRESULT_FROM_WIN32(GetLastError())); CloseServiceHandle(m); return false; }
    bool ok;
    if (start) {
        ok = StartServiceW(s, 0, nullptr) || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING;
        if (ok) ok = SvcWait(s, SERVICE_RUNNING, 30000);
    } else {
        SERVICE_STATUS st;
        ok = ControlService(s, SERVICE_CONTROL_STOP, &st) || GetLastError() == ERROR_SERVICE_NOT_ACTIVE;
        if (ok) ok = SvcWait(s, SERVICE_STOPPED, 30000);
    }
    if (!ok && err.empty()) err = Hex(HRESULT_FROM_WIN32(GetLastError()));
    CloseServiceHandle(s); CloseServiceHandle(m);
    return ok;
}

static int RunProc(const wstring& cmd) {
    std::vector<wchar_t> buf(cmd.begin(), cmd.end()); buf.push_back(0);
    STARTUPINFOW si = { sizeof si }; PROCESS_INFORMATION pi;
    if (!CreateProcessW(nullptr, buf.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) return -1;
    WaitForSingleObject(pi.hProcess, 60000);
    DWORD code = 1; GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    return (int)code;
}

struct CopyResult { int files = 0, fail = 0; wstring firstErr; };

static void CopyTree(const wstring& src, const wstring& dst, CopyResult& r) {
    SHCreateDirectoryExW(nullptr, dst.c_str(), nullptr);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((src + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) { ++r.fail; if (r.firstErr.empty()) r.firstErr = src; return; }
    do {
        wstring n = fd.cFileName;
        if (n == L"." || n == L"..") continue;
        wstring s = src + L"\\" + n, d = dst + L"\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) CopyTree(s, d, r);
        else if (CopyFileW(s.c_str(), d.c_str(), FALSE)) ++r.files;
        else { ++r.fail; if (r.firstErr.empty()) r.firstErr = s; }
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

static bool Exists(const wstring& p) { return GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

static wstring RegString(HKEY root, const wchar_t* key, const wchar_t* val) {
    wchar_t b[256]; DWORD n = sizeof b;
    if (RegGetValueW(root, key, val, RRF_RT_REG_SZ, nullptr, b, &n) == ERROR_SUCCESS) return b;
    return L"";
}

static wstring InfoValue(const wstring& text, const wstring& key) {
    size_t p = 0;
    while (p < text.size()) {
        size_t e = text.find(L'\n', p);
        wstring line = Trim(text.substr(p, e == wstring::npos ? wstring::npos : e - p));
        if (line.compare(0, key.size() + 1, key + L"=") == 0) return line.substr(key.size() + 1);
        if (e == wstring::npos) break;
        p = e + 1;
    }
    return L"";
}

// Reads both the current backup format (info.txt, English keys) and the older one (bilgi.txt, Turkish keys).
static wstring ReadBackupInfo(const wstring& dir, bool& ok) {
    wstring t = ReadTextFile(dir + L"\\info.txt", ok);
    if (!ok) t = ReadTextFile(dir + L"\\bilgi.txt", ok);
    return t;
}

static wstring InfoGet(const wstring& info, const wchar_t* key, const wchar_t* legacyKey) {
    wstring v = InfoValue(info, key);
    if (v.empty() && legacyKey) v = InfoValue(info, legacyKey);
    return v;
}

static wstring StoreDir() { return Env(L"SystemRoot") + L"\\System32\\spp\\store"; }
static wstring ProfileDir() { return Env(L"SystemRoot") + L"\\ServiceProfiles\\NetworkService\\AppData\\Roaming\\Microsoft\\SoftwareProtectionPlatform"; }

static bool BackupTo(const wstring& dir) {
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);

    wchar_t cn[MAX_COMPUTERNAME_LENGTH + 1]; DWORD cnl = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameW(cn, &cnl);
    wstring cv = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    wstring info = L"MachineGuid=" + RegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid") + L"\r\n" +
                   L"Computer=" + cn + L"\r\n" +
                   L"Version=" + RegString(HKEY_LOCAL_MACHINE, cv.c_str(), L"ProductName") + L" (build " +
                   RegString(HKEY_LOCAL_MACHINE, cv.c_str(), L"CurrentBuild") + L")\r\n" +
                   L"Date=" + Stamp() + L"\r\n" +
                   L"Status=" + g_statText + L"\r\n";

    Append(T(L"Yazılım Koruma hizmeti (sppsvc) durduruluyor...\n"));
    wstring err;
    if (!SvcControl(L"sppsvc", false, err)) {
        Append(T(L"Hizmet durdurulamadı: ") + err + L"\n");
        SvcControl(L"sppsvc", true, err);
        return false;
    }
    CopyResult a, b;
    bool haveA = Exists(StoreDir()), haveB = Exists(ProfileDir());
    if (haveA) CopyTree(StoreDir(), dir + L"\\store", a);
    if (haveB) CopyTree(ProfileDir(), dir + L"\\nsprofile", b);
    int rc = RunProc(L"\"" + Env(L"SystemRoot") + L"\\System32\\reg.exe\" export \"" + kSppRegKey + L"\" \"" + dir + L"\\spp.reg\" /y");
    bool wrote = WriteUtf16File(dir + L"\\info.txt", info);

    wstring e2;
    bool started = SvcControl(L"sppsvc", true, e2);
    wchar_t m[256];
    wsprintfW(m, T(L"Lisans deposu: %d dosya, profil: %d dosya kopyalandı.\n"), a.files, b.files);
    Append(m);
    if (a.fail + b.fail) Append(T(L"Kopyalanamayan öğe var, ilki: ") + (a.firstErr.empty() ? b.firstErr : a.firstErr) + L"\n");
    if (!haveA) Append(T(L"Uyarı: lisans deposu klasörü bulunamadı.\n"));
    if (rc != 0) Append(T(L"Uyarı: kayıt defteri dışa aktarılamadı.\n"));
    if (!wrote) Append(T(L"Uyarı: bilgi dosyası yazılamadı.\n"));
    Append(started ? T(L"Hizmet yeniden başlatıldı.\n") : T(L"Uyarı: hizmet yeniden başlatılamadı: ") + e2 + L"\n");
    return haveA && a.fail + b.fail == 0 && rc == 0 && wrote && started;
}

static bool PickFolder(const wchar_t* title, wstring& out) {
    BROWSEINFOW bi = {};
    bi.hwndOwner = g_main; bi.lpszTitle = title;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return false;
    wchar_t p[MAX_PATH];
    bool ok = SHGetPathFromIDListW(pidl, p) != 0;
    CoTaskMemFree(pidl);
    if (ok) out = p;
    return ok;
}

static wstring ComputerName() {
    wchar_t cn[MAX_COMPUTERNAME_LENGTH + 1]; DWORD n = MAX_COMPUTERNAME_LENGTH + 1;
    return GetComputerNameW(cn, &n) ? wstring(cn, n) : wstring(L"PC");
}

static wstring MachineGuid() {
    return RegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid");
}

// ---- Yedek seçme penceresi

struct BackupEntry { wstring path, label; bool mine; };

struct PickState { std::vector<BackupEntry>* list; HWND lbl, lb, ok, cancel; int sel; bool done, accepted; };

static LRESULT CALLBACK PickProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    PickState* s = (PickState*)GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (msg) {
    case WM_CREATE: {
        s = (PickState*)((CREATESTRUCTW*)lp)->lpCreateParams;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)s);
        s->lbl = CreateWindowW(L"STATIC", T(L"Geri yüklenecek yedeği listeden seçin (en yeni üstte):"), WS_CHILD | WS_VISIBLE,
                               S(14), S(12), S(740), S(22), h, nullptr, nullptr, nullptr);
        SendMessageW(s->lbl, WM_SETFONT, (WPARAM)g_font, TRUE);
        s->lb = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                                S(14), S(40), S(732), S(300), h, (HMENU)1, nullptr, nullptr);
        SendMessageW(s->lb, WM_SETFONT, (WPARAM)g_mono, TRUE);
        SendMessageW(s->lb, LB_SETITEMHEIGHT, 0, S(26));
        for (auto& e : *s->list) SendMessageW(s->lb, LB_ADDSTRING, 0, (LPARAM)e.label.c_str());
        SendMessageW(s->lb, LB_SETHORIZONTALEXTENT, S(1200), 0);
        SendMessageW(s->lb, LB_SETCURSEL, 0, 0);
        s->ok = CreateWindowW(L"BUTTON", T(L"Tamam"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, S(552), S(352), S(92), S(30), h, (HMENU)IDOK, nullptr, nullptr);
        s->cancel = CreateWindowW(L"BUTTON", T(L"İptal"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, S(654), S(352), S(92), S(30), h, (HMENU)IDCANCEL, nullptr, nullptr);
        SendMessageW(s->ok, WM_SETFONT, (WPARAM)g_font, TRUE);
        SendMessageW(s->cancel, WM_SETFONT, (WPARAM)g_font, TRUE);
        SetFocus(s->lb);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK || (LOWORD(wp) == 1 && HIWORD(wp) == LBN_DBLCLK)) {
            int i = (int)SendMessageW(s->lb, LB_GETCURSEL, 0, 0);
            if (i >= 0) { s->sel = i; s->accepted = true; s->done = true; }
        } else if (LOWORD(wp) == IDCANCEL) s->done = true;
        return 0;
    case WM_CLOSE: s->done = true; return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

static bool PickBackup(std::vector<BackupEntry>& list, int& sel) {
    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = PickProc; wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = L"LYPick";
        RegisterClassW(&wc); reg = true;
    }
    PickState s = {};
    s.list = &list;
    RECT cr = { 0, 0, S(760), S(396) };
    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRectEx(&cr, style, FALSE, WS_EX_DLGMODALFRAME);
    RECT pr; GetWindowRect(g_main, &pr);
    HWND w = CreateWindowExW(WS_EX_DLGMODALFRAME, L"LYPick", T(L"Geri yüklenecek yedeği seçin"), style,
                             pr.left + S(60), pr.top + S(90), cr.right - cr.left, cr.bottom - cr.top, g_main, nullptr, GetModuleHandleW(nullptr), &s);
    EnableWindow(g_main, FALSE);
    ShowWindow(w, SW_SHOW);
    MSG m;
    while (!s.done && GetMessageW(&m, nullptr, 0, 0)) {
        if (!IsDialogMessageW(w, &m)) { TranslateMessage(&m); DispatchMessageW(&m); }
    }
    EnableWindow(g_main, TRUE);
    DestroyWindow(w);
    SetForegroundWindow(g_main);
    if (s.accepted) sel = s.sel;
    return s.accepted;
}

static std::vector<wstring> SubDirs(const wstring& dir) {
    std::vector<wstring> r;
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return r;
    do {
        wstring n = fd.cFileName;
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && n != L"." && n != L"..") r.push_back(n);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    return r;
}

static void AddIfBackup(const wstring& dir, std::vector<BackupEntry>& out) {
    bool ok;
    wstring info = ReadBackupInfo(dir, ok);
    if (!ok || !Exists(dir + L"\\store")) return;
    BackupEntry e;
    e.path = dir;
    e.mine = _wcsicmp(MachineGuid().c_str(), InfoValue(info, L"MachineGuid").c_str()) == 0;
    wstring comp = InfoGet(info, L"Computer", L"Bilgisayar");
    wstring leaf = dir.substr(dir.find_last_of(L'\\') + 1);
    while (comp.size() < 18) comp += L' ';
    e.label = wstring(e.mine ? L"* " : L"  ") + comp + L"  " + leaf + L"   " +
              (e.mine ? T(L"(bu bilgisayar)") : T(L"(başka bilgisayar)"));
    out.push_back(e);
}

// <kök>\<Bilgisayar Adı>\<Tarih> düzenindeki yedekleri (ve doğrudan seçilen yedek / bilgisayar klasörünü) bulur.
static std::vector<BackupEntry> CollectBackups(const wstring& root) {
    std::vector<BackupEntry> out;
    AddIfBackup(root, out);
    for (const wstring& a : SubDirs(root)) {
        AddIfBackup(root + L"\\" + a, out);
        for (const wstring& b : SubDirs(root + L"\\" + a)) AddIfBackup(root + L"\\" + a + L"\\" + b, out);
    }
    std::sort(out.begin(), out.end(), [](const BackupEntry& x, const BackupEntry& y) {
        if (x.mine != y.mine) return x.mine;
        return x.path > y.path;
    });
    return out;
}

static void CmdBackup() {
    wstring root;
    if (!PickFolder(T(L"Yedeğin kaydedileceği klasörü seçin"), root)) return;
    wstring dir = root + L"\\" + ComputerName() + L"\\" + Stamp();
    SetOut(T(L"Yedekleme hedefi: ") + dir + L"\n\n");
    bool ok = BackupTo(dir);
    wstring e;
    ConnectWmi(e);
    Refresh();
    UpdateHeader();
    Append(ok ? T(L"\nYedekleme başarıyla tamamlandı.\n") : T(L"\nYedekleme tamamlandı, ancak uyarılar var (yukarıya bakın).\n"));
}

static void CmdRestore() {
    wstring root;
    if (!PickFolder(T(L"Yedeklerin bulunduğu klasörü seçin (yedek alırken seçtiğiniz klasör)"), root)) return;
    std::vector<BackupEntry> found = CollectBackups(root);
    if (found.empty()) {
        MessageBoxW(g_main, T(L"Bu klasörde lisans yedeği bulunamadı.\n\nBeklenen düzen: <klasör>\\<Bilgisayar Adı>\\<Tarih>"),
                    T(L"Yedek bulunamadı"), MB_OK | MB_ICONERROR);
        return;
    }
    int sel = 0;
    if (!PickBackup(found, sel)) return;
    wstring dir = found[sel].path;
    bool okInfo;
    wstring info = ReadBackupInfo(dir, okInfo);
    wstring guid = MachineGuid();
    bool same = _wcsicmp(guid.c_str(), InfoValue(info, L"MachineGuid").c_str()) == 0;
    wstring msg = wstring(T(L"Yedek bilgileri:\n\n  Bilgisayar: ")) + InfoGet(info, L"Computer", L"Bilgisayar") + T(L"\n  Sürüm: ") + InfoGet(info, L"Version", L"Surum") +
                  T(L"\n  Tarih: ") + InfoGet(info, L"Date", L"Tarih") + T(L"\n  Durum: ") + InfoGet(info, L"Status", L"Durum") + L"\n\n";
    if (!same)
        msg += T(L"DİKKAT: Bu yedek BU bilgisayara ait görünmüyor (makine kimliği farklı). Lisans verisi donanıma bağlıdır; geri yükleme büyük olasılıkla çalışmaz ve mevcut lisansı bozabilir.\n\n");
    msg += T(L"Mevcut lisans deposunun üzerine yazılacak. Önce otomatik olarak mevcut hâl yedeklenecek. Devam edilsin mi?");
    if (MessageBoxW(g_main, msg.c_str(), T(L"Geri yüklemeyi onayla"), MB_YESNO | (same ? MB_ICONQUESTION : MB_ICONERROR | MB_DEFBUTTON2)) != IDYES) return;

    wstring pre = Env(L"ProgramData") + L"\\WindowsLicenseManager\\" + ComputerName() + L"\\" + Stamp() + L"_onceki";
    SetOut(T(L"Geri yükleme öncesi mevcut hâl yedekleniyor:\n") + pre + L"\n\n");
    if (!BackupTo(pre)) {
        if (MessageBoxW(g_main, T(L"Mevcut hâl yedeklenirken sorun oluştu. Yine de geri yüklemeye devam edilsin mi?"), T(L"Uyarı"),
                        MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) { Append(T(L"\nİptal edildi.\n")); return; }
    }

    Append(T(L"\nGeri yükleniyor...\nYazılım Koruma hizmeti durduruluyor...\n"));
    wstring err;
    if (!SvcControl(L"sppsvc", false, err)) {
        Append(T(L"Hizmet durdurulamadı: ") + err + L"\n");
        SvcControl(L"sppsvc", true, err);
        return;
    }
    CopyResult a, b;
    CopyTree(dir + L"\\store", StoreDir(), a);
    if (Exists(dir + L"\\nsprofile")) CopyTree(dir + L"\\nsprofile", ProfileDir(), b);
    int rc = Exists(dir + L"\\spp.reg") ? RunProc(L"\"" + Env(L"SystemRoot") + L"\\System32\\reg.exe\" import \"" + dir + L"\\spp.reg\"") : 0;
    wstring e2;
    bool started = SvcControl(L"sppsvc", true, e2);

    wchar_t m[256];
    wsprintfW(m, T(L"Depo: %d dosya, profil: %d dosya geri yüklendi.\n"), a.files, b.files);
    Append(m);
    if (a.fail + b.fail) Append(T(L"Geri yüklenemeyen öğe var, ilki: ") + (a.firstErr.empty() ? b.firstErr : a.firstErr) + L"\n");
    if (rc != 0) Append(T(L"Uyarı: kayıt defteri içe aktarılamadı.\n"));
    Append(started ? T(L"Hizmet yeniden başlatıldı.\n") : T(L"Uyarı: hizmet yeniden başlatılamadı: ") + e2 + L"\n");

    wstring e3;
    ConnectWmi(e3);
    Refresh();
    UpdateHeader();
    Append(T(L"\nGeri yükleme bitti. Güncel durum:\n\n"));
    ObjList prods = GetProducts(L"");
    for (auto* p : prods) {
        Append(Str(p, L"Name") + L"\n  " + StatusName(Num(p, L"LicenseStatus")) + L" — " + ExpiryText(p, Num(p, L"LicenseStatus")) + L"\n");
    }
    ReleaseAll(prods);
    Append(T(L"\nSorun olursa önceki hâle dönmek için şu yedeği geri yükleyin:\n") + pre + L"\n");
    Append(T(L"Değişikliğin tam etkili olması için bilgisayarı yeniden başlatmanız önerilir.\n"));
}

// ---------------------------------------------------------------- Lisans türleri penceresi

struct TypesState { HWND list, edit, close, lbl; bool done; };

static void SetEditText(HWND e, const wchar_t* t) {
    wstring c;
    for (const wchar_t* p = t; *p; ++p) { if (*p == L'\n') c += L'\r'; if (*p != L'\r') c += *p; }
    SetWindowTextW(e, c.c_str());
    SendMessageW(e, EM_SETSEL, 0, 0);
    SendMessageW(e, EM_SCROLLCARET, 0, 0);
}

static int ChannelIndex() {
    const wstring& c = g_channel;
    if (c.empty()) return -1;
    if (_wcsicmp(c.c_str(), L"Retail") == 0) return 1;
    if (c.compare(0, 3, L"OEM") == 0) return 2;
    if (_wcsicmp(c.c_str(), L"Volume:MAK") == 0) return 4;
    if (_wcsicmp(c.c_str(), L"Volume:GVLK") == 0) return 5;
    if (_wcsicmp(c.c_str(), L"Volume:CSVLK") == 0) return 6;
    return -1;
}

static LRESULT CALLBACK TypesProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    TypesState* s = (TypesState*)GetWindowLongPtrW(h, GWLP_USERDATA);
    switch (msg) {
    case WM_CREATE: {
        s = (TypesState*)((CREATESTRUCTW*)lp)->lpCreateParams;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)s);
        int mine = ChannelIndex();
        wstring info = g_channel.empty() ? T(L"Lisans kanalı tespit edilemedi. Soldaki listeden bir tür seçin.")
                                         : T(L"Sizin lisans kanalınız: ") + g_channel + (mine >= 0 ? T(L"  (listede işaretlendi)") : L"");
        s->lbl = CreateWindowW(L"STATIC", info.c_str(), WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, h, nullptr, nullptr, nullptr);
        SendMessageW(s->lbl, WM_SETFONT, (WPARAM)g_font, TRUE);
        s->list = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                                  0, 0, 10, 10, h, (HMENU)1, nullptr, nullptr);
        SendMessageW(s->list, WM_SETFONT, (WPARAM)g_font, TRUE);
        SendMessageW(s->list, LB_SETITEMHEIGHT, 0, S(26));
        for (int i = 0; i < kLicTypeCount; ++i) {
            wstring t = LicTypes()[i].title;
            if (i == mine) t += T(L"   ◄ sizin lisansınız");
            SendMessageW(s->list, LB_ADDSTRING, 0, (LPARAM)t.c_str());
        }
        s->edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                  0, 0, 10, 10, h, (HMENU)2, nullptr, nullptr);
        SendMessageW(s->edit, WM_SETFONT, (WPARAM)g_mono, TRUE);
        SendMessageW(s->edit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(S(14), S(14)));
        s->close = CreateWindowW(L"BUTTON", T(L"Kapat"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 0, 0, 10, 10, h, (HMENU)IDCANCEL, nullptr, nullptr);
        SendMessageW(s->close, WM_SETFONT, (WPARAM)g_font, TRUE);
        int sel = mine >= 0 ? mine : 0;
        SendMessageW(s->list, LB_SETCURSEL, sel, 0);
        SetEditText(s->edit, LicTypes()[sel].text);
        return 0;
    }
    case WM_SIZE: {
        if (!s) return 0;
        RECT rc; GetClientRect(h, &rc);
        int lw = S(320), top = S(40), bot = S(54);
        SetWindowPos(s->lbl, nullptr, S(14), S(10), rc.right - S(28), S(24), SWP_NOZORDER);
        SetWindowPos(s->list, nullptr, S(12), top, lw, rc.bottom - top - bot, SWP_NOZORDER);
        SetWindowPos(s->edit, nullptr, S(12) + lw + S(10), top, rc.right - lw - S(34), rc.bottom - top - bot, SWP_NOZORDER);
        SetWindowPos(s->close, nullptr, rc.right - S(112), rc.bottom - S(44), S(100), S(32), SWP_NOZORDER);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == 1 && HIWORD(wp) == LBN_SELCHANGE) {
            int i = (int)SendMessageW(s->list, LB_GETCURSEL, 0, 0);
            if (i >= 0 && i < kLicTypeCount) SetEditText(s->edit, LicTypes()[i].text);
        } else if (LOWORD(wp) == IDCANCEL) s->done = true;
        return 0;
    case WM_CTLCOLORSTATIC:
        if (s && (HWND)lp == s->edit) { SetTextColor((HDC)wp, kText); SetBkColor((HDC)wp, RGB(255, 255, 255)); return (LRESULT)g_outBrush; }
        SetBkMode((HDC)wp, TRANSPARENT);
        return (LRESULT)g_bgBrush;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lp)->ptMinTrackSize = { S(820), S(480) };
        return 0;
    case WM_CLOSE: if (s) s->done = true; return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

static void ShowTypes() {
    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = TypesProc; wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = g_bgBrush;
        wc.hIcon = LoadIconW(wc.hInstance, MAKEINTRESOURCEW(1));
        wc.lpszClassName = L"LYTypes";
        RegisterClassW(&wc); reg = true;
    }
    TypesState s = {};
    RECT pr; GetWindowRect(g_main, &pr);
    HWND w = CreateWindowExW(0, L"LYTypes", T(L"Lisans Türleri"), WS_OVERLAPPEDWINDOW,
                             pr.left + S(30), pr.top + S(30), S(1180), S(740), g_main, nullptr, GetModuleHandleW(nullptr), &s);
    EnableWindow(g_main, FALSE);
    ShowWindow(w, SW_SHOW);
    MSG m;
    while (!s.done && GetMessageW(&m, nullptr, 0, 0)) {
        if (!IsDialogMessageW(w, &m)) { TranslateMessage(&m); DispatchMessageW(&m); }
    }
    EnableWindow(g_main, TRUE);
    DestroyWindow(w);
    SetForegroundWindow(g_main);
}

// ---------------------------------------------------------------- Komut dağıtımı

static void CmdCopy() {
    SendMessageW(g_out, EM_SETSEL, 0, -1);
    SendMessageW(g_out, WM_COPY, 0, 0);
    SendMessageW(g_out, EM_SETSEL, -1, 0);
}

static void ApplyLanguage();

static void Dispatch(int id) {
    HCURSOR old = SetCursor(LoadCursor(nullptr, IDC_WAIT));
    wstring v;
    bool refreshHeader = true;
    switch (id) {
    case C_DLI: case C_DLV:
        if (Prompt(id == C_DLI ? T(L"Lisans Bilgisi (/dli)") : T(L"Ayrıntılı Lisans Bilgisi (/dlv)"),
                   T(L"Etkinleştirme Kimliği (boş = Windows, all = tümü):"), v))
            ShowInfo(id == C_DLV, v);
        refreshHeader = false;
        break;
    case C_QDLI: ShowInfo(false, L""); break;
    case C_QDLV: ShowInfo(true, L""); break;
    case C_QATO: CmdAto(false); break;
    case C_GUIDE: ShowText(GuideText()); refreshHeader = false; break;
    case C_TYPES: ShowTypes(); refreshHeader = false; break;
    case C_XPR: CmdXpr(); refreshHeader = false; break;
    case C_IPK: CmdIpk(); break;
    case C_ATO: CmdAto(true); break;
    case C_UPK: CmdUpk(); break;
    case C_CPKY: CmdCpky(); break;
    case C_SKMS: CmdSkms(); break;
    case C_CKMS: CmdCkms(); break;
    case C_SKMSD: CmdSkmsDomain(); break;
    case C_CKMSD: CmdCkmsDomain(); break;
    case C_SKHC: ServiceBool(L"DisableKeyManagementServiceHostCaching", false, T(L"KMS ana bilgisayar önbelleği etkinleştirildi.")); break;
    case C_CKHC: ServiceBool(L"DisableKeyManagementServiceHostCaching", true, T(L"KMS ana bilgisayar önbelleği devre dışı bırakıldı.")); break;
    case C_ACTTYPE: CmdActType(); break;
    case C_SAI: ServiceNumber(T(L"Etkinleştirme Aralığı (/sai)"), T(L"Aralık (dakika):"), L"SetVLActivationInterval", T(L"Etkinleştirme aralığı (dk):")); break;
    case C_SRI: ServiceNumber(T(L"Yenileme Aralığı (/sri)"), T(L"Aralık (dakika):"), L"SetVLRenewalInterval", T(L"Yenileme aralığı (dk):")); break;
    case C_SPRT: ServiceNumber(T(L"KMS Dinleme Portu (/sprt)"), T(L"Port numarası:"), L"SetKeyManagementServiceListeningPort", T(L"KMS dinleme portu:")); break;
    case C_SDNS: ServiceBool(L"DisableKeyManagementServiceDnsPublishing", false, T(L"KMS DNS yayınlama etkinleştirildi.")); break;
    case C_CDNS: ServiceBool(L"DisableKeyManagementServiceDnsPublishing", true, T(L"KMS DNS yayınlama devre dışı bırakıldı.")); break;
    case C_SPRI: ServiceBool(L"EnableKeyManagementServiceLowPriority", false, T(L"KMS önceliği normal olarak ayarlandı.")); break;
    case C_CPRI: ServiceBool(L"EnableKeyManagementServiceLowPriority", true, T(L"KMS önceliği düşük olarak ayarlandı.")); break;
    case C_DTI: CmdDti(); refreshHeader = false; break;
    case C_ATP: CmdAtp(); break;
    case C_REARM: CmdRearm(); break;
    case C_REARMAPP: CmdRearmApp(); break;
    case C_REARMSKU: CmdRearmSku(); break;
    case C_ILC: CmdIlc(); break;
    case C_RILC: CmdRilc(); break;
    case C_BACKUP: CmdBackup(); refreshHeader = false; break;
    case C_RESTORE: CmdRestore(); refreshHeader = false; break;
    case C_LANG_TR: case C_LANG_EN:
        g_lang = (id == C_LANG_EN) ? LANG_EN_ID : LANG_TR_ID;
        SaveLanguage();
        ApplyLanguage();
        refreshHeader = false;
        break;
    case C_CLEAR: SetWindowTextW(g_out, L""); refreshHeader = false; break;
    case C_COPY: CmdCopy(); refreshHeader = false; break;
    case C_EXIT: DestroyWindow(g_main); return;
    case C_ABOUT:
        MessageBoxW(g_main, T(L"Windows Lisans Yöneticisi\n\nslmgr.vbs işlevlerinin WMI üzerinden C++ ile yeniden yazımı; lisans deposu yedekleme ve geri yükleme dahil.\nKomutlar Windows Yazılım Lisans Hizmeti (SoftwareLicensingService / SoftwareLicensingProduct) ile çalışır.\n\nAçık kaynak (MIT lisansı)."),
                    T(L"Hakkında"), MB_OK | MB_ICONINFORMATION);
        refreshHeader = false;
        break;
    }
    SetCursor(old);
    if (refreshHeader) UpdateHeader();
}

// ---------------------------------------------------------------- Ana pencere

static HMENU BuildMenu() {
    HMENU bar = CreateMenu();
    auto popup = [&](const wchar_t* title) { HMENU m = CreatePopupMenu(); AppendMenuW(bar, MF_POPUP, (UINT_PTR)m, title); return m; };
    auto item = [](HMENU m, UINT id, const wchar_t* t) { AppendMenuW(m, MF_STRING, id, t); };
    auto sep = [](HMENU m) { AppendMenuW(m, MF_SEPARATOR, 0, nullptr); };

    HMENU f = popup(T(L"&Dosya"));
    item(f, C_COPY, T(L"Çıktıyı kopyala")); item(f, C_CLEAR, T(L"Çıktıyı temizle")); sep(f); item(f, C_EXIT, T(L"Çıkış"));

    HMENU l = popup(T(L"&Lisans"));
    item(l, C_DLI, T(L"Lisans bilgisini göster (/dli)..."));
    item(l, C_DLV, T(L"Ayrıntılı lisans bilgisi (/dlv)..."));
    item(l, C_XPR, T(L"Bitiş tarihini göster (/xpr)..."));
    sep(l);
    item(l, C_IPK, T(L"Ürün anahtarı yükle (/ipk)..."));
    item(l, C_ATO, T(L"Windows'u etkinleştir (/ato)..."));
    item(l, C_UPK, T(L"Ürün anahtarını kaldır (/upk)..."));
    item(l, C_CPKY, T(L"Anahtarı kayıt defterinden sil (/cpky)"));

    HMENU y = popup(T(L"&Yedekleme"));
    item(y, C_BACKUP, T(L"Lisansı yedekle..."));
    item(y, C_RESTORE, T(L"Yedekten geri yükle..."));

    HMENU k = popup(T(L"&KMS İstemcisi"));
    item(k, C_SKMS, T(L"KMS sunucusu ayarla (/skms)..."));
    item(k, C_CKMS, T(L"KMS sunucusu ayarını temizle (/ckms)..."));
    item(k, C_SKMSD, T(L"KMS arama etki alanı ayarla (/skms-domain)..."));
    item(k, C_CKMSD, T(L"KMS arama etki alanını temizle (/ckms-domain)..."));
    sep(k);
    item(k, C_SKHC, T(L"KMS ana bilgisayar önbelleğini etkinleştir (/skhc)"));
    item(k, C_CKHC, T(L"KMS ana bilgisayar önbelleğini devre dışı bırak (/ckhc)"));
    item(k, C_ACTTYPE, T(L"Etkinleştirme türü (/act-type)..."));

    HMENU s = popup(T(L"K&MS Sunucusu"));
    item(s, C_SAI, T(L"Etkinleştirme aralığı (/sai)..."));
    item(s, C_SRI, T(L"Yenileme aralığı (/sri)..."));
    item(s, C_SPRT, T(L"Dinleme portu (/sprt)..."));
    sep(s);
    item(s, C_SDNS, T(L"DNS yayınlamayı etkinleştir (/sdns)"));
    item(s, C_CDNS, T(L"DNS yayınlamayı devre dışı bırak (/cdns)"));
    item(s, C_SPRI, T(L"Normal öncelik (/spri)"));
    item(s, C_CPRI, T(L"Düşük öncelik (/cpri)"));

    HMENU o = popup(T(L"&Çevrimdışı"));
    item(o, C_DTI, T(L"Kurulum kimliğini göster (/dti)..."));
    item(o, C_ATP, T(L"Onay kimliği gir (/atp)..."));

    HMENU r = popup(T(L"&Rearm"));
    item(r, C_REARM, T(L"Windows rearm (/rearm)"));
    item(r, C_REARMAPP, T(L"Uygulama rearm (/rearm-app)..."));
    item(r, C_REARMSKU, T(L"SKU rearm (/rearm-sku)..."));

    HMENU d = popup(T(L"L&isans Dosyaları"));
    item(d, C_ILC, T(L"Lisans dosyası yükle (/ilc)..."));
    item(d, C_RILC, T(L"Sistem lisanslarını yeniden yükle (/rilc)"));

    HMENU lg = popup(L"&Dil / Language");
    AppendMenuW(lg, MF_STRING | (g_lang == LANG_TR_ID ? MF_CHECKED : 0), C_LANG_TR, L"Türkçe");
    AppendMenuW(lg, MF_STRING | (g_lang == LANG_EN_ID ? MF_CHECKED : 0), C_LANG_EN, L"English");

    HMENU h = popup(T(L"Y&ardım"));
    item(h, C_GUIDE, T(L"Kullanım kılavuzu (tüm özellikler)"));
    item(h, C_TYPES, T(L"Lisans türleri..."));
    sep(h);
    item(h, C_ABOUT, T(L"Hakkında"));
    return bar;
}

static const int kHdrH = 76, kBarH = 56, kBtnW = 118, kBtnH = 36;

static const struct { int id; const wchar_t* tr; } kButtons[] = {
    { C_QDLI, L"Durum" }, { C_QDLV, L"Ayrıntılı" }, { C_QATO, L"Etkinleştir" },
    { C_BACKUP, L"Yedekle" }, { C_RESTORE, L"Geri Yükle" },
    { C_GUIDE, L"Kılavuz" }, { C_TYPES, L"Lisans Türleri" } };

static void ApplyLanguage() {
    for (auto& b : kButtons) SetWindowTextW(GetDlgItem(g_main, b.id), T(b.tr));
    HMENU old = GetMenu(g_main);
    SetMenu(g_main, BuildMenu());
    if (old) DestroyMenu(old);
    DrawMenuBar(g_main);
    SetWindowTextW(g_main, T(L"Windows Lisans Yöneticisi"));
    InvalidateRect(g_main, nullptr, TRUE);
    ShowInfo(false, L"");
    UpdateHeader();
}

static void Layout(HWND h) {
    RECT rc; GetClientRect(h, &rc);
    int top = S(kHdrH) + S(kBarH);
    SetWindowPos(g_out, nullptr, 0, top, rc.right, rc.bottom - top, SWP_NOZORDER);
    int y = S(kHdrH) + (S(kBarH) - S(kBtnH)) / 2;
    int x2 = rc.right - S(16) - S(kBtnW + 20);
    HWND bt = GetDlgItem(h, C_TYPES), bg = GetDlgItem(h, C_GUIDE);
    if (bt) SetWindowPos(bt, nullptr, x2, y, S(kBtnW + 20), S(kBtnH), SWP_NOZORDER);
    if (bg) SetWindowPos(bg, nullptr, x2 - S(10) - S(kBtnW), y, S(kBtnW), S(kBtnH), SWP_NOZORDER);
}

static void PaintHeader(HWND h) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(h, &ps);
    RECT rc; GetClientRect(h, &rc);
    RECT hdr = { 0, 0, rc.right, S(kHdrH) };
    FillRect(dc, &hdr, g_hdrBrush);
    RECT line = { 0, S(kHdrH) - S(3), rc.right, S(kHdrH) };
    HBRUSH ab = CreateSolidBrush(kAccent); FillRect(dc, &line, ab); DeleteObject(ab);

    SetBkMode(dc, TRANSPARENT);
    HFONT old = (HFONT)SelectObject(dc, g_title);
    SetTextColor(dc, RGB(255, 255, 255));
    RECT t = { S(20), S(8), rc.right - S(20), S(42) };
    DrawTextW(dc, T(L"Windows Lisans Yöneticisi"), -1, &t, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    SelectObject(dc, g_small);
    int dy = S(51), r = S(5);
    HBRUSH db = CreateSolidBrush(g_statColor);
    HGDIOBJ ob = SelectObject(dc, db), op = SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, S(22), dy + S(4), S(22) + 2 * r, dy + S(4) + 2 * r);
    SelectObject(dc, ob); SelectObject(dc, op); DeleteObject(db);
    SetTextColor(dc, RGB(209, 213, 219));
    RECT st = { S(22) + 2 * r + S(8), dy - S(2), rc.right - S(20), dy + S(22) };
    DrawTextW(dc, g_statText.c_str(), -1, &st, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    SelectObject(dc, old);
    EndPaint(h, &ps);
}

static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        g_out = CreateWindowExW(0, L"EDIT", L"",
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                0, 0, 10, 10, h, nullptr, nullptr, nullptr);
        SendMessageW(g_out, WM_SETFONT, (WPARAM)g_mono, TRUE);
        SendMessageW(g_out, EM_SETLIMITTEXT, 0x7FFFFFFF, 0);
        SendMessageW(g_out, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(S(16), S(16)));
        int x = S(16), y = S(kHdrH) + (S(kBarH) - S(kBtnH)) / 2;
        for (auto& b : kButtons) {
            HWND bw = CreateWindowW(L"BUTTON", T(b.tr), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                    x, y, S(kBtnW), S(kBtnH), h, (HMENU)(INT_PTR)b.id, nullptr, nullptr);
            SendMessageW(bw, WM_SETFONT, (WPARAM)g_font, TRUE);
            x += S(kBtnW) + S(10);
        }
        SetMenu(h, BuildMenu());
        Layout(h);
        return 0;
    }
    case WM_SIZE: Layout(h); return 0;
    case WM_ERASEBKGND: {
        RECT rc; GetClientRect(h, &rc);
        FillRect((HDC)wp, &rc, g_bgBrush);
        return 1;
    }
    case WM_PAINT: PaintHeader(h); return 0;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
        if ((HWND)lp == g_out) {
            SetTextColor((HDC)wp, kText);
            SetBkColor((HDC)wp, RGB(255, 255, 255));
            return (LRESULT)g_outBrush;
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wp) >= C_DLI && LOWORD(wp) <= C_ABOUT) Dispatch(LOWORD(wp));
        return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lp)->ptMinTrackSize = { S(940), S(460) };
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR, int show) {
    LoadLanguage();
    HDC sdc = GetDC(nullptr);
    g_dpi = GetDeviceCaps(sdc, LOGPIXELSX);
    ReleaseDC(nullptr, sdc);
    wstring err;
    if (!InitCom(err)) {
        MessageBoxW(nullptr, (T(L"WMI bağlantısı kurulamadı: ") + err).c_str(), T(L"Hata"), MB_ICONERROR);
        return 1;
    }
    g_font = CreateFontW(-S(15), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_small = CreateFontW(-S(15), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_title = CreateFontW(-S(26), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_mono = CreateFontW(-S(15), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
    g_bgBrush = CreateSolidBrush(kBg);
    g_outBrush = CreateSolidBrush(RGB(255, 255, 255));
    g_hdrBrush = CreateSolidBrush(kHdr);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hi;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = (HICON)LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    wc.lpszClassName = L"LisansYonetici";
    RegisterClassW(&wc);
    g_main = CreateWindowW(wc.lpszClassName, T(L"Windows Lisans Yöneticisi"), WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, S(1040), S(700), nullptr, nullptr, hi, nullptr);
    SendMessageW(g_main, WM_SETICON, ICON_SMALL, (LPARAM)LoadImageW(hi, MAKEINTRESOURCEW(1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));
    ShowWindow(g_main, show);
    UpdateWindow(g_main);
    ShowInfo(false, L"");
    UpdateHeader();
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0)) {
        if (!IsDialogMessageW(g_main, &m)) { TranslateMessage(&m); DispatchMessageW(&m); }
    }
    if (g_svc) g_svc->Release();
    CoUninitialize();
    return 0;
}
