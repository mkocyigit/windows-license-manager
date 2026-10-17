// Windows License Manager - user guide and license types (English)
#pragma once

static const wchar_t* kGuideTextEn =
LR"LY(WINDOWS LICENSE MANAGER — USER GUIDE
================================================

This program does what Windows' built-in licensing tool slmgr.vbs does, but with a graphical
interface and written directly in C++. It works on every edition of Windows 10 and Windows 11
(Home, Pro, Enterprise, LTSC, Education). Commands are sent to the Windows Software Licensing
Service (through WMI: SoftwareLicensingService / SoftwareLicensingProduct). The program asks for
administrator rights every time it starts; licensing operations do not work without them.

LANGUAGE
  The language follows your Windows display language (Turkish or English) the first time. You can
  change it any time from the "Dil / Language" menu; the choice is remembered.

WHAT IS AN ACTIVATION ID?
  Every Windows edition / license has a GUID. If this box is left empty in most commands, the
  command applies to the primary Windows license. If you type "all" (only in the License
  Information / Detailed Information commands) every product with an installed key (including
  Office, etc.) is listed. Press the "Detailed" button to see the IDs.


1. QUICK BUTTONS (top of the window)
------------------------------------------------
Status ....... Shows the license name, description, activation ID, the last 5 characters of the
               key, the license status and the remaining grace period if any.
               (Same as slmgr /dli.) Changes nothing.
Detailed ..... The detailed form of Status: application ID, extended PID, key channel
               (Retail / OEM / Volume:MAK ...), offline installation ID, KMS info and the
               BIOS/OEM key. (slmgr /dlv)
Activate ..... Connects to Microsoft's activation servers with the installed key and activates
               Windows. Needs internet. (slmgr /ato)
Back Up ...... Backs up the license store to a folder you choose (see section 3).
Restore ...... Restores a backup taken earlier (see section 3).
Guide ........ This text.
License Types A detailed description of all license types (Retail, OEM, MAK, KMS, digital license
               etc.) in a separate window; your own license type is selected automatically.

The colored dot at the top of the window: green = licensed, yellow = in grace/notification period,
red = unlicensed. Next to it you see the edition name and the key channel.


2. LICENSE MENU
------------------------------------------------
Show license information (/dli) .... Same as Status; you may also type a specific activation ID or
                                     "all".
Detailed license information (/dlv)  Same as Detailed; accepts an ID or "all".
Show expiration date (/xpr) ........ Tells you when the activation ends. For permanent licenses
                                     (Retail, OEM, MAK) it says "permanently activated"; for KMS
                                     licenses it shows the 180-day end date; for evaluation
                                     editions it shows the evaluation end.
Install product key (/ipk) ......... Enters a new product key (XXXXX-XXXXX-XXXXX-XXXXX-XXXXX).
                                     The key is installed but not activated automatically; press
                                     "Activate" afterwards. It replaces the current key.
Activate Windows (/ato) ............ Tries to activate. With MAK/Retail keys it talks to Microsoft's
                                     servers, with KMS clients to the KMS server. The result
                                     (success / error code) is shown in the output.
Uninstall product key (/upk) ....... Removes the installed key. Windows becomes unlicensed.
                                     Asks for confirmation.
Clear key from registry (/cpky) .... Removes the plain-text key from the registry (a security
                                     measure). The license stays valid, activation is not
                                     affected. Asks for confirmation.


3. BACKUP MENU
------------------------------------------------
Back up license ... You choose a folder; the backup is created with this layout:
                        <chosen folder>\<Computer Name>\<Date>
                    (e.g. D:\Backup\X870E-APEX\2026-10-18_01-07-47). The following is copied into it:
                      - C:\Windows\System32\spp\store  (license store: data.dat, tokens.dat ...)
                      - the SoftwareProtectionPlatform folder in the NetworkService profile
                        (only if it exists on your Windows version)
                      - the SoftwareProtectionPlatform registry key (spp.reg)
                      - info.txt (computer name, machine ID, version, date, license status)
                    During copying the "Software Protection" service (sppsvc) is stopped for a few
                    seconds and then started again. The backup is not encrypted; store it safely.
