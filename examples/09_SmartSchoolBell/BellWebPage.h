#ifndef BELL_WEBPAGE_H
#define BELL_WEBPAGE_H

#include <Arduino.h>

// Halaman Web UI Interaktif Disimpan di Flash Memory (PROGMEM)
const char BELL_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IskakINO Smart School Bell</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700&family=JetBrains+Mono:wght@400;600&display=swap" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/xlsx@0.18.5/dist/xlsx.full.min.js"></script>
    <style>
        :root {
            --bg-gradient: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
            --card-bg: rgba(30, 41, 59, 0.75);
            --card-border: rgba(255, 255, 255, 0.08);
            --accent-primary: #3b82f6;
            --accent-success: #10b981;
            --accent-danger: #ef4444;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --glass-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Outfit', sans-serif; }
        body { background: var(--bg-gradient); min-height: 100vh; color: var(--text-main); padding: 20px; display: flex; justify-content: center; }
        .container { width: 100%; max-width: 950px; display: flex; flex-direction: column; gap: 18px; }
        .card {
            background: var(--card-bg); backdrop-filter: blur(16px); -webkit-backdrop-filter: blur(16px);
            border: 1px solid var(--card-border); border-radius: 16px; padding: 22px; box-shadow: var(--glass-shadow);
        }
        .header-card {
            display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 14px;
            background: linear-gradient(135deg, rgba(59, 130, 246, 0.15), rgba(30, 41, 59, 0.8));
            border: 1px solid rgba(59, 130, 246, 0.3);
        }
        .title-group h1 {
            font-size: 22px; font-weight: 700; background: linear-gradient(90deg, #60a5fa, #a78bfa);
            -webkit-background-clip: text; -webkit-text-fill-color: transparent;
        }
        .badge-live {
            display: inline-flex; align-items: center; gap: 6px; padding: 4px 10px; border-radius: 20px;
            font-size: 12px; font-weight: 600; background: rgba(16, 185, 129, 0.15); color: #34d399;
            border: 1px solid rgba(16, 185, 129, 0.3);
        }
        .profile-tabs { display: flex; gap: 8px; flex-wrap: wrap; margin-top: 12px; }
        .profile-tab {
            padding: 8px 14px; border-radius: 10px; font-size: 13px; font-weight: 600; background: rgba(15, 23, 42, 0.6);
            color: var(--text-muted); border: 1px solid var(--card-border); cursor: pointer; transition: all 0.2s ease;
        }
        .profile-tab.active { background: #2563eb; color: white; border-color: #60a5fa; box-shadow: 0 4px 12px rgba(37, 99, 235, 0.4); }
        .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 14px; }
        .status-box { background: rgba(15, 23, 42, 0.6); border: 1px solid var(--card-border); border-radius: 12px; padding: 14px; display: flex; flex-direction: column; gap: 4px; }
        .status-label { font-size: 12px; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.5px; }
        .status-val { font-size: 19px; font-weight: 700; font-family: 'JetBrains Mono', monospace; color: #38bdf8; }
        .btn-group { display: flex; gap: 10px; flex-wrap: wrap; }
        .btn {
            display: inline-flex; align-items: center; justify-content: center; gap: 6px; padding: 9px 16px;
            font-size: 13px; font-weight: 600; border-radius: 8px; border: none; cursor: pointer; color: white; text-decoration: none;
        }
        .btn-primary { background: #2563eb; }
        .btn-success { background: #059669; }
        .btn-danger { background: #dc2626; }
        .btn-secondary { background: #334155; color: #e2e8f0; }
        .btn-sm { padding: 5px 10px; font-size: 12px; border-radius: 6px; }
        .excel-box {
            border: 2px dashed rgba(16, 185, 129, 0.4); border-radius: 12px; padding: 18px;
            background: rgba(16, 185, 129, 0.05); text-align: center; display: flex; flex-direction: column; align-items: center; gap: 10px;
        }
        .table-responsive { overflow-x: auto; margin-top: 12px; }
        table { width: 100%; border-collapse: collapse; text-align: left; }
        th { background: rgba(15, 23, 42, 0.8); color: var(--text-muted); font-size: 12px; text-transform: uppercase; padding: 10px 12px; border-bottom: 1px solid var(--card-border); }
        td { padding: 12px; font-size: 13px; border-bottom: 1px solid var(--card-border); }
        .time-badge { font-family: 'JetBrains Mono', monospace; font-size: 14px; font-weight: 700; color: #60a5fa; background: rgba(59, 130, 246, 0.1); padding: 3px 6px; border-radius: 4px; }
        .prof-tag { font-size: 11px; font-weight: 600; padding: 2px 6px; border-radius: 10px; text-transform: uppercase; }
        .prof-reguler { background: rgba(59, 130, 246, 0.2); color: #93c5fd; }
        .prof-jumat { background: rgba(16, 185, 129, 0.2); color: #6ee7b7; }
        .prof-ujian { background: rgba(245, 158, 11, 0.2); color: #fcd34d; }
        .prof-ramadhan { background: rgba(139, 92, 246, 0.2); color: #c4b5fd; }
        .form-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(170px, 1fr)); gap: 12px; margin-top: 12px; }
        .form-control { background: rgba(15, 23, 42, 0.8); border: 1px solid var(--card-border); border-radius: 6px; padding: 8px 12px; color: white; font-size: 13px; outline: none; width: 100%; }
        .toast { position: fixed; bottom: 20px; right: 20px; background: #10b981; color: white; padding: 10px 18px; border-radius: 8px; display: none; z-index: 100; font-weight: 600; font-size: 13px; }
    </style>
</head>
<body>
<div class="container">
    <div class="card header-card">
        <div class="title-group">
            <h1>🔔 IskakINO Smart School Bell</h1>
            <p style="color: var(--text-muted); font-size: 13px; margin-top: 2px;">Web Portal & Multi-Profile Scheduler</p>
        </div>
        <span class="badge-live">ONLINE &bull; NTP SYNCED</span>
    </div>

    <!-- Mode Profil Selector -->
    <div class="card">
        <div style="display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 6px;">
            <h3 style="font-size: 15px; font-weight: 600; color: #60a5fa;">📁 Mode & Profil Jadwal Aktif</h3>
            <span style="font-size: 13px; font-weight: 600; color: #34d399;" id="active-profile-badge">Profil: 📘 Reguler</span>
        </div>
        <div class="profile-tabs">
            <button class="profile-tab active" onclick="setProfile('auto')">🔄 Otomatis</button>
            <button class="profile-tab" onclick="setProfile('reguler')">📘 Reguler</button>
            <button class="profile-tab" onclick="setProfile('jumat')">🕌 Khusus Jumat</button>
            <button class="profile-tab" onclick="setProfile('ujian')">📝 Ujian / UTS</button>
            <button class="profile-tab" onclick="setProfile('ramadhan')">🌙 Ramadhan</button>
            <button class="profile-tab" onclick="setProfile('libur')">🏖️ Libur (Off)</button>
        </div>
    </div>

    <!-- Status Grid -->
    <div class="status-grid">
        <div class="status-box">
            <span class="status-label">Waktu Sistem (NTP)</span>
            <span class="status-val" id="clock-display">07:00:00 WIB</span>
            <span style="font-size: 12px; color: #a5b4fc;" id="date-display">Senin</span>
        </div>
        <div class="status-box">
            <span class="status-label">Status Bel & Relay</span>
            <span class="status-val" style="color: #34d399; font-size: 16px;" id="system-status">● Siaga (Standby)</span>
            <span style="font-size: 12px; color: var(--text-muted);" id="relay-status">Relay Ampli: OFF</span>
        </div>
        <div class="status-box">
            <span class="status-label">Jadwal Terdekat</span>
            <span class="status-val" style="color: #facc15; font-size: 15px;" id="next-bell-text">07:00 Masuk Pagi</span>
            <span style="font-size: 12px; color: var(--text-muted);" id="countdown-text">Akan berbunyi...</span>
        </div>
    </div>

    <!-- Panel Import / Export Excel & JSON -->
    <div class="card">
        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; flex-wrap: wrap; gap: 8px;">
            <div>
                <h3 style="font-size: 15px; font-weight: 600; color: #34d399;">📊 Backup, Ekspor & Impor Jadwal</h3>
                <p style="font-size: 12px; color: var(--text-muted);">Backup jadwal ke format JSON atau upload dari Microsoft Excel (.xlsx).</p>
            </div>
            <div class="btn-group">
                <button class="btn btn-secondary btn-sm" onclick="exportJsonSchedule()">📤 Export JSON</button>
                <button class="btn btn-secondary btn-sm" onclick="document.getElementById('json-file-input').click()">📥 Import JSON</button>
                <button class="btn btn-success btn-sm" onclick="downloadExcelTemplate()">📑 Template Excel</button>
            </div>
        </div>
        <input type="file" id="json-file-input" accept=".json" style="display: none;" onchange="handleJsonImport(event)">
        <div class="excel-box">
            <span style="font-size: 28px;">📑</span>
            <div>
                <b style="font-size: 14px;">Pilih atau Tarik File Excel (.xlsx) / JSON (.json)</b>
                <p style="font-size: 12px; color: var(--text-muted); margin-top: 2px;">Data akan diverifikasi dan disimpan permanen ke Flash Storage ESP32.</p>
            </div>
            <div style="display:flex; gap:10px; flex-wrap:wrap; justify-content:center;">
                <input type="file" id="excel-file-input" accept=".xlsx, .xls" style="display: none;" onchange="handleExcelImport(event)">
                <button class="btn btn-success btn-sm" onclick="document.getElementById('excel-file-input').click()">📂 Browse File Excel</button>
                <button class="btn btn-primary btn-sm" onclick="document.getElementById('json-file-input').click()">📂 Browse File JSON</button>
            </div>
        </div>
    </div>

    <!-- Kontrol Manual Cepat -->
    <div class="card">
        <h3 style="font-size: 15px; font-weight: 600; margin-bottom: 12px;">⚡ Kontrol Cepat</h3>
        <div class="btn-group">
            <button class="btn btn-danger" onclick="triggerBell(1, 'Bel Manual Web')">🚨 Bunyikan Bel Sekarang</button>
            <button class="btn btn-primary" onclick="testChimeSound(1)">🔊 Test Chime Audio</button>
        </div>
    </div>

    <!-- Tabel Jadwal -->
    <div class="card">
        <div style="display: flex; justify-content: space-between; align-items: center; flex-wrap: wrap; gap: 8px;">
            <h3 style="font-size: 15px; font-weight: 600;" id="table-title">📋 Tabel Jadwal Bel</h3>
            <span style="font-size: 12px; color: var(--text-muted);" id="total-bell-count">Total: 8 Jadwal</span>
        </div>
        <div class="table-responsive">
            <table>
                <thead>
                    <tr><th>No</th><th>Waktu</th><th>Kegiatan</th><th>Profil</th><th>Track</th><th>Hari</th><th>Aksi</th></tr>
                </thead>
                <tbody id="schedule-tbody"></tbody>
            </table>
        </div>
    </div>
</div>

<div class="toast" id="toast-msg">Jadwal berhasil diperbarui!</div>

<script>
    let activeProfileMode = 'auto';
    let schedules = [
        { id: 1, hour: 7, minute: 0, label: "Masuk Reguler", profile: "reguler", track: 1, days: "Senin-Jumat", enabled: true },
        { id: 2, hour: 9, minute: 45, label: "Istirahat 1", profile: "reguler", track: 2, days: "Senin-Jumat", enabled: true },
        { id: 3, hour: 10, minute: 15, label: "Masuk Kelas", profile: "reguler", track: 3, days: "Senin-Jumat", enabled: true },
        { id: 4, hour: 14, minute: 0, label: "Pulang Reguler", profile: "reguler", track: 4, days: "Senin-Jumat", enabled: true },
        { id: 5, hour: 7, minute: 0, label: "Masuk Jumat", profile: "jumat", track: 1, days: "Jumat", enabled: true },
        { id: 6, hour: 11, minute: 0, label: "Pulang Sholat Jumat", profile: "jumat", track: 4, days: "Jumat", enabled: true },
        { id: 7, hour: 7, minute: 30, label: "Ujian Sesi 1", profile: "ujian", track: 1, days: "Senin-Jumat", enabled: true },
        { id: 8, hour: 11, minute: 30, label: "Ujian Selesai", profile: "ujian", track: 4, days: "Senin-Jumat", enabled: true }
    ];

    let audioCtx = null;
    function showToast(msg) {
        const toast = document.getElementById('toast-msg');
        toast.innerText = msg; toast.style.display = 'block';
        setTimeout(() => { toast.style.display = 'none'; }, 3000);
    }
    function getEffectiveProfile() {
        if (activeProfileMode !== 'auto') return activeProfileMode;
        const day = new Date().getDay();
        if (day === 0) return 'libur';
        if (day === 5) return 'jumat';
        return 'reguler';
    }
    function setProfile(prof) {
        activeProfileMode = prof;
        document.querySelectorAll('.profile-tab').forEach(el => el.classList.remove('active'));
        event.target.classList.add('active');
        const eff = getEffectiveProfile();
        document.getElementById('active-profile-badge').innerText = `Profil: ${eff.toUpperCase()}`;
        renderTable();
        showToast(`Mode diganti ke: ${eff.toUpperCase()}`);
    }
    function playChimeNote(freq, startTime, duration) {
        if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        let osc = audioCtx.createOscillator(), gain = audioCtx.createGain();
        osc.type = 'sine'; osc.frequency.setValueAtTime(freq, startTime);
        gain.gain.setValueAtTime(0.3, startTime); gain.gain.exponentialRampToValueAtTime(0.0001, startTime + duration);
        osc.connect(gain); gain.connect(audioCtx.destination);
        osc.start(startTime); osc.stop(startTime + duration);
    }
    function testChimeSound(track = 1) {
        if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        let now = audioCtx.currentTime;
        playChimeNote(659.25, now, 0.6); playChimeNote(523.25, now + 0.5, 0.6);
        playChimeNote(587.33, now + 1.0, 0.6); playChimeNote(392.00, now + 1.5, 1.2);
    }
    function triggerBell(track, label) {
        testChimeSound(track);
        fetch(`/bell/trigger?track=${track}`).catch(()=>{});
        showToast(`Bel dipicu: ${label}`);
    }
    function renderTable() {
        const tbody = document.getElementById('schedule-tbody');
        tbody.innerHTML = '';
        const eff = getEffectiveProfile();
        let filtered = schedules.filter(s => s.profile === eff || s.profile === 'all');
        filtered.sort((a,b) => (a.hour*60+a.minute) - (b.hour*60+b.minute));
        filtered.forEach((s, idx) => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td>${idx + 1}</td>
                <td><span class="time-badge">${String(s.hour).padStart(2,'0')}:${String(s.minute).padStart(2,'0')}</span></td>
                <td><b>${s.label}</b></td>
                <td><span class="prof-tag prof-${s.profile}">${s.profile}</span></td>
                <td>Track 0${s.track}</td>
                <td><span style="color:#94a3b8;font-size:12px;">${s.days}</span></td>
                <td><button class="btn btn-primary btn-sm" onclick="triggerBell(${s.track}, '${s.label}')">▶ Test</button></td>
            `;
            tbody.appendChild(tr);
        });
        document.getElementById('total-bell-count').innerText = `Menampilkan ${filtered.length} dari ${schedules.length} jadwal`;
    }
    function exportJsonSchedule() {
        location.href = '/bell/export_json';
        showToast("📤 Mengunduh backup JSON jadwal...");
    }
    function handleJsonImport(event) {
        const file = event.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = function(e) {
            try {
                const text = e.target.result;
                const parsed = JSON.parse(text);
                if (parsed && parsed.schedules) {
                    fetch('/bell/import_json', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: text
                    }).then(r => r.json()).then(res => {
                        showToast(`✅ Berhasil import ${res.count || parsed.schedules.length} jadwal JSON!`);
                        setTimeout(() => location.reload(), 1500);
                    }).catch(err => {
                        showToast("Gagal menyimpan ke ESP32: " + err.message);
                    });
                } else {
                    alert("Format JSON tidak valid (harus memuat array 'schedules')");
                }
            } catch (err) {
                alert("Gagal membaca file JSON: " + err.message);
            }
        };
        reader.readAsText(file);
        event.target.value = '';
    }
    function downloadExcelTemplate() {
        const data = schedules.map(s => ({ "Jam": `${String(s.hour).padStart(2,'0')}:${String(s.minute).padStart(2,'0')}`, "Kegiatan": s.label, "Profil": s.profile, "Track_MP3": s.track, "Hari": s.days }));
        const ws = XLSX.utils.json_to_sheet(data), wb = XLSX.utils.book_new();
        XLSX.utils.book_append_sheet(wb, ws, "Jadwal_Bel");
        XLSX.writeFile(wb, "Template_Jadwal_Bel_Sekolah.xlsx");
        showToast("📥 Template Excel didownload!");
    }
    function exportToExcel() {
        downloadExcelTemplate();
    }
    function handleExcelImport(event) {
        const file = event.target.files[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = function(e) {
            try {
                const data = new Uint8Array(e.target.result);
                const workbook = XLSX.read(data, { type: 'array' });
                const jsonData = XLSX.utils.sheet_to_json(workbook.Sheets[workbook.SheetNames[0]]);
                let newSchedules = [];
                jsonData.forEach((row, idx) => {
                    let parts = String(row["Jam"] || "07:00").split(':');
                    newSchedules.push({
                        id: Date.now() + idx,
                        hour: parseInt(parts[0]) || 0,
                        minute: parseInt(parts[1]) || 0,
                        label: row["Kegiatan"] || `Jadwal ${idx+1}`,
                        profile: (row["Profil"] || "reguler").toLowerCase(),
                        track: parseInt(row["Track_MP3"] || 1),
                        days: row["Hari"] || "Senin-Jumat",
                        enabled: true
                    });
                });
                schedules = newSchedules;
                renderTable();
                showToast(`✅ Berhasil import ${jsonData.length} jadwal Excel!`);
            } catch (err) { alert("Gagal membaca Excel: " + err.message); }
        };
        reader.readAsArrayBuffer(file);
        event.target.value = '';
    }
    function updateClock() {
        const now = new Date();
        const days = ['Minggu', 'Senin', 'Selasa', 'Rabu', 'Kamis', 'Jumat', 'Sabtu'];
        document.getElementById('clock-display').innerText = `${String(now.getHours()).padStart(2,'0')}:${String(now.getMinutes()).padStart(2,'0')}:${String(now.getSeconds()).padStart(2,'0')} WIB`;
        document.getElementById('date-display').innerText = days[now.getDay()];
    }
    setInterval(updateClock, 1000);
    renderTable();
    updateClock();
</script>
</body>
</html>
)rawliteral";

#endif // BELL_WEBPAGE_H
