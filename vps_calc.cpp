// ZENVORA HOST - kalkulator harga VPS + pembuat link order WhatsApp (versi C++)
// Compile: g++ -std=c++17 -O2 -Wall -o vps_calc vps_calc.cpp
// Jalankan: ./vps_calc

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const string ADMIN_WA = "6289623600908";
const long BACKUP_PRICE = 10000;  // auto backup malam / 30 hari
const long RAM_RATE = 6000;       // per GB
const long CPU_RATE = 2600;       // per vCPU

struct Disk { string name; long rate; };
struct Package { string name, spec; long price; };

const vector<int> RAM_OPTIONS = {1, 2, 4, 8, 16, 32, 64};
const vector<int> CPU_OPTIONS = {1, 2, 4, 6, 8, 12, 16};
const vector<Disk> DISKS = {{"NVMe", 1700}, {"SSD", 1500}, {"HDD", 605}};
const vector<string> OS_LIST = {
    "Ubuntu 24.04 LTS", "Ubuntu 22.04 LTS", "Debian 13", "Debian 12", "AlmaLinux 9",
    "Rocky Linux 9", "Fedora Server", "openSUSE Leap", "Arch Linux"};
const vector<Package> PACKAGES = {
    {"VPS Starter",  "1 vCPU, RAM 1 GB, NVMe 20 GB",  20100},
    {"VPS Standard", "2 vCPU, RAM 4 GB, NVMe 60 GB",  129000},
    {"VPS Advanced", "4 vCPU, RAM 8 GB, NVMe 100 GB", 240000}};

// 20100 -> "Rp20.100"
string money(long n) {
    string s = to_string(n), out;
    int count = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        out.insert(out.begin(), s[i]);
        if (++count % 3 == 0 && i > 0) out.insert(out.begin(), '.');
    }
    return "Rp" + out;
}

// Setara encodeURIComponent di JavaScript
string urlEncode(const string &in) {
    static const char *hex = "0123456789ABCDEF";
    const string safe = "-_.!~*'()";
    string out;
    for (unsigned char c : in) {
        if (isalnum(c) || safe.find((char)c) != string::npos) {
            out += (char)c;
        } else {
            out += '%';
            out += hex[c >> 4];
            out += hex[c & 15];
        }
    }
    return out;
}

string waLink(const string &msg) {
    return "https://wa.me/" + ADMIN_WA + "?text=" + urlEncode(msg);
}

string join(const vector<string> &v, const string &sep) {
    string out;
    for (size_t i = 0; i < v.size(); ++i) out += (i ? sep : "") + v[i];
    return out;
}

// ---- input ----
string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string ask(const string &q) {
    cout << q << flush;
    string line;
    if (!getline(cin, line)) {
        cout << "\n(input berakhir)\n";
        exit(1);
    }
    return trim(line);
}

// Mengembalikan indeks (0-based) pilihan pengguna
int choose(const string &title, const vector<string> &labels) {
    cout << "\n" << title << "\n";
    for (size_t i = 0; i < labels.size(); ++i) cout << "  " << (i + 1) << ") " << labels[i] << "\n";
    for (;;) {
        long n = strtol(ask("Pilih nomor: ").c_str(), nullptr, 10);
        if (n >= 1 && n <= (long)labels.size()) return (int)n - 1;
        cout << "  Pilihan tidak valid.\n";
    }
}

bool yesNo(const string &q) {
    for (;;) {
        string a = ask(q + " (y/n): ");
        transform(a.begin(), a.end(), a.begin(), [](unsigned char c) { return (char)tolower(c); });
        if (a == "y" || a == "ya" || a == "yes") return true;
        if (a == "n" || a == "t" || a == "no" || a == "tidak") return false;
        cout << "  Jawab y atau n.\n";
    }
}

long askStorage() {
    for (;;) {
        string a = ask("\nPenyimpanan dalam GB (10-2000) [40]: ");
        if (a.empty()) return 40;
        char *end = nullptr;
        double n = strtod(a.c_str(), &end);
        if (end != a.c_str() && *end == '\0') {
            long s = (long)n;  // dibulatkan ke bawah (input sudah positif)
            if (n < 10) s = 10;
            return max(10L, min(2000L, s));
        }
        cout << "  Masukkan angka.\n";
    }
}

// ---- alur pesanan ----
string orderPackage() {
    vector<string> labels;
    for (const auto &p : PACKAGES) labels.push_back(p.name + " - " + p.spec + " - " + money(p.price) + " / 30 hari");
    const Package &p = PACKAGES[choose("Pilih paket:", labels)];
    bool backup = yesNo("\nTambah auto backup malam (+" + money(BACKUP_PRICE) + ")?");
    long total = p.price + (backup ? BACKUP_PRICE : 0);
    return join({"Halo admin ZENVORA HOST, saya ingin memesan " + p.name + " (" + p.spec + ").",
                 "",
                 "Harga paket: " + money(p.price),
                 "Auto backup malam: " + string(backup ? "Ya (+" + money(BACKUP_PRICE) + ")" : "Tidak"),
                 "Total: " + money(total) + " / 30 hari",
                 "",
                 "Mohon info stok dan cara pembayaran."},
                "\n");
}

string orderCustom() {
    vector<string> l;

    for (int x : RAM_OPTIONS) l.push_back(to_string(x) + " GB");
    int ram = RAM_OPTIONS[choose("RAM:", l)];

    l.clear();
    for (int x : CPU_OPTIONS) l.push_back(to_string(x) + " Core");
    int cpu = CPU_OPTIONS[choose("vCPU:", l)];

    l.clear();
    for (const auto &d : DISKS) l.push_back(d.name + " - " + money(d.rate) + "/GB");
    const Disk &disk = DISKS[choose("Jenis disk:", l)];

    long storage = askStorage();
    const string &os = OS_LIST[choose("Sistem operasi Linux:", OS_LIST)];
    bool backup = yesNo("\nTambah auto backup malam (+" + money(BACKUP_PRICE) + ")?");

    long ramCost = ram * RAM_RATE;
    long cpuCost = cpu * CPU_RATE;
    long diskCost = storage * disk.rate;
    long total = ramCost + cpuCost + diskCost + (backup ? BACKUP_PRICE : 0);

    return join({"Halo admin ZENVORA HOST, saya ingin memesan CUSTOM VPS.",
                 "",
                 "RAM: " + to_string(ram) + " GB (" + money(ramCost) + ")",
                 "vCPU: " + to_string(cpu) + " Core (" + money(cpuCost) + ")",
                 "Disk: " + disk.name,
                 "Storage: " + to_string(storage) + " GB (" + money(diskCost) + ")",
                 "OS: " + os,
                 "Auto backup malam: " + string(backup ? "Ya, setiap malam (" + money(BACKUP_PRICE) + ")" : "Tidak"),
                 "",
                 "Estimasi: " + money(total) + " / 30 hari",
                 "Mohon cek stok dan konfirmasi harga akhir. Saya akan membayar melalui WhatsApp setelah mendapat instruksi."},
                "\n");
}

int main() {
    cout << "=== ZENVORA HOST - Kalkulator VPS ===\n";
    int mode = choose("Mau pesan apa?", {"Paket siap pakai", "Custom VPS"});
    string msg = (mode == 0) ? orderPackage() : orderCustom();

    cout << "\n--- Pesan untuk admin ---\n" << msg << "\n";
    cout << "\n--- Link WhatsApp ---\n" << waLink(msg) << "\n";
    return 0;
}