Restore from backup  You choose the folder you used when backing up; the program finds the
                    <Computer Name>\<Date> backups below it and lists them (backups of this
                    computer and the newest first), and you pick one. The program first checks whether the
                    backup belongs to this computer (machine ID). Then it AUTOMATICALLY backs up
                    the current state to C:\ProgramData\WindowsLicenseManager\<Computer Name>\<Date>_onceki BEFORE the
                    restore, and then restores the files and the registry. If anything goes wrong
                    you can go back by restoring that previous backup.
                    This works on the same computer and the same Windows edition. License data is
                    tied to the hardware; it cannot be moved to another computer.


4. KMS CLIENT MENU  (for computers that activate through KMS in organizations)
------------------------------------------------
Set KMS server (/skms) ............ Sets the KMS server the computer will contact by hand:
                                     "server" or "server:port" (for IPv6 [address]:port). If no
                                     port is given the default (1688) is used.
Clear KMS server setting (/ckms) .. Removes the manually entered server; the computer goes back to
                                     finding it automatically via DNS.
Set KMS lookup domain (/skms-domain)
                                     Sets the domain in which the DNS SRV record of the KMS server
                                     is searched (e.g. company.local).
Clear KMS lookup domain (/ckms-domain)  Removes that setting.
Enable / Disable KMS host caching (/skhc, /ckhc)
                                     Decides whether the discovered KMS server is remembered. With
                                     caching on, the client does not search again every time; you
                                     may need to turn it off if the network changes.
Activation type (/act-type) ....... Limits which volume activation type the computer may use:
                                     0 = all, 1 = Active Directory, 2 = KMS, 3 = Token.


5. KMS SERVER MENU  (if the computer is set up as a KMS host)
------------------------------------------------
Activation interval (/sai) ........ How often (in minutes) a client retries when it could not
                                     activate (default 120).
Renewal interval (/sri) ........... How often (in minutes) activated clients renew their
                                     activation (default 10080 = 7 days).
Listening port (/sprt) ............ The TCP port KMS listens on (default 1688).
Enable / Disable DNS publishing (/sdns, /cdns)
                                     Decides whether the KMS server announces itself in DNS (SRV
                                     record).
Normal priority / Low priority (/spri, /cpri)
                                     Sets the CPU priority of the KMS service. Use low priority if
                                     the server does other work too.


6. OFFLINE ACTIVATION MENU  (machines without internet; phone activation)
------------------------------------------------
Show installation ID (/dti) ....... Shows the long computer-specific "Installation ID". You give
                                     this ID to Microsoft's activation phone line / activation page.
Enter confirmation ID (/atp) ...... You enter the "Confirmation ID" you received in return;
                                     Windows activates offline.
  Steps: (1) Show the installation ID, (2) get the confirmation ID from Microsoft,
  (3) enter the confirmation ID. If it succeeds, Status shows "Licensed".


7. REARM MENU
------------------------------------------------
Windows rearm (/rearm) ............ Resets the evaluation/grace period. It can only be done a
                                     limited number of times, and the remaining count decreases.
                                     A restart is needed afterwards. Asks for confirmation.
Application rearm (/rearm-app) .... Resets the period of a specific application (by application
                                     ID). The default ID belongs to Windows.
SKU rearm (/rearm-sku) ............ Resets the period of a specific license (activation ID).
  Rearm is not needed on a permanently activated (Retail / OEM / MAK) Windows.


8. LICENSE FILES MENU
------------------------------------------------
Install license file (/ilc) ...... Installs a .xrm-ms license file into the system. Usually used
                                     for extra license files supplied by Microsoft.
Reinstall system licenses (/rilc) . Reinstalls Windows' own license files (C:\Windows\System32\spp\
                                     tokens and \oem). Helps repair corrupted license data.


