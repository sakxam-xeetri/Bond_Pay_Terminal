#ifndef WEB_H
#define WEB_H

#include <pgmspace.h>

// ── BondPay Compact UI ──────────────────────────────────────────────
// Reliable compact HTML for ESP8266 SoftAP serving.
// Streams via chunked writes to avoid OOM. ~13KB PROGMEM.

const char HTML_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>BondPay Terminal</title>
<style>
:root{--bg:#0a0e1a;--card:#131830;--bdr:rgba(255,255,255,.07);--pri:#00d2ff;--prih:#00b4db;--suc:#10b981;--err:#ef4444;--txt:#f0f1f4;--dim:#8a8fa0;--glass:rgba(19,24,48,.75)}
*{box-sizing:border-box;margin:0;padding:0;font-family:system-ui,-apple-system,sans-serif}
body{background:var(--bg);color:var(--txt);min-height:100vh;overflow-x:hidden}
a,button{color:inherit;text-decoration:none}
/* Header */
.hd{position:sticky;top:0;z-index:50;background:var(--glass);-webkit-backdrop-filter:blur(10px);backdrop-filter:blur(10px);border-bottom:1px solid var(--bdr);padding:.75rem 1.2rem;display:flex;justify-content:space-between;align-items:center}
.lg{font-size:1.15rem;font-weight:700;background:linear-gradient(135deg,#fff,var(--pri));-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-clip:text}
.st{display:flex;align-items:center;gap:.4rem;font-size:.82rem;color:var(--suc)}
.dot{width:7px;height:7px;background:var(--suc);border-radius:50%;box-shadow:0 0 6px var(--suc);animation:pulse 2s infinite}
/* Tabs */
.tb{display:flex;gap:2px;background:var(--card);padding:4px 6px;overflow-x:auto;border-bottom:1px solid var(--bdr);-webkit-overflow-scrolling:touch}
.tb button{background:none;border:none;color:var(--dim);padding:10px 14px;font-size:.84rem;font-weight:500;cursor:pointer;border-radius:6px;white-space:nowrap;transition:all .2s}
.tb button.a{background:rgba(0,210,255,.08);color:var(--pri);border:1px solid rgba(0,210,255,.15)}
.tb button:hover{color:var(--txt)}
/* Panels */
.pn{display:none;padding:1.2rem;max-width:960px;margin:0 auto;animation:fadeIn .25s ease}
.pn.a{display:block}
h1{font-size:1.4rem;font-weight:700;margin-bottom:.3rem}
.sub{color:var(--dim);font-size:.88rem;margin-bottom:1.5rem}
/* Stats */
.sg{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:1rem;margin-bottom:1.5rem}
.sc{background:var(--card);border:1px solid var(--bdr);border-radius:10px;padding:1.2rem;position:relative;overflow:hidden}
.sc::before{content:'';position:absolute;top:0;left:0;width:3px;height:100%;background:var(--pri)}
.sc.ok::before{background:var(--suc)}.sc.er::before{background:var(--err)}
.sc .t{font-size:.78rem;font-weight:500;color:var(--dim);text-transform:uppercase;letter-spacing:.4px;margin-bottom:.4rem}
.sc .v{font-size:1.6rem;font-weight:700}
/* Cards */
.cd{background:var(--card);border:1px solid var(--bdr);border-radius:10px;padding:1.2rem;margin-bottom:1.2rem}
.cd h2{font-size:1.05rem;font-weight:600;margin-bottom:1rem}
/* Form */
.fg{margin-bottom:1rem}
.fl{display:block;font-size:.85rem;font-weight:500;color:var(--dim);margin-bottom:.35rem}
input[type=text],input[type=number]{width:100%;background:rgba(10,14,26,.5);border:1px solid var(--bdr);border-radius:7px;padding:.65rem .85rem;color:var(--txt);font-size:.92rem;transition:border-color .2s}
input:focus{outline:none;border-color:var(--pri)}
/* Buttons */
.btn{display:inline-flex;align-items:center;justify-content:center;padding:.65rem 1.2rem;font-size:.88rem;font-weight:600;border-radius:7px;border:none;cursor:pointer;transition:all .2s}
.bp{background:var(--pri);color:#000}.bp:hover{background:var(--prih)}
.bs{background:var(--suc);color:#fff}
.bd{background:var(--err);color:#fff}
.bo{background:transparent;border:1px solid var(--bdr);color:var(--txt)}.bo:hover{background:rgba(255,255,255,.04)}
.bf{width:100%}
/* Tables */
.tc{overflow-x:auto;border:1px solid var(--bdr);border-radius:8px}
table{width:100%;border-collapse:collapse;font-size:.85rem}
th{background:rgba(255,255,255,.02);color:var(--dim);padding:.7rem .9rem;text-align:left;font-size:.78rem;font-weight:600;text-transform:uppercase;border-bottom:1px solid var(--bdr)}
td{padding:.7rem .9rem;border-bottom:1px solid var(--bdr)}
tr:last-child td{border-bottom:none}
tr:hover td{background:rgba(255,255,255,.01)}
.bg{display:inline-block;padding:3px 7px;border-radius:4px;font-size:.72rem;font-weight:600;text-transform:uppercase}
.bg-ok{background:rgba(16,185,129,.12);color:var(--suc)}
.bg-er{background:rgba(239,68,68,.12);color:var(--err)}
/* Overlay */
.ov{position:fixed;top:0;left:0;right:0;bottom:0;background:rgba(10,14,26,.9);-webkit-backdrop-filter:blur(6px);backdrop-filter:blur(6px);z-index:100;display:none;align-items:center;justify-content:center;padding:1.2rem}
.ov.a{display:flex}
.ml{background:var(--card);border:1px solid rgba(255,255,255,.1);border-radius:14px;max-width:420px;width:100%;padding:2rem;text-align:center;box-shadow:0 16px 40px rgba(0,0,0,.5);animation:scaleIn .25s ease}
/* Scanner ring animation */
.ring{width:90px;height:90px;margin:0 auto 1.2rem;position:relative;border:2px solid rgba(0,210,255,.2);border-radius:50%;display:flex;align-items:center;justify-content:center;background:rgba(0,210,255,.04)}
.ring::after{content:'';position:absolute;inset:-2px;border:2px solid var(--pri);border-radius:50%;animation:radar 1.5s linear infinite}
.ring svg{width:2.5rem;height:2.5rem;fill:var(--pri)}
/* Result icons */
.rok{width:70px;height:70px;margin:0 auto 1rem;background:rgba(16,185,129,.1);border:2px solid var(--suc);border-radius:50%;display:flex;align-items:center;justify-content:center}
.rer{width:70px;height:70px;margin:0 auto 1rem;background:rgba(239,68,68,.1);border:2px solid var(--err);border-radius:50%;display:flex;align-items:center;justify-content:center}
/* Grid helpers */
.sb{display:flex;gap:.5rem;margin-bottom:.8rem}
.gr{display:grid;gap:1.2rem}
@media(min-width:640px){.gr2{grid-template-columns:2fr 1fr}}
/* Animations */
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}
@keyframes fadeIn{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:translateY(0)}}
@keyframes scaleIn{from{opacity:0;transform:scale(.95)}to{opacity:1;transform:scale(1)}}
@keyframes radar{from{transform:scale(1);opacity:1}to{transform:scale(1.5);opacity:0}}
</style></head><body>
<div class="hd"><span class="lg">BondPay</span><div class="st"><div class="dot"></div><span id="clk">Online</span></div></div>
<div class="tb" id="tabs">
<button class="a" onclick="sw('dash')">Dashboard</button>
<button onclick="sw('cards')">Cards</button>
<button onclick="sw('pay')">Payment</button>
<button onclick="sw('txn')">Logs</button>
<button onclick="sw('set')">Settings</button>
</div>

<div id="p-dash" class="pn a">
<h1 id="snd">BondPay Station</h1><p class="sub">Offline terminal dashboard</p>
<div class="sg">
<div class="sc"><div class="t">Active Cards</div><div class="v" id="s-cc">0</div></div>
<div class="sc ok"><div class="t">Total Balance</div><div class="v" id="s-bt">NPR 0</div></div>
<div class="sc er"><div class="t">Pending Sync</div><div class="v" id="s-ps">0</div></div>
</div>
<div class="gr gr2">
<div class="cd"><h2>Recent Transactions</h2><div class="tc"><table><thead><tr><th>Time</th><th>Name</th><th>Amt</th><th>Bal</th><th>Status</th></tr></thead><tbody id="rtl"><tr><td colspan="5" style="text-align:center;color:var(--dim)">No transactions</td></tr></tbody></table></div></div>
<div class="cd"><h2>Sync Queue</h2><p style="color:var(--dim);font-size:.85rem;margin-bottom:1rem">Push local transactions for cloud upload.</p><button class="btn bp bf" onclick="doSync()">Sync Now</button></div>
</div></div>

<div id="p-cards" class="pn">
<h1>Card Management</h1><p class="sub">Manage RFID cards & balances</p>
<div class="gr gr2">
<div class="cd"><h2>Registered Cards</h2><div class="tc"><table><thead><tr><th>Name</th><th>ID</th><th>UID</th><th>Balance</th><th>Act</th></tr></thead><tbody id="cl"><tr><td colspan="5" style="text-align:center;color:var(--dim)">No cards</td></tr></tbody></table></div></div>
<div class="cd"><h2>Add New Card</h2><form onsubmit="regCard(event)">
<div class="fg"><label class="fl">Name</label><input type="text" id="rn" required placeholder="Sakshyam"></div>
<div class="fg"><label class="fl">User ID</label><input type="text" id="ru" required placeholder="BP001"></div>
<div class="fg"><label class="fl">Balance (NPR)</label><input type="number" id="rb" required min="0" value="1000" step="0.01"></div>
<button type="submit" class="btn bp bf">Register Card</button></form></div>
</div></div>

<div id="p-pay" class="pn">
<h1>Payment</h1><p class="sub">Tap card to deduct balance</p>
<div style="max-width:400px;margin:0 auto">
<div class="cd" style="text-align:center">
<div class="ring" style="margin-top:.5rem"><svg viewBox="0 0 24 24"><rect x="2" y="5" width="20" height="14" rx="2" fill="none" stroke="currentColor" stroke-width="1.5"/><line x1="2" y1="10" x2="22" y2="10" stroke="currentColor" stroke-width="1.5"/></svg></div>
<h2 style="margin-bottom:.5rem">Initiate Payment</h2>
<p style="color:var(--dim);font-size:.88rem;margin-bottom:1.5rem">Enter amount, then tap card on reader.</p>
<form onsubmit="startPay(event)">
<div class="fg"><label class="fl">Amount (NPR)</label><input type="number" id="pa" style="font-size:1.3rem;text-align:center;padding:.8rem" required min="0.01" step="0.01" placeholder="0.00"></div>
<button type="submit" class="btn bp bf">Start Payment</button></form>
</div></div></div>

<div id="p-txn" class="pn">
<h1>Transaction Log</h1><p class="sub">Full audit history</p>
<div class="cd"><div class="sb"><input type="text" id="ts" oninput="filt()" placeholder="Search..." style="flex:1"><button class="btn bo" onclick="ld()">Refresh</button></div>
<div class="tc"><table><thead><tr><th>#</th><th>Time</th><th>UID</th><th>Name</th><th>Amt</th><th>Prev</th><th>Rem</th><th>Status</th></tr></thead><tbody id="atl"><tr><td colspan="8" style="text-align:center;color:var(--dim)">No records</td></tr></tbody></table></div></div></div>

<div id="p-set" class="pn">
<h1>Settings</h1><p class="sub">Station configuration</p>
<div class="gr gr2">
<div class="cd"><h2>System</h2><form onsubmit="saveCfg(event)">
<div class="fg"><label class="fl">Station Name</label><input type="text" id="sn" required value="BondPay Station 1"></div>
<button type="submit" class="btn bp">Save</button></form>
<div style="margin-top:1.2rem;border-top:1px solid var(--bdr);padding-top:1.2rem">
<h3 style="color:var(--err);font-size:.9rem;margin-bottom:.5rem;font-weight:600">Danger Zone</h3>
<div style="display:flex;gap:.5rem;flex-wrap:wrap"><button class="btn bd" onclick="clrLogs()">Clear Logs</button><button class="btn bo" onclick="fReset()">Factory Reset</button></div>
</div></div>
<div class="cd"><h2>Terminal Info</h2><div style="font-size:.88rem;display:flex;flex-direction:column;gap:.6rem">
<div style="display:flex;justify-content:space-between"><span style="color:var(--dim)">SSID:</span><b>BondPay</b></div>
<div style="display:flex;justify-content:space-between"><span style="color:var(--dim)">IP:</span><b>192.168.4.1</b></div>
<div style="display:flex;justify-content:space-between"><span style="color:var(--dim)">Free Mem:</span><b id="sh">--</b></div>
<div style="display:flex;justify-content:space-between"><span style="color:var(--dim)">Uptime:</span><b id="su">--</b></div>
</div></div>
</div></div>

<div id="ov" class="ov"><div class="ml" id="mc"></div></div>

<script>
var ct='dash',ip=0,ot=[],P='POST';
window.onload=function(){ld();syncT();poll();setInterval(function(){if(ct==='dash')ld()},8000)};
function sw(t){var p=document.querySelectorAll('.pn'),b=document.getElementById('tabs').children;
for(var i=0;i<p.length;i++)p[i].classList.remove('a');
for(var i=0;i<b.length;i++)b[i].classList.remove('a');
document.getElementById('p-'+t).classList.add('a');
var m={dash:0,cards:1,pay:2,txn:3,set:4};if(m[t]!==undefined)b[m[t]].classList.add('a');
ct=t;ld()}
function syncT(){var e=Math.floor(Date.now()/1000);fetch('/api/time?epoch='+e,{method:P}).catch(function(){});
setInterval(function(){document.getElementById('clk').innerText=new Date().toLocaleTimeString()},1000)}
function ld(){fetch('/api/status').then(function(r){return r.json()}).then(function(d){
document.getElementById('s-ps').innerText=d.pendingSyncCount||0;
document.getElementById('sh').innerText=Math.round(d.freeHeap/1024)+' KB';
var s=d.uptime/1000;document.getElementById('su').innerText=Math.floor(s/3600)+'h '+Math.floor((s%3600)/60)+'m';
return fetch('/api/cards')}).then(function(r){return r.json()}).then(function(c){
document.getElementById('s-cc').innerText=c.length;
var t=0;for(var i=0;i<c.length;i++)t+=parseFloat(c[i].balance||0);
document.getElementById('s-bt').innerText='NPR '+t.toFixed(2);rCards(c);
return fetch('/api/transactions')}).then(function(r){return r.json()}).then(function(x){
ot=x;rTxn(x);rRecent(x)}).catch(function(){})}
function E(s){if(!s)return'';return String(s).replace(/[&<>"']/g,function(c){return{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]||c})}
function rCards(c){var b=document.getElementById('cl');
if(!c.length){b.innerHTML='<tr><td colspan="5" style="text-align:center;color:var(--dim)">No cards</td></tr>';return}
var h='';for(var i=0;i<c.length;i++){var d=c[i];
h+='<tr><td><b>'+E(d.name)+'</b></td><td>'+E(d.userId)+'</td><td><code>'+E(d.uid)+'</code></td><td><b>NPR '+parseFloat(d.balance).toFixed(2)+'</b></td><td><button class="btn bo" style="padding:3px 8px;font-size:.72rem" onclick="edBal(\''+E(d.uid)+'\','+d.balance+')">Edit</button> <button class="btn bd" style="padding:3px 8px;font-size:.72rem" onclick="delC(\''+E(d.uid)+'\')">Del</button></td></tr>'}
b.innerHTML=h}
function rTxn(x){var b=document.getElementById('atl');
if(!x.length){b.innerHTML='<tr><td colspan="8" style="text-align:center;color:var(--dim)">No records</td></tr>';return}
var h='',s=x.slice().reverse();for(var i=0;i<s.length;i++){var t=s[i],sc=t.status.toLowerCase().indexOf('success')!==-1?'bg-ok':'bg-er';
h+='<tr><td>#'+t.id+'</td><td>'+E(t.timestamp)+'</td><td><code>'+E(t.uid)+'</code></td><td>'+E(t.name)+'</td><td><b>'+parseFloat(t.amount).toFixed(2)+'</b></td><td>'+parseFloat(t.prevBal).toFixed(2)+'</td><td>'+parseFloat(t.remBal).toFixed(2)+'</td><td><span class="bg '+sc+'">'+E(t.status)+'</span></td></tr>'}
b.innerHTML=h}
function rRecent(x){var b=document.getElementById('rtl');
if(!x.length){b.innerHTML='<tr><td colspan="5" style="text-align:center;color:var(--dim)">No transactions</td></tr>';return}
var r=x.slice().reverse().slice(0,5),h='';for(var i=0;i<r.length;i++){var t=r[i],sc=t.status.toLowerCase().indexOf('success')!==-1?'bg-ok':'bg-er';
var ts=t.timestamp?t.timestamp.split(' ')[1]||t.timestamp:'';
h+='<tr><td>'+E(ts)+'</td><td>'+E(t.name)+'</td><td><b>'+parseFloat(t.amount).toFixed(2)+'</b></td><td>'+parseFloat(t.remBal).toFixed(2)+'</td><td><span class="bg '+sc+'">'+E(t.status)+'</span></td></tr>'}
b.innerHTML=h}
function poll(){if(ip)return;ip=1;(function p(){
fetch('/api/status').then(function(r){return r.json()}).then(function(d){hState(d);setTimeout(p,1200)}).catch(function(){setTimeout(p,3000)})})()}
function hState(d){var o=document.getElementById('ov'),m=document.getElementById('mc');
if(d.mode==='PAYMENT'){o.classList.add('a');
m.innerHTML='<div class="ring"><svg viewBox="0 0 24 24"><rect x="2" y="5" width="20" height="14" rx="2" fill="none" stroke="var(--pri)" stroke-width="1.5"/></svg></div><h2 style="margin-bottom:.5rem">Waiting for Card</h2><p style="color:var(--pri);font-size:1.1rem;font-weight:700;margin-bottom:.5rem">NPR '+parseFloat(d.activeAmount).toFixed(2)+'</p><p style="color:var(--dim);margin-bottom:1.5rem">Tap RFID card on reader</p><button class="btn bo" onclick="cxl(\'payment\')">Cancel</button>'}
else if(d.mode==='ADD_CARD'){o.classList.add('a');
m.innerHTML='<div class="ring"><svg viewBox="0 0 24 24"><path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z" fill="var(--pri)"/></svg></div><h2 style="margin-bottom:.5rem">Scan New Card</h2><p style="color:var(--pri);font-weight:700;margin-bottom:.5rem">ID: '+E(d.activeUserId)+'</p><p style="color:var(--dim);margin-bottom:1.5rem">Tap unregistered RFID card</p><button class="btn bo" onclick="cxl(\'cards\')">Cancel</button>'}
else{if(d.lastEvent&&d.lastEvent.processed===false){showRes(d.lastEvent)}
else if(!m.querySelector('.rs')){o.classList.remove('a')}}}
function showRes(ev){var o=document.getElementById('ov'),m=document.getElementById('mc');o.classList.add('a');
var ok=ev.status.toLowerCase().indexOf('success')!==-1;
if(ok){m.innerHTML='<div class="rs"><div class="rok"><svg style="width:2rem;height:2rem" viewBox="0 0 24 24" fill="none" stroke="var(--suc)" stroke-width="3"><polyline points="20 6 9 17 4 12"/></svg></div><h2 style="color:var(--suc);margin-bottom:.3rem">Success</h2><p style="font-weight:600;margin-bottom:.2rem">'+E(ev.name||'')+'</p><p style="color:var(--dim);font-size:.82rem;margin-bottom:1rem">UID: '+E(ev.uid)+'</p><div style="background:rgba(255,255,255,.02);border:1px solid var(--bdr);padding:.8rem;border-radius:7px;text-align:left;margin-bottom:1rem"><div style="display:flex;justify-content:space-between;margin-bottom:.3rem"><span>Charged:</span><b>NPR '+parseFloat(ev.amount||0).toFixed(2)+'</b></div><div style="display:flex;justify-content:space-between"><span>Balance:</span><b>NPR '+parseFloat(ev.remBal||0).toFixed(2)+'</b></div></div><p style="color:var(--dim);font-size:.72rem">Auto-closing...</p></div>'}
else{m.innerHTML='<div class="rs"><div class="rer"><svg style="width:2rem;height:2rem" viewBox="0 0 24 24" fill="none" stroke="var(--err)" stroke-width="3"><line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/></svg></div><h2 style="color:var(--err);margin-bottom:.5rem">Failed</h2><p style="font-weight:600;margin-bottom:1rem">'+E(ev.message||'Declined')+'</p><button class="btn bo bf" onclick="clOv()">Close</button></div>'}
fetch('/api/status/acknowledge-event',{method:P});setTimeout(clOv,3500)}
function clOv(){document.getElementById('ov').classList.remove('a');ld()}
function startPay(e){e.preventDefault();var a=document.getElementById('pa').value;if(a<=0)return alert('Enter valid amount');fetch('/api/payment/start?amount='+a,{method:P}).then(function(){document.getElementById('pa').value=''})}
function regCard(e){e.preventDefault();var n=document.getElementById('rn').value,u=document.getElementById('ru').value,b=document.getElementById('rb').value;fetch('/api/cards/add?name='+encodeURIComponent(n)+'&userId='+encodeURIComponent(u)+'&balance='+b,{method:P}).then(function(){document.getElementById('rn').value='';document.getElementById('ru').value='';document.getElementById('rb').value='1000'})}
function cxl(t){fetch(t==='payment'?'/api/payment/cancel':'/api/cards/cancel',{method:P}).then(clOv)}
function edBal(uid,cur){var n=prompt('New balance for '+uid+':',cur);if(n===null)return;var v=parseFloat(n);if(isNaN(v)||v<0)return alert('Invalid');fetch('/api/cards/update-balance?uid='+uid+'&balance='+v,{method:P}).then(function(r){if(r.ok){alert('Updated');ld()}else alert('Failed')})}
function delC(uid){if(!confirm('Delete card '+uid+'?'))return;fetch('/api/cards/delete?uid='+uid,{method:P}).then(function(){ld()})}
function doSync(){fetch('/api/transactions/sync',{method:P}).then(function(){alert('Synced');ld()})}
function clrLogs(){if(!confirm('Clear all logs?'))return;fetch('/api/transactions/clear',{method:P}).then(function(){alert('Cleared');ld()})}
function fReset(){if(!confirm('WARNING: Delete ALL data?'))return;fetch('/api/settings/factory-reset',{method:P}).then(function(){alert('Resetting...');location.reload()})}
function saveCfg(e){e.preventDefault();var n=document.getElementById('sn').value;fetch('/api/settings?stationName='+encodeURIComponent(n),{method:P}).then(function(){alert('Saved');document.getElementById('snd').innerText=n})}
function filt(){var q=document.getElementById('ts').value.toLowerCase();var f=ot.filter(function(t){return(t.uid||'').toLowerCase().indexOf(q)!==-1||(t.name||'').toLowerCase().indexOf(q)!==-1||(t.status||'').toLowerCase().indexOf(q)!==-1||(t.timestamp||'').toLowerCase().indexOf(q)!==-1});rTxn(f)}
</script></body></html>)rawliteral";

// ── Helper: Send page using chunked writes ────────────────────────────
// Streams PROGMEM in 256-byte pieces without allocating the entire page.
// Uses yield() between chunks to keep WiFi stack responsive.
inline void sendChunkedPage(ESP8266WebServer &srv) {
  srv.setContentLength(CONTENT_LENGTH_UNKNOWN);
  srv.send(200, "text/html", "");
  WiFiClient client = srv.client();

  if (!client.connected()) return;

  size_t len = strlen_P(HTML_PAGE);
  size_t sent = 0;
  char buf[256];

  while (sent < len && client.connected()) {
    size_t piece = len - sent;
    if (piece > sizeof(buf)) piece = sizeof(buf);
    memcpy_P(buf, HTML_PAGE + sent, piece);
    size_t written = client.write(buf, piece);
    if (written == 0) break;  // Client disconnected
    sent += written;
    yield();
  }
}

#endif
