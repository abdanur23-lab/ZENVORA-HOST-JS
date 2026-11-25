#!/usr/bin/env node
// ZENVORA HOST - kalkulator harga VPS + pembuat link order WhatsApp (versi Node.js)
// Jalankan: node vps_calc.js

const readline = require("readline");

const ADMIN_WA = "6289623600908";
const BACKUP_PRICE = 10000; // auto backup malam / 30 hari
const RAM_RATE = 6000;      // per GB
const CPU_RATE = 2600;      // per vCPU

const RAM_OPTIONS = [1, 2, 4, 8, 16, 32, 64];
const CPU_OPTIONS = [1, 2, 4, 6, 8, 12, 16];
const DISKS = [
  { name: "NVMe", rate: 1700 },
  { name: "SSD", rate: 1500 },
  { name: "HDD", rate: 605 },
];
const OS_LIST = [
  "Ubuntu 24.04 LTS", "Ubuntu 22.04 LTS", "Debian 13", "Debian 12", "AlmaLinux 9",
  "Rocky Linux 9", "Fedora Server", "openSUSE Leap", "Arch Linux",
];
const PACKAGES = [
  { name: "VPS Starter",  spec: "1 vCPU, RAM 1 GB, NVMe 20 GB",  price: 20100 },
  { name: "VPS Standard", spec: "2 vCPU, RAM 4 GB, NVMe 60 GB",  price: 129000 },
  { name: "VPS Advanced", spec: "4 vCPU, RAM 8 GB, NVMe 100 GB", price: 240000 },
];

const money = (n) => "Rp" + String(Math.round(n)).replace(/\B(?=(\d{3})+(?!\d))/g, ".");
const waLink = (msg) => "https://wa.me/" + ADMIN_WA + "?text=" + encodeURIComponent(msg);

// ---- input (membaca baris satu per satu, aman untuk stdin pipe) ----
const rl = readline.createInterface({ input: process.stdin });
const lines = rl[Symbol.asyncIterator]();

async function ask(q) {
  process.stdout.write(q);
  const r = await lines.next();
  if (r.done) {
    console.log("\n(input berakhir)");
    process.exit(1);
  }
  return r.value.trim();
}

async function choose(title, labels) {
  console.log("\n" + title);
  labels.forEach((l, i) => console.log("  " + (i + 1) + ") " + l));
  for (;;) {
    const n = parseInt(await ask("Pilih nomor: "), 10);
    if (n >= 1 && n <= labels.length) return n - 1;
    console.log("  Pilihan tidak valid.");
  }
}

async function yesNo(q) {
  for (;;) {
    const a = (await ask(q + " (y/n): ")).toLowerCase();
    if (["y", "ya", "yes"].includes(a)) return true;
    if (["n", "t", "no", "tidak"].includes(a)) return false;
    console.log("  Jawab y atau n.");
  }
}

async function askStorage() {
  for (;;) {
    const a = await ask("\nPenyimpanan dalam GB (10-2000) [40]: ");
    if (a === "") return 40;
    const n = Number(a);
    if (Number.isFinite(n)) return Math.max(10, Math.min(2000, Math.floor(n)));
    console.log("  Masukkan angka.");
  }
}

// ---- alur pesanan ----
async function orderPackage() {
  const p = PACKAGES[await choose("Pilih paket:",
    PACKAGES.map((x) => x.name + " - " + x.spec + " - " + money(x.price) + " / 30 hari"))];
  const backup = await yesNo("\nTambah auto backup malam (+" + money(BACKUP_PRICE) + ")?");
  const total = p.price + (backup ? BACKUP_PRICE : 0);
  return [
    "Halo admin ZENVORA HOST, saya ingin memesan " + p.name + " (" + p.spec + ").",
    "",
    "Harga paket: " + money(p.price),
    "Auto backup malam: " + (backup ? "Ya (+" + money(BACKUP_PRICE) + ")" : "Tidak"),
    "Total: " + money(total) + " / 30 hari",
    "",
    "Mohon info stok dan cara pembayaran.",
  ].join("\n");
}

async function orderCustom() {
  const ram = RAM_OPTIONS[await choose("RAM:", RAM_OPTIONS.map((x) => x + " GB"))];
  const cpu = CPU_OPTIONS[await choose("vCPU:", CPU_OPTIONS.map((x) => x + " Core"))];
  const disk = DISKS[await choose("Jenis disk:", DISKS.map((d) => d.name + " - " + money(d.rate) + "/GB"))];
  const storage = await askStorage();
  const os = OS_LIST[await choose("Sistem operasi Linux:", OS_LIST)];
  const backup = await yesNo("\nTambah auto backup malam (+" + money(BACKUP_PRICE) + ")?");

  const ramCost = ram * RAM_RATE;
  const cpuCost = cpu * CPU_RATE;
  const diskCost = storage * disk.rate;
  const total = ramCost + cpuCost + diskCost + (backup ? BACKUP_PRICE : 0);

  return [
    "Halo admin ZENVORA HOST, saya ingin memesan CUSTOM VPS.",
    "",
    "RAM: " + ram + " GB (" + money(ramCost) + ")",
    "vCPU: " + cpu + " Core (" + money(cpuCost) + ")",
    "Disk: " + disk.name,
    "Storage: " + storage + " GB (" + money(diskCost) + ")",
    "OS: " + os,
    "Auto backup malam: " + (backup ? "Ya, setiap malam (" + money(BACKUP_PRICE) + ")" : "Tidak"),
    "",
    "Estimasi: " + money(total) + " / 30 hari",
    "Mohon cek stok dan konfirmasi harga akhir. Saya akan membayar melalui WhatsApp setelah mendapat instruksi.",
  ].join("\n");
}

(async () => {
  console.log("=== ZENVORA HOST - Kalkulator VPS ===");
  const mode = await choose("Mau pesan apa?", ["Paket siap pakai", "Custom VPS"]);
  const msg = mode === 0 ? await orderPackage() : await orderCustom();

  console.log("\n--- Pesan untuk admin ---\n" + msg);
  console.log("\n--- Link WhatsApp ---\n" + waLink(msg));
  rl.close();
})();