9. COMMON PROBLEMS
------------------------------------------------
0xC004F074  The KMS server could not be reached. Check the network, server name and port.
0xC004C003  The key is blocked / invalid. Verify the key.
0xC004F050  The product key is invalid. Check the key and the edition.
0xC004C008  The key's activation limit has been reached (common with MAK). Contact Microsoft
            Volume Licensing for more activations or activate by phone.
0x8007232B  No KMS record was found in DNS.
0x80070005  Access denied. Make sure the program runs as administrator.

NOTE: This program is only meant to manage licenses that you own or have the right to use.
Activation is done against Microsoft's own servers or your organization's KMS server; there is no
feature that redirects to third-party activation servers.
)LY";

// ----------------------------------------------------------------------------------------
static const LicType kLicTypesEn[] = {
{ L"What is a license type? (Getting started)",
LR"LY(LICENSE TYPE ≠ WINDOWS EDITION

The "edition" (Home, Pro, Enterprise, Education...) tells you which features you have. The
"license type" tells you HOW and FROM WHOM you obtained that edition, that is, the rights of
the license and how it is activated. The same edition (e.g. Pro) can come with a Retail, OEM,
MAK or KMS license; the type changes transferability, the way of activation and the validity
period.

To find out your own license type, press "Detailed" in the program and look at the "Product
Key Channel" line. In this window the type matching your channel is selected automatically.

CHANNEL NAMES (Product Key Channel):
  Retail ............ Retail
  OEM:DM / OEM:SLP / OEM:NONSLP ... OEM types
  Volume:MAK ........ Multiple Activation Key (MAK)
  Volume:GVLK ....... KMS client key
  Volume:CSVLK ...... KMS host key

GENERAL RULE: A license may only be used in line with Microsoft's license terms. The usage rights
of each type (how many computers, transferable or not, who may use it) are different.)LY" },

{ L"Retail (Full Packaged Product)",
LR"LY(RETAIL (Full Packaged Product)

WHAT IT IS
  An individual license bought as a box or digitally from a store, the Microsoft Store or a
  reseller. The key channel appears as "Retail".

HOW IT ACTIVATES
  The 25-character product key is entered and activated with Microsoft's servers (online or by
  phone). On Windows 10/11 the activation mostly becomes a "digital license" tied to the
  hardware ID (see Digital License).

TRANSFERABILITY
  It can be removed from one computer and moved to another; it may only be used on one computer at
  a time. It is the most flexible individual license type.

VALIDITY
  Permanent; no time limit. Receives updates for the whole support life of Windows.

WHO IT IS FOR
  Individual users who build their own computer, replace parts or want to move the license later.

IN THIS PROGRAM
  Install key → Activate. The channel appears as "Retail" in Status.)LY" },

{ L"OEM (license that comes with the computer)",
LR"LY(OEM — ORIGINAL EQUIPMENT MANUFACTURER

WHAT IT IS
  A license that a computer manufacturer (Dell, HP, Lenovo, ASUS, etc.) or a system builder sells
  together with the device. It is cheaper because it is tied to that device.

SUB-TYPES
  OEM:DM ....... "Digital Marker". The key is embedded in the motherboard's BIOS/UEFI (MSDM
                 table). Common since Windows 8. On a clean install the key is found
                 automatically without asking and Windows activates. The program shows the
                 BIOS/OEM key under "Detailed".
  OEM:SLP ...... "System Locked Preinstallation". A Windows 7-era method; activates offline using
                 the SLIC table in the BIOS and the manufacturer's certificate.
  OEM:NONSLP ... Keys given to OEMs that are not embedded in the manufacturer's BIOS; mostly
                 activate online.

TRANSFERABILITY
  According to the license terms it is tied to the device and cannot be moved to another
  computer. Replacing the motherboard may invalidate the license.

VALIDITY
  Permanent; limited to the life of the device.

WHO IT IS FOR
  People who buy a ready-made computer; users of branded laptops / desktops.

