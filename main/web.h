#ifndef WEB_H
#define WEB_H

#include <pgmspace.h>

// ── Ultra-compact BondPay UI ──────────────────────────────────────────
// Stripped to ~14KB for ESP8266 flash. All functionality preserved.
// Pure HTML + minimal inline CSS + essential JS only.

const char HTML_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>BondPay Station</title>
<style>
*{box-sizing:border-box;margin:0;padding:0;font-family:system-ui,sans-serif}
body{background:#0b0f19;color:#eee;min-height:100vh}
a,button{color:inherit}
header{background:#141928;padding:.7rem 1rem;display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid #222}
.logo{font-size:1.1rem;font-weight:700;color:#00d2ff}
.st{color:#10b981;font-size:.8rem}
.tabs{display:flex;gap:2px;background:#141928;padding:4px 8px;overflow-x:auto;border-bottom:1px solid #222}
.tabs button{background:none;border:none;color:#777;padding:8px 12px;font-size:.82rem;cursor:pointer;border-radius:4px;white-space:nowrap}
.tabs button.a{background:#1a2240;color:#00d2ff}
.tp{display:none;padding:1rem;max-width:900px;margin:0 auto}
.tp.a{display:block}
h1{font-size:1.3rem;margin-bottom:.3rem}
.sub{color:#777;font-size:.85rem;margin-bottom:1rem}
.sg{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:.8rem;margin-bottom:1rem}
.sc{background:#161b30;border:1px solid #222;border-radius:8px;padding:1rem;border-left:3px solid #00d2ff}
.sc.ok{border-left-color:#10b981}.sc.er{border-left-color:#ef4444}
.sc .t{font-size:.75rem;color:#777;text-transform:uppercase;margin-bottom:4px}
.sc .v{font-size:1.4rem;font-weight:700}
.cd{background:#161b30;border:1px solid #222;border-radius:8px;padding:1rem;margin-bottom:1rem}
.cd h2{font-size:1rem;margin-bottom:.8rem}
.fg{margin-bottom:.8rem}
.fl{display:block;font-size:.82rem;color:#777;margin-bottom:4px}
input[type=text],input[type=number]{width:100%;background:#0d111d;border:1px solid #333;border-radius:6px;padding:.55rem .7rem;color:#eee;font-size:.9rem}
input:focus{outline:none;border-color:#00d2ff}
.btn{display:inline-block;padding:.55rem 1rem;font-size:.85rem;font-weight:600;border-radius:6px;border:none;cursor:pointer}
.bp{background:#00d2ff;color:#000}.bp:hover{background:#00b4db}
.bs{background:#10b981;color:#fff}
.bd{background:#ef4444;color:#fff}
.bo{background:none;border:1px solid #333;color:#eee}
.bf{width:100%}
.tc{overflow-x:auto}
table{width:100%;border-collapse:collapse;font-size:.85rem}
th{background:#111;color:#777;padding:.6rem;text-align:left;font-size:.75rem;text-transform:uppercase}
td{padding:.6rem;border-bottom:1px solid #1a1a2e}
.bg{display:inline-block;padding:2px 6px;border-radius:3px;font-size:.7rem;font-weight:600;text-transform:uppercase}
.bg-ok{background:rgba(16,185,129,.15);color:#10b981}
.bg-er{background:rgba(239,68,68,.15);color:#ef4444}
.ov{position:fixed;top:0;left:0;right:0;bottom:0;background:rgba(0,0,0,.85);z-index:100;display:none;align-items:center;justify-content:center;padding:1rem}
.ov.a{display:flex}
.ml{background:#161b30;border:1px solid #333;border-radius:12px;max-width:400px;width:100%;padding:1.5rem;text-align:center}
.sb{display:flex;gap:.5rem;margin-bottom:.8rem}
.gr{display:grid;gap:1rem}
@media(min-width:600px){.gr2{grid-template-columns:2fr 1fr}}
</style></head><body>
<header><span class="logo">BondPay</span><span class="st" id="clk">● Online</span></header>
<div class="tabs" id="tabs">
<button class="a" onclick="sw('dash')">Dashboard</button>
<button onclick="sw('cards')">Cards</button>
<button onclick="sw('pay')">Payment</button>
<button onclick="sw('txn')">Logs</button>
<button onclick="sw('set')">Settings</button>
</div>

<div id="p-dash" class="tp a">
<h1 id="snd">BondPay Station</h1><p class="sub">Offline terminal dashboard</p>
<div class="sg">
<div class="sc"><div class="t">Cards</div><div class="v" id="s-cc">0</div></div>
<div class="sc ok"><div class="t">Balance</div><div class="v" id="s-bt">NPR 0</div></div>
<div class="sc er"><div class="t">Pending</div><div class="v" id="s-ps">0</div></div>
</div>
<div class="gr gr2">
<div class="cd"><h2>Recent</h2><div class="tc"><table><thead><tr><th>Time</th><th>Name</th><th>Amt</th><th>Bal</th><th>Status</th></tr></thead><tbody id="rtl"><tr><td colspan="5" style="text-align:center;color:#555">No transactions</td></tr></tbody></table></div></div>
<div class="cd"><h2>Sync</h2><p style="color:#777;font-size:.85rem;margin-bottom:.8rem">Queue local transactions for upload.</p><button class="btn bp bf" onclick="doSync()">Sync Now</button></div>
</div></div>

<div id="p-cards" class="tp">
<h1>Cards</h1><p class="sub">Manage RFID cards</p>
<div class="gr gr2">
<div class="cd"><h2>Registered</h2><div class="tc"><table><thead><tr><th>Name</th><th>ID</th><th>UID</th><th>Balance</th><th>Act</th></tr></thead><tbody id="cl"><tr><td colspan="5" style="text-align:center;color:#555">No cards</td></tr></tbody></table></div></div>
<div class="cd"><h2>Add Card</h2><form onsubmit="regCard(event)">
<div class="fg"><label class="fl">Name</label><input type="text" id="rn" required placeholder="Sakshyam"></div>
<div class="fg"><label class="fl">User ID</label><input type="text" id="ru" required placeholder="BP001"></div>
<div class="fg"><label class="fl">Balance (NPR)</label><input type="number" id="rb" required min="0" value="1000" step="0.01"></div>
<button type="submit" class="btn bp bf">Register</button></form></div>
</div></div>

<div id="p-pay" class="tp">
<h1>Payment</h1><p class="sub">Tap card to deduct balance</p>
<div style="max-width:380px;margin:0 auto">
<div class="cd" style="text-align:center">
<h2 style="margin-bottom:1rem">Initiate Payment</h2>
<form onsubmit="startPay(event)">
<div class="fg"><label class="fl">Amount (NPR)</label><input type="number" id="pa" style="font-size:1.3rem;text-align:center;padding:.8rem" required min="0.01" step="0.01" placeholder="0.00"></div>
<button type="submit" class="btn bp bf">Start Payment</button></form>
</div></div></div>

<div id="p-txn" class="tp">
<h1>Transaction Log</h1><p class="sub">Full audit history</p>
<div class="cd"><div class="sb"><input type="text" id="ts" oninput="filt()" placeholder="Search..."><button class="btn bo" onclick="ld()">Refresh</button></div>
<div class="tc"><table><thead><tr><th>#</th><th>Time</th><th>UID</th><th>Name</th><th>Amt</th><th>Prev</th><th>Rem</th><th>Status</th></tr></thead><tbody id="atl"><tr><td colspan="8" style="text-align:center;color:#555">No records</td></tr></tbody></table></div></div></div>

<div id="p-set" class="tp">
<h1>Settings</h1><p class="sub">Station configuration</p>
<div class="gr gr2">
<div class="cd"><h2>System</h2><form onsubmit="saveCfg(event)">
<div class="fg"><label class="fl">Station Name</label><input type="text" id="sn" required value="BondPay Station 1"></div>
<button type="submit" class="btn bp">Save</button></form>
<div style="margin-top:1rem;border-top:1px solid #222;padding-top:1rem">
<h3 style="color:#ef4444;font-size:.9rem;margin-bottom:.5rem">Danger Zone</h3>
<div style="display:flex;gap:.5rem;flex-wrap:wrap"><button class="btn bd" onclick="clrLogs()">Clear Logs</button><button class="btn bo" onclick="fReset()">Factory Reset</button></div>
</div></div>
<div class="cd"><h2>Info</h2><div style="font-size:.85rem">
<div style="display:flex;justify-content:space-between;margin-bottom:.4rem"><span style="color:#777">SSID:</span><b>BondPay</b></div>
<div style="display:flex;justify-content:space-between;margin-bottom:.4rem"><span style="color:#777">IP:</span><b>192.168.4.1</b></div>
<div style="display:flex;justify-content:space-between;margin-bottom:.4rem"><span style="color:#777">Free Mem:</span><b id="sh">--</b></div>
<div style="display:flex;justify-content:space-between"><span style="color:#777">Uptime:</span><b id="su">--</b></div>
</div></div>
</div></div>

<div id="ov" class="ov"><div class="ml" id="mc"></div></div>

<script>
var ct='dash',ip=0,ot=[],P='POST';
onload=function(){ld();syncT();poll();setInterval(function(){if(ct==='dash')ld()},8000)};
function sw(t){var p=document.querySelectorAll('.tp'),b=document.getElementById('tabs').children;
for(var i=0;i<p.length;i++)p[i].classList.remove('a');
for(var i=0;i<b.length;i++)b[i].classList.remove('a');
document.getElementById('p-'+t).classList.add('a');
var m={dash:0,cards:1,pay:2,txn:3,set:4};if(m[t]!==undefined)b[m[t]].classList.add('a');
ct=t;ld()}
function syncT(){var e=Math.floor(Date.now()/1000);fetch('/api/time?epoch='+e,{method:P}).catch(function(){});
setInterval(function(){document.getElementById('clk').innerText='● '+new Date().toLocaleTimeString()},1000)}
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
if(!c.length){b.innerHTML='<tr><td colspan="5" style="text-align:center;color:#555">No cards</td></tr>';return}
var h='';for(var i=0;i<c.length;i++){var d=c[i];
h+='<tr><td><b>'+E(d.name)+'</b></td><td>'+E(d.userId)+'</td><td><code>'+E(d.uid)+'</code></td><td><b>NPR '+parseFloat(d.balance).toFixed(2)+'</b></td><td><button class="btn bo" style="padding:3px 8px;font-size:.75rem" onclick="edBal(\''+E(d.uid)+'\','+d.balance+')">Edit</button> <button class="btn bd" style="padding:3px 8px;font-size:.75rem" onclick="delC(\''+E(d.uid)+'\')">Del</button></td></tr>'}
b.innerHTML=h}
function rTxn(x){var b=document.getElementById('atl');
if(!x.length){b.innerHTML='<tr><td colspan="8" style="text-align:center;color:#555">No records</td></tr>';return}
var h='',s=x.slice().reverse();for(var i=0;i<s.length;i++){var t=s[i],sc=t.status.toLowerCase().indexOf('success')!==-1?'bg-ok':'bg-er';
h+='<tr><td>#'+t.id+'</td><td>'+E(t.timestamp)+'</td><td><code>'+E(t.uid)+'</code></td><td>'+E(t.name)+'</td><td><b>'+parseFloat(t.amount).toFixed(2)+'</b></td><td>'+parseFloat(t.prevBal).toFixed(2)+'</td><td>'+parseFloat(t.remBal).toFixed(2)+'</td><td><span class="bg '+sc+'">'+E(t.status)+'</span></td></tr>'}
b.innerHTML=h}
function rRecent(x){var b=document.getElementById('rtl');
if(!x.length){b.innerHTML='<tr><td colspan="5" style="text-align:center;color:#555">No transactions</td></tr>';return}
var r=x.slice().reverse().slice(0,5),h='';for(var i=0;i<r.length;i++){var t=r[i],sc=t.status.toLowerCase().indexOf('success')!==-1?'bg-ok':'bg-er';
var ts=t.timestamp?t.timestamp.split(' ')[1]||t.timestamp:'';
h+='<tr><td>'+E(ts)+'</td><td>'+E(t.name)+'</td><td><b>'+parseFloat(t.amount).toFixed(2)+'</b></td><td>'+parseFloat(t.remBal).toFixed(2)+'</td><td><span class="bg '+sc+'">'+E(t.status)+'</span></td></tr>'}
b.innerHTML=h}
function poll(){if(ip)return;ip=1;(function p(){
fetch('/api/status').then(function(r){return r.json()}).then(function(d){hState(d);setTimeout(p,1500)}).catch(function(){setTimeout(p,3000)})})()}
function hState(d){var o=document.getElementById('ov'),m=document.getElementById('mc');
if(d.mode==='PAYMENT'){o.classList.add('a');
m.innerHTML='<h2 style="margin-bottom:.5rem">⏳ Waiting for Card</h2><p style="color:#00d2ff;font-size:1.1rem;font-weight:700;margin-bottom:.5rem">NPR '+parseFloat(d.activeAmount).toFixed(2)+'</p><p style="color:#777;margin-bottom:1rem">Tap RFID card on reader</p><button class="btn bo" onclick="cxl(\'payment\')">Cancel</button>'}
else if(d.mode==='ADD_CARD'){o.classList.add('a');
m.innerHTML='<h2 style="margin-bottom:.5rem">📇 Scan New Card</h2><p style="color:#00d2ff;font-weight:700;margin-bottom:.5rem">ID: '+E(d.activeUserId)+'</p><p style="color:#777;margin-bottom:1rem">Tap unregistered RFID card</p><button class="btn bo" onclick="cxl(\'cards\')">Cancel</button>'}
else{if(d.lastEvent&&d.lastEvent.processed===false){showRes(d.lastEvent)}
else if(!m.querySelector('.rs')){o.classList.remove('a')}}}
function showRes(ev){var o=document.getElementById('ov'),m=document.getElementById('mc');o.classList.add('a');
var ok=ev.status.toLowerCase().indexOf('success')!==-1;
if(ok){m.innerHTML='<div class="rs"><div style="font-size:2.5rem;margin-bottom:.5rem">✅</div><h2 style="color:#10b981;margin-bottom:.3rem">Success</h2><p style="font-weight:600;margin-bottom:.2rem">'+E(ev.name||'')+'</p><p style="color:#777;font-size:.85rem;margin-bottom:.8rem">UID: '+E(ev.uid)+'</p><div style="background:#0d111d;border:1px solid #222;padding:.8rem;border-radius:6px;text-align:left;margin-bottom:.8rem"><div style="display:flex;justify-content:space-between;margin-bottom:.3rem"><span>Charged:</span><b>NPR '+parseFloat(ev.amount||0).toFixed(2)+'</b></div><div style="display:flex;justify-content:space-between"><span>Balance:</span><b>NPR '+parseFloat(ev.remBal||0).toFixed(2)+'</b></div></div><p style="color:#555;font-size:.75rem">Auto-closing...</p></div>'}
else{m.innerHTML='<div class="rs"><div style="font-size:2.5rem;margin-bottom:.5rem">❌</div><h2 style="color:#ef4444;margin-bottom:.5rem">Failed</h2><p style="font-weight:600;margin-bottom:1rem">'+E(ev.message||'Declined')+'</p><button class="btn bo bf" onclick="clOv()">Close</button></div>'}
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
// Streams PROGMEM in 512-byte pieces without allocating the entire page.
inline void sendChunkedPage(ESP8266WebServer &srv) {
  srv.setContentLength(CONTENT_LENGTH_UNKNOWN);
  srv.send(200, "text/html", "");
  WiFiClient client = srv.client();

  size_t len = strlen_P(HTML_PAGE);
  size_t sent = 0;
  char buf[512];

  while (sent < len) {
    size_t piece = len - sent;
    if (piece > sizeof(buf)) piece = sizeof(buf);
    memcpy_P(buf, HTML_PAGE + sent, piece);
    client.write(buf, piece);
    sent += piece;
    yield();
  }
}

#endif