IN THIS PROGRAM
  Usually there is nothing to do; Windows activates by itself. You can see the key in the BIOS
  under "Detailed".)LY" },

{ L"Digital License (Entitlement / HWID)",
LR"LY(DIGITAL LICENSE (Digital Entitlement / HWID)

WHAT IT IS
  On Windows 10 and 11, an activation right kept on a Microsoft server and tied to the "fingerprint"
  of your hardware (Hardware ID) instead of a product key. You do not need to type a new product
  key.

HOW IT IS CREATED
  - After the free upgrade from Windows 7/8.1,
  - When Windows is bought from the Microsoft Store,
  - After activating with a Retail key (automatically),
  - From an activated Windows in the Insider Preview program.

ON A CLEAN INSTALL
  If you reinstall Windows on the same hardware and choose "I don't have a product key", it
  activates by itself once connected to the internet. You must choose the correct edition
  (Home/Pro).

MICROSOFT ACCOUNT
  If you link the license to your Microsoft account, after a major hardware change (especially the
  motherboard) you can move the license to the new hardware with Settings → Activation →
  "Troubleshoot" → "I changed hardware on this device recently".

IS THE KEY VISIBLE?
  No, only the last 5 characters are visible. That is why the backup feature backs up the license
  store, not a "key".)LY" },

{ L"Volume: MAK (Multiple Activation Key)",
LR"LY(VOLUME: MAK — MULTIPLE ACTIVATION KEY

WHAT IT IS
  A key specific to an organization, issued under a Volume Licensing agreement. A single key can
  activate a number of computers defined by Microsoft. Every activation counts against that number.

HOW IT ACTIVATES
  The key is installed and the computer connects once to Microsoft's activation servers (over the
  internet or by phone). After activation it is PERMANENT: no continuous connection, server or
  renewal is needed.
  On machines without internet: Offline menu → Show installation ID, Enter confirmation ID.

ACTIVATION COUNT
  How many times the key can be used depends on the agreement. When the count is used up you get
  error 0xC004C008; contact your Volume Licensing provider for more activations. A reinstall on the
  same computer may in some cases use another activation.

TRANSFERABILITY
  The key can be used on devices covered by the agreement; it belongs to the organization.

VALIDITY
  Permanent. There is no 180-day renewal like KMS.

WHO IT IS FOR
  Small/medium businesses and organizations that work independently of a network or have few
  computers. Backup/restore is especially useful on MAK computers.

IN THIS PROGRAM
  Install product key → Activate Windows. In "Detailed" the channel appears as "Volume:MAK".)LY" },

{ L"Volume: GVLK (KMS client)",
LR"LY(VOLUME: GVLK — GENERIC VOLUME LICENSE KEY (KMS CLIENT)

WHAT IT IS
  In Volume Licensing, the client key that lets a computer activate by connecting to a KMS server
  inside the organization. It usually comes pre-installed on Volume editions of Windows (Pro
  Volume, Enterprise, Education); you do not need to enter a key.

HOW IT WORKS
  The client finds the KMS server through a DNS "SRV" record (_vlmcs._tcp) on the network and
  activates. The KMS server is on the local network; Microsoft is not contacted.

DURATION
  The activation is valid for 180 days. By default the client renews every 7 days (default
  renewal interval 10080 min). If it cannot reach the network for 180 days, the license goes into
  notification mode.

THRESHOLD
  For a KMS server to activate clients it must see a minimum number of them: 25 computers for
  Windows clients, 5 for server operating systems.

WHO IT IS FOR
  Large organizations and schools with hundreds of computers in a domain network.

IN THIS PROGRAM
  KMS Client menu: set server, clear, lookup domain, caching, activation type. "Activate" contacts
  the server. You can see the end date with /xpr.)LY" },

{ L"Volume: CSVLK (KMS host key)",
LR"LY(VOLUME: CSVLK — CUSTOMER SPECIFIC VOLUME LICENSE KEY (KMS HOST)

WHAT IT IS
  The key used to set up a KMS server (host) in your organization. The machine activated with this
  key manages the activation of clients on the network.

HOW IT WORKS
  The KMS host activates once with Microsoft using this key. Then it answers activation requests
  from clients (TCP 1688) and announces itself in DNS with an SRV record.

LIMITS
  A limited number of hosts can be activated with the key (depends on the agreement).

WHO IT IS FOR
  IT administrators only; people who will set up a KMS server on the network.

IN THIS PROGRAM
  KMS Server menu: activation / renewal interval, listening port, DNS publishing, priority.)LY" },

{ L"Active Directory-Based Activation (ADBA)",
LR"LY(ACTIVE DIRECTORY-BASED ACTIVATION (ADBA)

WHAT IT IS
  Since Windows Server 2012, computers in an Active Directory domain can activate through the
  domain without a separate KMS server. The activation object is stored in Active Directory.

HOW IT WORKS
  Computers that join the domain get the activation information automatically from the domain
  controller. The same KMS client keys (GVLK) are used and the duration is 180 days; it is renewed
  as long as there is a connection to the domain.

WHO IT IS FOR
  Organizations with a domain environment.

IN THIS PROGRAM
  Commands specific to ADBA (/ad-activation-*) are NOT included in this version. With the
  activation type setting (/act-type 1) this method can be turned on or off.)LY" },

{ L"Token-based Activation",
LR"LY(TOKEN-BASED ACTIVATION

WHAT IT IS
  A method for high-security environments, introduced with Windows 8.1 / Server 2012 R2.
  Activation is done with a "token" created with an organization certificate stored on a smart card.
  The computers do not need access to the internet or to KMS.

WHO IT IS FOR
  Isolated organizations without internet access: defense, laboratories, closed networks.

IN THIS PROGRAM
  Token activation commands (/lil, /ril, /stao, etc.) are NOT included in this version. The token
  type can be turned on or off with /act-type 3.)LY" },

{ L"Subscription Activation",
LR"LY(WINDOWS SUBSCRIPTION ACTIVATION

WHAT IT IS
  A Windows 10/11 Pro computer automatically upgrades to the Enterprise / Education edition using
  the organization's subscription (Microsoft 365 / Windows Enterprise E3, E5 or Education). No
  product key is entered; the user signs in with the organization account (Microsoft Entra ID).

DURATION
  Valid for as long as the subscription lasts. When the subscription ends, the computer returns to
  Pro; no data is lost.

WHO IT IS FOR
  Organizations with a Microsoft 365 / Windows Enterprise subscription.

IN THIS PROGRAM
  No key management is needed. The license state appears in Status.)LY" },

{ L"Evaluation",
LR"LY(EVALUATION

WHAT IT IS
  A free, time-limited edition of Windows Enterprise or Server. It is for testing and evaluation.

DURATION
  Usually 90 days (Enterprise) or 180 days (Server). You can see the time with "Show expiration
  date (/xpr)".

WHEN IT ENDS
  Windows shows notifications, puts a watermark on the desktop and may shut down at intervals. No
  data is deleted; if you enter a valid key and activate, it converts to the full edition (if the
  editions are compatible).

REARM
  The evaluation period can be extended a limited number of times (rearm). The right to extend and
  its conditions depend on the edition.

IN THIS PROGRAM
  See the time with /xpr, extend it with /rearm (if you have the right), or enter a key and
  activate.)LY" },

{ L"LTSC / IoT / Embedded",
LR"LY(LTSC — LONG-TERM SERVICING CHANNEL, IoT, EMBEDDED EDITIONS

WHAT IT IS
  Windows 10/11 Enterprise LTSC and IoT Enterprise LTSC are long-lifecycle editions that do not
  receive feature updates; they receive only security and stability updates. They are for systems
  that should not change: kiosks, medical devices, industrial machines, ATMs.

LIFECYCLE
  LTSC 2024: 5 years; IoT Enterprise LTSC: 10 years of support.

LICENSE TYPES
  - LTSC generally comes with Volume Licensing (MAK or KMS).
  - IoT Enterprise is mostly sold through the OEM channel (via an embedded device manufacturer).

NOTES
  The Microsoft Store, Cortana and some default apps are not present on LTSC. Activation and
  management are the same as with other Enterprise editions.

IN THIS PROGRAM
  All commands work the same way on LTSC.)LY" },

{ L"Education / Academic",
LR"LY(EDUCATION / ACADEMIC

WHAT IT IS
  Windows Education / Pro Education editions for schools, universities and educational
  institutions. Supplied with education pricing through Volume Licensing (MAK/KMS) or subscription
  activation.

FEATURES
  It contains many of the security and management features of Enterprise; some entertainment-
  oriented features are restricted.

STUDENTS / STAFF
  Some institutions provide digital licenses (discounted or free) to students; the conditions
  vary from institution to institution.

IN THIS PROGRAM
  If your institution uses KMS, the KMS Client menu; if it uses MAK, Install Key + Activate.)LY" },

{ L"ESU (Extended Security Updates)",
LR"LY(ESU — EXTENDED SECURITY UPDATES

WHAT IT IS
  Paid additional security updates that Microsoft offers for a limited time for a Windows version
  whose normal support period has ended. They contain no feature updates or technical support, only
  critical/important security fixes.

HOW IT ACTIVATES
  A separate license key (of the MAK type) is installed for ESU and registers with a separate
  activation ID. It is independent of the normal Windows license.

HOW IT APPEARS
  In License → Detailed license information (/dlv), type "all" in the ID field; ESU is listed as a
  separate product.

NOTE
  The scope, duration and price of the ESU program are decided by Microsoft; check the current
  terms in Microsoft's official documentation.)LY" },

{ L"Insider Preview",
LR"LY(INSIDER PREVIEW (Windows Insider Program)

WHAT IT IS
  Microsoft's preview builds that let you test new Windows features early. There are channels such
  as Dev, Beta and Release Preview.

LICENSE
  There is no separate key: you move to an Insider build from an already activated Windows
  (usually with a digital license). Activation continues from the previous license.

CAUTION
  Preview builds can be unstable; they are not recommended for production use.

IN THIS PROGRAM
  The license state appears as normal. If you have activation problems you may need to return
  Windows to the original licensed edition.)LY" },

{ L"Generic / default installation keys",
LR"LY(GENERIC INSTALLATION KEYS

WHAT THEY ARE
  Public keys that do NOT activate and are used to be able to choose the Windows edition (Home,
  Pro, Enterprise...) during setup. Microsoft publishes them in its documentation.

WHAT THEY ARE USED FOR
  To finish the installation on a computer or to change the edition (e.g. Pro → Enterprise). The
  key does not activate the computer after installation; a separate activation is needed (digital
  license, MAK, KMS, etc.).

CAUTION
  A computer that seems to be activated with this kind of key may in fact be activated by KMS or
  another method. Verify the source of the activation with Status / Detailed.)LY" },

{ L"License statuses and grace periods",
LR"LY(LICENSE STATUSES

The values you may see on the "License Status" line of the program:

  Licensed ........................ Activated and valid.
  Unlicensed ...................... No valid license. Install a key / activate.
  Initial grace period ............ Newly installed Windows, not yet activated (short period).
  Additional grace period ......... Extra time after a hardware change or rearm.
  Non-genuine grace period ........ Windows' genuineness cannot be verified; fix it before time runs out.
  Notification .................... Time is up. Windows constantly shows activation warnings and
                                    restricts some features (such as personalization).
  Extended grace period ........... Extra time. Seen only in special cases.

GRACE PERIOD
  The time is shown in minutes (e.g. 259200 min = 180 days). For KMS and evaluation editions this
  time can also be seen as an expiration date (/xpr).

PERMANENT vs TIMED
  Retail, OEM, MAK and digital licenses are permanent. KMS / ADBA licenses last 180 days and must be
  renewed. Evaluation editions end after a fixed period.)LY" },
};
