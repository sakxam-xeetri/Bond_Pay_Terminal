#ifndef WEB_H
#define WEB_H

#include <pgmspace.h>

const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>BondPay Offline Station</title>
    <style>
        :root {
            --bg-color: #0b0f19;
            --card-bg: #161b30;
            --border-color: rgba(255, 255, 255, 0.06);
            --primary: #00d2ff;
            --primary-hover: #00b4db;
            --success: #10b981;
            --success-hover: #059669;
            --error: #ef4444;
            --text-main: #f3f4f6;
            --text-muted: #9ca3af;
            --glass-bg: rgba(22, 27, 48, 0.7);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            -webkit-font-smoothing: antialiased;
        }

        body {
            background-color: var(--bg-color);
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            overflow-x: hidden;
        }

        /* Top Navigation Bar */
        header {
            position: sticky;
            top: 0;
            z-index: 50;
            background: var(--glass-bg);
            backdrop-filter: blur(12px);
            -webkit-backdrop-filter: blur(12px);
            border-bottom: 1px solid var(--border-color);
            padding: 1rem 1.5rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .logo-section {
            display: flex;
            align-items: center;
            gap: 0.75rem;
        }

        .logo-icon {
            width: 2rem;
            height: 2rem;
            fill: var(--primary);
        }

        .logo-text {
            font-size: 1.25rem;
            font-weight: 700;
            letter-spacing: 0.5px;
            background: linear-gradient(135deg, #ffffff 0%, var(--primary) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .status-badge {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            background: rgba(16, 185, 129, 0.1);
            color: var(--success);
            padding: 0.4rem 0.8rem;
            border-radius: 9999px;
            font-size: 0.85rem;
            font-weight: 500;
            border: 1px solid rgba(16, 185, 129, 0.2);
        }

        .status-dot {
            width: 8px;
            height: 8px;
            background-color: var(--success);
            border-radius: 50%;
            box-shadow: 0 0 8px var(--success);
            animation: pulse 2s infinite;
        }

        /* Main Container Layout */
        .app-container {
            display: flex;
            flex: 1;
        }

        /* Sidebar */
        sidebar {
            width: 260px;
            border-right: 1px solid var(--border-color);
            padding: 1.5rem 1rem;
            display: flex;
            flex-direction: column;
            gap: 0.5rem;
            background-color: rgba(22, 27, 48, 0.3);
        }

        .nav-btn {
            display: flex;
            align-items: center;
            gap: 0.75rem;
            padding: 0.75rem 1rem;
            border: none;
            border-radius: 8px;
            background: transparent;
            color: var(--text-muted);
            font-size: 0.95rem;
            font-weight: 500;
            cursor: pointer;
            text-align: left;
            transition: all 0.2s ease;
        }

        .nav-btn svg {
            width: 1.25rem;
            height: 1.25rem;
            stroke: currentColor;
            stroke-width: 2;
            fill: none;
            transition: stroke 0.2s ease;
        }

        .nav-btn:hover {
            background-color: rgba(255, 255, 255, 0.04);
            color: var(--text-main);
        }

        .nav-btn.active {
            background-color: rgba(0, 210, 255, 0.08);
            color: var(--primary);
            border: 1px solid rgba(0, 210, 255, 0.15);
        }

        /* Content Area */
        main.content {
            flex: 1;
            padding: 2rem;
            max-width: 1200px;
            margin: 0 auto;
            width: 100%;
        }

        .tab-panel {
            display: none;
            animation: fadeIn 0.3s ease;
        }

        .tab-panel.active {
            display: block;
        }

        /* Typography Helpers */
        .page-title {
            font-size: 1.75rem;
            font-weight: 700;
            margin-bottom: 0.5rem;
        }

        .page-subtitle {
            font-size: 0.95rem;
            color: var(--text-muted);
            margin-bottom: 2rem;
        }

        /* Stat Grid */
        .stat-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 1.25rem;
            margin-bottom: 2rem;
        }

        .stat-card {
            background-color: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 1.5rem;
            position: relative;
            overflow: hidden;
        }

        .stat-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 4px;
            height: 100%;
            background: var(--primary);
        }

        .stat-card.success::before { background: var(--success); }
        .stat-card.error::before { background: var(--error); }

        .stat-title {
            font-size: 0.85rem;
            font-weight: 500;
            color: var(--text-muted);
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 0.5rem;
        }

        .stat-val {
            font-size: 1.85rem;
            font-weight: 700;
            color: var(--text-main);
        }

        /* Action Forms */
        .card-row {
            display: grid;
            grid-template-columns: 1fr;
            gap: 1.5rem;
        }

        @media (min-width: 768px) {
            .card-row {
                grid-template-columns: 2fr 1fr;
            }
        }

        .form-card {
            background-color: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 1.75rem;
        }

        .form-group {
            margin-bottom: 1.25rem;
        }

        .form-label {
            display: block;
            font-size: 0.9rem;
            font-weight: 500;
            margin-bottom: 0.5rem;
            color: var(--text-muted);
        }

        .form-input {
            width: 100%;
            background-color: rgba(11, 15, 25, 0.5);
            border: 1px solid var(--border-color);
            border-radius: 8px;
            padding: 0.75rem 1rem;
            color: var(--text-main);
            font-size: 0.95rem;
            transition: border-color 0.2s;
        }

        .form-input:focus {
            outline: none;
            border-color: var(--primary);
        }

        .btn {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
            padding: 0.75rem 1.5rem;
            font-size: 0.95rem;
            font-weight: 600;
            border-radius: 8px;
            border: none;
            cursor: pointer;
            transition: all 0.2s ease;
        }

        .btn-primary {
            background-color: var(--primary);
            color: #000000;
        }

        .btn-primary:hover {
            background-color: var(--primary-hover);
        }

        .btn-success {
            background-color: var(--success);
            color: #ffffff;
        }

        .btn-success:hover {
            background-color: var(--success-hover);
        }

        .btn-danger {
            background-color: var(--error);
            color: #ffffff;
        }

        .btn-danger:hover {
            opacity: 0.9;
        }

        .btn-outline {
            background: transparent;
            border: 1px solid var(--border-color);
            color: var(--text-main);
        }

        .btn-outline:hover {
            background: rgba(255, 255, 255, 0.04);
        }

        .btn-full {
            width: 100%;
        }

        /* Tables & Lists */
        .table-container {
            width: 100%;
            overflow-x: auto;
            border: 1px solid var(--border-color);
            border-radius: 10px;
            background-color: var(--card-bg);
        }

        table {
            width: 100%;
            border-collapse: collapse;
            text-align: left;
        }

        th {
            background-color: rgba(255, 255, 255, 0.02);
            color: var(--text-muted);
            font-size: 0.85rem;
            font-weight: 600;
            text-transform: uppercase;
            padding: 1rem 1.25rem;
            border-bottom: 1px solid var(--border-color);
        }

        td {
            padding: 1rem 1.25rem;
            border-bottom: 1px solid var(--border-color);
            font-size: 0.95rem;
        }

        tr:last-child td {
            border-bottom: none;
        }

        tr:hover td {
            background-color: rgba(255, 255, 255, 0.01);
        }

        .badge {
            display: inline-block;
            padding: 0.25rem 0.5rem;
            border-radius: 4px;
            font-size: 0.75rem;
            font-weight: 600;
            text-transform: uppercase;
        }

        .badge-success {
            background-color: rgba(16, 185, 129, 0.1);
            color: var(--success);
        }

        .badge-danger {
            background-color: rgba(239, 68, 68, 0.1);
            color: var(--error);
        }

        .search-bar {
            margin-bottom: 1rem;
            display: flex;
            gap: 0.75rem;
        }

        /* Overlay / Modal Styles */
        .overlay {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: rgba(11, 15, 25, 0.85);
            backdrop-filter: blur(8px);
            z-index: 100;
            display: none;
            align-items: center;
            justify-content: center;
            padding: 1.5rem;
        }

        .overlay.active {
            display: flex;
        }

        .modal {
            background-color: var(--card-bg);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 16px;
            width: 100%;
            max-width: 460px;
            padding: 2.25rem;
            text-align: center;
            box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.5);
            animation: scaleIn 0.3s cubic-bezier(0.16, 1, 0.3, 1);
        }

        /* RFID scanner effect */
        .scanner-visual {
            width: 100px;
            height: 100px;
            margin: 0 auto 1.5rem;
            position: relative;
            background: rgba(0, 210, 255, 0.05);
            border: 2px solid rgba(0, 210, 255, 0.2);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .scanner-visual svg {
            width: 3rem;
            height: 3rem;
            fill: var(--primary);
        }

        .scanner-ring {
            position: absolute;
            top: -2px;
            left: -2px;
            right: -2px;
            bottom: -2px;
            border: 2px solid var(--primary);
            border-radius: 50%;
            animation: radar 1.5s linear infinite;
        }

        .success-checkmark {
            width: 80px;
            height: 80px;
            margin: 0 auto 1.5rem;
            background: rgba(16, 185, 129, 0.1);
            border: 2px solid var(--success);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: var(--success);
        }

        .failure-cross {
            width: 80px;
            height: 80px;
            margin: 0 auto 1.5rem;
            background: rgba(239, 68, 68, 0.1);
            border: 2px solid var(--error);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: var(--error);
        }

        /* Mobile Bottom Tab Bar */
        @media (max-width: 768px) {
            .app-container {
                flex-direction: column;
            }

            sidebar {
                width: 100%;
                flex-direction: row;
                justify-content: space-around;
                position: fixed;
                bottom: 0;
                left: 0;
                z-index: 40;
                background-color: var(--card-bg);
                border-right: none;
                border-top: 1px solid var(--border-color);
                padding: 0.5rem;
                gap: 0;
            }

            .nav-btn {
                flex-direction: column;
                align-items: center;
                gap: 0.25rem;
                padding: 0.5rem;
                font-size: 0.75rem;
                flex: 1;
            }

            .nav-btn svg {
                width: 1.25rem;
                height: 1.25rem;
            }

            main.content {
                padding: 1.25rem;
                margin-bottom: 70px; /* Space for fixed bottom bar */
            }

            .page-title {
                font-size: 1.5rem;
            }
        }

        /* Keyframes */
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(8px); }
            to { opacity: 1; transform: translateY(0); }
        }

        @keyframes scaleIn {
            from { opacity: 0; transform: scale(0.95); }
            to { opacity: 1; transform: scale(1); }
        }

        @keyframes radar {
            from { transform: scale(1); opacity: 1; }
            to { transform: scale(1.4); opacity: 0; }
        }
    </style>
</head>
<body>

    <!-- Header -->
    <header>
        <div class="logo-section">
            <svg class="logo-icon" viewBox="0 0 24 24">
                <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z"/>
            </svg>
            <div class="logo-text">BondPay Station</div>
        </div>
        <div class="status-badge">
            <div class="status-dot"></div>
            <span id="station-clock">Online</span>
        </div>
    </header>

    <!-- Main Content Wrapper -->
    <div class="app-container">
        <!-- Sidebar Navigation -->
        <sidebar>
            <button class="nav-btn active" onclick="switchTab('dashboard')">
                <svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="9" rx="1"/><rect x="14" y="3" width="7" height="5" rx="1"/><rect x="14" y="12" width="7" height="9" rx="1"/><rect x="3" y="16" width="7" height="5" rx="1"/></svg>
                <span>Dashboard</span>
            </button>
            <button class="nav-btn" onclick="switchTab('cards')">
                <svg viewBox="0 0 24 24"><rect x="2" y="5" width="20" height="14" rx="2"/><line x1="2" y1="10" x2="22" y2="10"/></svg>
                <span>Cards</span>
            </button>
            <button class="nav-btn" onclick="switchTab('payments')">
                <svg viewBox="0 0 24 24"><line x1="12" y1="1" x2="12" y2="23"/><path d="M17 5H9.5a3.5 3.5 0 0 0 0 7h5a3.5 3.5 0 0 1 0 7H6"/></svg>
                <span>Payments</span>
            </button>
            <button class="nav-btn" onclick="switchTab('transactions')">
                <svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><polyline points="12 6 12 12 16 14"/></svg>
                <span>Transactions</span>
            </button>
            <button class="nav-btn" onclick="switchTab('settings')">
                <svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
                <span>Settings</span>
            </button>
        </sidebar>

        <!-- Main Content Area -->
        <main class="content">

            <!-- TAB 1: DASHBOARD -->
            <div id="tab-dashboard" class="tab-panel active">
                <h1 class="page-title" id="station-name-display">BondPay Station</h1>
                <p class="page-subtitle">Welcome to offline terminal management dashboard.</p>
                
                <div class="stat-grid">
                    <div class="stat-card">
                        <div class="stat-title">Total Active Cards</div>
                        <div class="stat-val" id="stat-cards-count">0</div>
                    </div>
                    <div class="stat-card success">
                        <div class="stat-title">Total Local Balance</div>
                        <div class="stat-val" id="stat-balance-total">NPR 0.00</div>
                    </div>
                    <div class="stat-card error">
                        <div class="stat-title">Pending Sync</div>
                        <div class="stat-val" id="stat-pending-sync">0</div>
                    </div>
                </div>

                <div class="card-row">
                    <div class="form-card">
                        <h2 style="font-size: 1.2rem; margin-bottom: 1.25rem; font-weight: 600;">Recent Transactions</h2>
                        <div class="table-container">
                            <table>
                                <thead>
                                    <tr>
                                        <th>Timestamp</th>
                                        <th>Name</th>
                                        <th>Amount</th>
                                        <th>Remaining</th>
                                        <th>Status</th>
                                    </tr>
                                </thead>
                                <tbody id="recent-transactions-list">
                                    <tr><td colspan="5" style="text-align: center; color: var(--text-muted);">No transactions recorded.</td></tr>
                                </tbody>
                            </table>
                        </div>
                    </div>
                    <div>
                        <div class="form-card" style="display: flex; flex-direction: column; gap: 1rem;">
                            <h2 style="font-size: 1.2rem; font-weight: 600;">Simulated Sync</h2>
                            <p style="font-size: 0.9rem; color: var(--text-muted); line-height: 1.4;">
                                BondPay Station queues transaction logs locally when server connection is down. Use local sync to finalize database uploads.
                            </p>
                            <button class="btn btn-primary btn-full" onclick="syncTransactions()">Sync Queue Now</button>
                        </div>
                    </div>
                </div>
            </div>

            <!-- TAB 2: CARDS -->
            <div id="tab-cards" class="tab-panel">
                <h1 class="page-title">Cardholder Management</h1>
                <p class="page-subtitle">Add, remove, and manage local customer RFID cards.</p>

                <div class="card-row">
                    <div class="form-card">
                        <h2 style="font-size: 1.2rem; margin-bottom: 1.25rem; font-weight: 600;">Registered Cards</h2>
                        <div class="table-container">
                            <table>
                                <thead>
                                    <tr>
                                        <th>Name</th>
                                        <th>User ID</th>
                                        <th>Card UID</th>
                                        <th>Balance</th>
                                        <th>Actions</th>
                                    </tr>
                                </thead>
                                <tbody id="cards-list">
                                    <tr><td colspan="5" style="text-align: center; color: var(--text-muted);">No cards registered.</td></tr>
                                </tbody>
                            </table>
                        </div>
                    </div>
                    <div>
                        <div class="form-card">
                            <h2 style="font-size: 1.2rem; margin-bottom: 1.25rem; font-weight: 600;">Add Card</h2>
                            <form id="add-card-form" onsubmit="startRegisterCard(event)">
                                <div class="form-group">
                                    <label class="form-label">Customer Name</label>
                                    <input type="text" class="form-input" id="reg-name" required placeholder="e.g. Sakshyam">
                                </div>
                                <div class="form-group">
                                    <label class="form-label">User ID</label>
                                    <input type="text" class="form-input" id="reg-userid" required placeholder="e.g. BP001">
                                </div>
                                <div class="form-group">
                                    <label class="form-label">Initial Balance (NPR)</label>
                                    <input type="number" class="form-input" id="reg-balance" required min="0" value="1000" step="0.01">
                                </div>
                                <button type="submit" class="btn btn-primary btn-full">Register Card</button>
                            </form>
                        </div>
                    </div>
                </div>
            </div>

            <!-- TAB 3: PAYMENTS -->
            <div id="tab-payments" class="tab-panel">
                <h1 class="page-title">Offline RFID Payment</h1>
                <p class="page-subtitle">Initiate transactions via local balance deductions.</p>

                <div class="card-row" style="grid-template-columns: 1fr; max-width: 500px; margin: 0 auto;">
                    <div class="form-card" style="text-align: center;">
                        <div class="scanner-visual" style="margin-top: 1rem; margin-bottom: 2rem;">
                            <svg viewBox="0 0 24 24">
                                <rect x="2" y="5" width="20" height="14" rx="2"/><line x1="2" y1="10" x2="22" y2="10"/>
                            </svg>
                        </div>
                        <h2 style="font-size: 1.4rem; margin-bottom: 0.5rem; font-weight: 700;">Initiate Payment</h2>
                        <p style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 2rem;">
                            Enter deduction amount to put terminal in listening mode.
                        </p>
                        <form id="payment-form" onsubmit="startPayment(event)">
                            <div class="form-group" style="text-align: left;">
                                <label class="form-label">Deduction Amount (NPR)</label>
                                <input type="number" class="form-input" id="pay-amount" style="font-size: 1.5rem; text-align: center; height: 3.5rem;" required min="0.01" step="0.01" placeholder="0.00">
                            </div>
                            <button type="submit" class="btn btn-primary btn-full" style="height: 3rem; font-size: 1.1rem;">Start Payment</button>
                        </form>
                    </div>
                </div>
            </div>

            <!-- TAB 4: TRANSACTIONS -->
            <div id="tab-transactions" class="tab-panel">
                <h1 class="page-title">Transaction Log</h1>
                <p class="page-subtitle">Search, filter, and review historical offline station audits.</p>

                <div class="form-card">
                    <div class="search-bar">
                        <input type="text" class="form-input" id="txn-search" oninput="filterTransactions()" placeholder="Search by name, UID, status...">
                        <button class="btn btn-outline" onclick="loadData()">Refresh</button>
                    </div>
                    <div class="table-container">
                        <table>
                            <thead>
                                <tr>
                                    <th>ID</th>
                                    <th>Timestamp</th>
                                    <th>Card UID</th>
                                    <th>Name</th>
                                    <th>Amount</th>
                                    <th>Prev Bal</th>
                                    <th>Remaining Bal</th>
                                    <th>Status</th>
                                </tr>
                            </thead>
                            <tbody id="all-transactions-list">
                                <tr><td colspan="8" style="text-align: center; color: var(--text-muted);">No records found.</td></tr>
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>

            <!-- TAB 5: SETTINGS -->
            <div id="tab-settings" class="tab-panel">
                <h1 class="page-title">Station Configuration</h1>
                <p class="page-subtitle">Configure hardware configurations and local databases.</p>

                <div class="card-row">
                    <div class="form-card">
                        <h2 style="font-size: 1.2rem; margin-bottom: 1.25rem; font-weight: 600;">System Settings</h2>
                        <form id="settings-form" onsubmit="saveSettingsForm(event)">
                            <div class="form-group">
                                <label class="form-label">Station Name</label>
                                <input type="text" class="form-input" id="settings-station-name" required value="BondPay Station 1">
                            </div>
                            <button type="submit" class="btn btn-primary">Save Settings</button>
                        </form>
                        
                        <div style="margin-top: 2rem; border-top: 1px solid var(--border-color); padding-top: 2rem;">
                            <h3 style="font-size: 1rem; color: var(--error); margin-bottom: 0.5rem; font-weight: 600;">Danger Zone</h3>
                            <p style="font-size: 0.9rem; color: var(--text-muted); margin-bottom: 1rem;">
                                Once you purge cards and transaction data, the station database is permanently wiped.
                            </p>
                            <div style="display: flex; gap: 0.75rem;">
                                <button class="btn btn-danger" onclick="clearTransactionsData()">Clear Transaction Logs</button>
                                <button class="btn btn-danger btn-outline" onclick="factoryResetDevice()">Reset All Databases</button>
                            </div>
                        </div>
                    </div>
                    <div>
                        <div class="form-card">
                            <h2 style="font-size: 1.2rem; margin-bottom: 1.25rem; font-weight: 600;">Terminal Info</h2>
                            <div style="display: flex; flex-direction: column; gap: 0.75rem; font-size: 0.9rem;">
                                <div style="display: flex; justify-content: space-between;">
                                    <span style="color: var(--text-muted);">SSID:</span>
                                    <strong>BondPay</strong>
                                </div>
                                <div style="display: flex; justify-content: space-between;">
                                    <span style="color: var(--text-muted);">IP Address:</span>
                                    <strong>192.168.4.1</strong>
                                </div>
                                <div style="display: flex; justify-content: space-between;">
                                    <span style="color: var(--text-muted);">Memory Free:</span>
                                    <strong id="sys-free-heap">-- KB</strong>
                                </div>
                                <div style="display: flex; justify-content: space-between;">
                                    <span style="color: var(--text-muted);">Uptime:</span>
                                    <strong id="sys-uptime">--</strong>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

        </main>
    </div>

    <!-- Active Action Overlay Modal -->
    <div id="action-overlay" class="overlay">
        <div class="modal" id="overlay-modal-content">
            <!-- Dynamically populated -->
        </div>
    </div>

    <script>
        let currentTab = 'dashboard';
        let isPolling = false;
        let originalTransactions = [];
        let rtcSynced = false;

        // Auto tab load
        window.onload = function() {
            switchTab('dashboard');
            syncTime();
            loadData();
            startPollingStatus();
            
            // Periodically refresh data
            setInterval(() => {
                if (currentTab === 'dashboard') loadData();
            }, 5000);
        };

        // Navigation
        function switchTab(tabId) {
            document.querySelectorAll('.tab-panel').forEach(panel => {
                panel.classList.remove('active');
            });
            document.querySelectorAll('.nav-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            
            document.getElementById(`tab-${tabId}`).classList.add('active');
            const targetBtn = Array.from(document.querySelectorAll('.nav-btn')).find(btn => 
                btn.getAttribute('onclick').includes(tabId)
            );
            if (targetBtn) targetBtn.classList.add('active');
            
            currentTab = tabId;
            loadData();
        }

        // Sync Epoch Time to ESP8266
        function syncTime() {
            const epoch = Math.floor(Date.now() / 1000);
            fetch(`/api/time?epoch=${epoch}`, { method: 'POST' })
                .then(res => {
                    rtcSynced = true;
                    updateClock();
                });
        }

        function updateClock() {
            const now = new Date();
            document.getElementById('station-clock').innerText = now.toLocaleTimeString();
            setTimeout(updateClock, 1000);
        }

        // Load Data from API
        function loadData() {
            fetch('/api/status')
                .valueOrNull = null; // helper
            
            // Get stats & system values
            fetch('/api/status')
                .then(res => res.json())
                .then(data => {
                    document.getElementById('stat-pending-sync').innerText = data.pendingSyncCount || 0;
                    document.getElementById('sys-free-heap').innerText = Math.round(data.freeHeap / 1024) + ' KB';
                    
                    // Format uptime
                    let seconds = data.uptime / 1000;
                    let hrs = Math.floor(seconds / 3600);
                    let mins = Math.floor((seconds % 3600) / 60);
                    document.getElementById('sys-uptime').innerText = `${hrs}h ${mins}m`;
                });

            // Get cards
            fetch('/api/cards')
                .then(res => res.json())
                .then(cards => {
                    document.getElementById('stat-cards-count').innerText = cards.length;
                    
                    let totalBalance = cards.reduce((sum, c) => sum + parseFloat(c.balance || 0), 0);
                    document.getElementById('stat-balance-total').innerText = 'NPR ' + totalBalance.toFixed(2);
                    
                    renderCardsTable(cards);
                })
                .catch(err => console.log('Error loading cards', err));

            // Get transactions
            fetch('/api/transactions')
                .then(res => res.json())
                .then(txns => {
                    originalTransactions = txns;
                    renderTransactionsTable(txns);
                    renderRecentTransactions(txns);
                })
                .catch(err => console.log('Error loading transactions', err));
        }

        function renderCardsTable(cards) {
            const tbody = document.getElementById('cards-list');
            if (cards.length === 0) {
                tbody.innerHTML = `<tr><td colspan="5" style="text-align: center; color: var(--text-muted);">No cards registered.</td></tr>`;
                return;
            }
            
            tbody.innerHTML = cards.map(c => `
                <tr>
                    <td><strong>${escapeHTML(c.name)}</strong></td>
                    <td><code>${escapeHTML(c.userId)}</code></td>
                    <td><code>${escapeHTML(c.uid)}</code></td>
                    <td><strong>NPR ${parseFloat(c.balance).toFixed(2)}</strong></td>
                    <td style="display: flex; gap: 0.5rem;">
                        <button class="btn btn-outline" style="padding: 0.35rem 0.75rem; font-size: 0.8rem;" onclick="promptUpdateBalance('${c.uid}', ${c.balance})">Edit</button>
                        <button class="btn btn-danger btn-outline" style="padding: 0.35rem 0.75rem; font-size: 0.8rem;" onclick="deleteCard('${c.uid}')">Delete</button>
                    </td>
                </tr>
            `).join('');
        }

        function renderTransactionsTable(txns) {
            const tbody = document.getElementById('all-transactions-list');
            if (txns.length === 0) {
                tbody.innerHTML = `<tr><td colspan="8" style="text-align: center; color: var(--text-muted);">No records found.</td></tr>`;
                return;
            }
            
            // Sort by id descending (newest first)
            const sorted = [...txns].reverse();
            
            tbody.innerHTML = sorted.map(t => {
                const statusClass = t.status.toLowerCase().includes('success') ? 'badge-success' : 'badge-danger';
                return `
                    <tr>
                        <td><code>#${t.id}</code></td>
                        <td>${escapeHTML(t.timestamp)}</td>
                        <td><code>${escapeHTML(t.uid)}</code></td>
                        <td>${escapeHTML(t.name)}</td>
                        <td><strong>NPR ${parseFloat(t.amount).toFixed(2)}</strong></td>
                        <td>NPR ${parseFloat(t.prevBal).toFixed(2)}</td>
                        <td>NPR ${parseFloat(t.remBal).toFixed(2)}</td>
                        <td><span class="badge ${statusClass}">${escapeHTML(t.status)}</span></td>
                    </tr>
                `;
            }).join('');
        }

        function renderRecentTransactions(txns) {
            const tbody = document.getElementById('recent-transactions-list');
            if (txns.length === 0) {
                tbody.innerHTML = `<tr><td colspan="5" style="text-align: center; color: var(--text-muted);">No transactions.</td></tr>`;
                return;
            }
            
            // Last 5 txns
            const recent = [...txns].reverse().slice(0, 5);
            
            tbody.innerHTML = recent.map(t => {
                const statusClass = t.status.toLowerCase().includes('success') ? 'badge-success' : 'badge-danger';
                return `
                    <tr>
                        <td>${escapeHTML(t.timestamp.split(' ')[1] || t.timestamp)}</td>
                        <td>${escapeHTML(t.name)}</td>
                        <td><strong>NPR ${parseFloat(t.amount).toFixed(2)}</strong></td>
                        <td>NPR ${parseFloat(t.remBal).toFixed(2)}</td>
                        <td><span class="badge ${statusClass}">${escapeHTML(t.status)}</span></td>
                    </tr>
                `;
            }).join('');
        }

        // Polling loop for hardware scans
        function startPollingStatus() {
            if (isPolling) return;
            isPolling = true;
            
            const poll = () => {
                fetch('/api/status')
                    .then(res => res.json())
                    .then(data => {
                        handleESPState(data);
                        setTimeout(poll, 800);
                    })
                    .catch(err => {
                        console.log('Polling failure', err);
                        setTimeout(poll, 2000);
                    });
            };
            poll();
        }

        function handleESPState(data) {
            const overlay = document.getElementById('action-overlay');
            const modal = document.getElementById('overlay-modal-content');
            
            // Check if ESP is in active scan mode
            if (data.mode === 'PAYMENT') {
                overlay.classList.add('active');
                modal.innerHTML = `
                    <div class="scanner-visual">
                        <div class="scanner-ring"></div>
                        <svg viewBox="0 0 24 24">
                            <rect x="2" y="5" width="20" height="14" rx="2"/><line x1="2" y1="10" x2="22" y2="10"/>
                        </svg>
                    </div>
                    <h2 style="font-size: 1.4rem; margin-bottom: 0.5rem; font-weight: 700;">Waiting for Card</h2>
                    <p style="font-size: 1.1rem; color: var(--primary); font-weight: bold; margin-bottom: 0.5rem;">Amount: NPR ${parseFloat(data.activeAmount).toFixed(2)}</p>
                    <p style="font-size: 0.95rem; color: var(--text-muted); margin-bottom: 2rem;">Tap customer RFID wallet against the terminal reader.</p>
                    <button class="btn btn-outline" onclick="cancelActiveAction('payment')">Cancel Payment</button>
                `;
            } else if (data.mode === 'ADD_CARD') {
                overlay.classList.add('active');
                modal.innerHTML = `
                    <div class="scanner-visual">
                        <div class="scanner-ring"></div>
                        <svg viewBox="0 0 24 24">
                            <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z"/>
                        </svg>
                    </div>
                    <h2 style="font-size: 1.4rem; margin-bottom: 0.5rem; font-weight: 700;">Scan New Card</h2>
                    <p style="font-size: 1.1rem; color: var(--primary); font-weight: bold; margin-bottom: 0.5rem;">User ID: ${escapeHTML(data.activeUserId)}</p>
                    <p style="font-size: 0.95rem; color: var(--text-muted); margin-bottom: 2rem;">Tap clean unallocated RFID card now to assign credentials.</p>
                    <button class="btn btn-outline" onclick="cancelActiveAction('cards')">Cancel Registration</button>
                `;
            } else {
                // If ESP is READY, check if we need to show temporary scan results
                if (data.lastEvent && data.lastEvent.processed === false) {
                    showScanResult(data.lastEvent);
                } else if (!overlay.querySelector('.result-screen')) {
                    // Only hide if we aren't showing a temporary result screen
                    overlay.classList.remove('active');
                }
            }
        }

        // Show Temporary Success/Failure Screen
        function showScanResult(event) {
            const overlay = document.getElementById('action-overlay');
            const modal = document.getElementById('overlay-modal-content');
            overlay.classList.add('active');
            
            const isSuccess = event.status.toLowerCase().includes('success');
            
            if (isSuccess) {
                modal.innerHTML = `
                    <div class="result-screen">
                        <div class="success-checkmark">
                            <svg style="width:2.5rem; height:2.5rem;" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3">
                                <polyline points="20 6 9 17 4 12"/>
                            </svg>
                        </div>
                        <h2 style="font-size: 1.5rem; color: var(--success); margin-bottom: 0.5rem; font-weight: 700;">Success</h2>
                        <p style="font-size: 1.1rem; font-weight: 600; margin-bottom: 0.25rem;">${escapeHTML(event.name || '')}</p>
                        <p style="font-size: 0.95rem; color: var(--text-muted); margin-bottom: 1.5rem;">UID: <code>${escapeHTML(event.uid)}</code></p>
                        <div style="background: rgba(255,255,255,0.02); border: 1px solid var(--border-color); padding: 1rem; border-radius: 8px; margin-bottom: 1.5rem; text-align: left;">
                            <div style="display:flex; justify-content:space-between; margin-bottom:0.5rem;">
                                <span>Charged:</span><strong>NPR ${parseFloat(event.amount || 0).toFixed(2)}</strong>
                            </div>
                            <div style="display:flex; justify-content:space-between;">
                                <span>Balance Remaining:</span><strong>NPR ${parseFloat(event.remBal || 0).toFixed(2)}</strong>
                            </div>
                        </div>
                        <p style="font-size: 0.8rem; color: var(--text-muted);">Auto-closing in 3 seconds...</p>
                    </div>
                `;
            } else {
                modal.innerHTML = `
                    <div class="result-screen">
                        <div class="failure-cross">
                            <svg style="width:2.5rem; height:2.5rem;" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3">
                                <line x1="18" y1="6" x2="6" y2="18"/><line x1="6" y1="6" x2="18" y2="18"/>
                            </svg>
                        </div>
                        <h2 style="font-size: 1.5rem; color: var(--error); margin-bottom: 0.5rem; font-weight: 700;">Payment Failed</h2>
                        <p style="font-size: 1.1rem; font-weight: 600; margin-bottom: 1.5rem;">${escapeHTML(event.message || 'Transaction Declined')}</p>
                        <button class="btn btn-outline btn-full" onclick="closeOverlay()">Close</button>
                    </div>
                `;
            }
            
            // Mark event as processed on the backend so it doesn't loop
            fetch('/api/status/acknowledge-event', { method: 'POST' });
            
            // Auto close after 3.5 seconds
            setTimeout(() => {
                closeOverlay();
            }, 3500);
        }

        function closeOverlay() {
            document.getElementById('action-overlay').classList.remove('active');
            loadData();
        }

        // Actions
        function startPayment(e) {
            e.preventDefault();
            const amount = document.getElementById('pay-amount').value;
            if (amount <= 0) return alert('Please enter a valid amount.');
            
            fetch(`/api/payment/start?amount=${amount}`, { method: 'POST' })
                .then(res => {
                    document.getElementById('pay-amount').value = '';
                });
        }

        function startRegisterCard(e) {
            e.preventDefault();
            const name = document.getElementById('reg-name').value;
            const userId = document.getElementById('reg-userid').value;
            const balance = document.getElementById('reg-balance').value;
            
            fetch(`/api/cards/add?name=${encodeURIComponent(name)}&userId=${encodeURIComponent(userId)}&balance=${balance}`, { method: 'POST' })
                .then(res => {
                    document.getElementById('reg-name').value = '';
                    document.getElementById('reg-userid').value = '';
                    document.getElementById('reg-balance').value = '1000';
                });
        }

        function cancelActiveAction(type) {
            const url = type === 'payment' ? '/api/payment/cancel' : '/api/cards/cancel';
            fetch(url, { method: 'POST' }).then(() => {
                closeOverlay();
            });
        }

        function promptUpdateBalance(uid, currentBal) {
            const newBal = prompt(`Enter new balance (NPR) for card UID ${uid}:`, currentBal);
            if (newBal === null) return;
            
            const balVal = parseFloat(newBal);
            if (isNaN(balVal) || balVal < 0) return alert('Invalid balance amount.');
            
            fetch(`/api/cards/update-balance?uid=${uid}&balance=${balVal}`, { method: 'POST' })
                .then(res => {
                    if (res.ok) {
                        alert('Balance updated successfully.');
                        loadData();
                    } else {
                        alert('Update failed.');
                    }
                });
        }

        function deleteCard(uid) {
            if (!confirm(`Are you sure you want to delete card UID: ${uid}?`)) return;
            
            fetch(`/api/cards/delete?uid=${uid}`, { method: 'POST' })
                .then(res => {
                    loadData();
                });
        }

        function syncTransactions() {
            fetch('/api/transactions/sync', { method: 'POST' })
                .then(res => {
                    alert('Simulated transaction queue synchronization successful.');
                    loadData();
                });
        }

        function clearTransactionsData() {
            if (!confirm('Are you sure you want to completely clear the transaction logs?')) return;
            fetch('/api/transactions/clear', { method: 'POST' })
                .then(() => {
                    alert('Transaction logs cleared.');
                    loadData();
                });
        }

        function factoryResetDevice() {
            if (!confirm('WARNING: This will delete all cards, settings, and transactions. Proceed?')) return;
            fetch('/api/settings/factory-reset', { method: 'POST' })
                .then(() => {
                    alert('Station factory reset complete. ESP will restart.');
                    location.reload();
                });
        }

        function saveSettingsForm(e) {
            e.preventDefault();
            const stationName = document.getElementById('settings-station-name').value;
            fetch(`/api/settings?stationName=${encodeURIComponent(stationName)}`, { method: 'POST' })
                .then(res => {
                    alert('Settings saved.');
                    document.getElementById('station-name-display').innerText = stationName;
                });
        }

        function filterTransactions() {
            const query = document.getElementById('txn-search').value.toLowerCase();
            const filtered = originalTransactions.filter(t => 
                t.uid.toLowerCase().includes(query) ||
                t.name.toLowerCase().includes(query) ||
                t.status.toLowerCase().includes(query) ||
                t.timestamp.toLowerCase().includes(query)
            );
            renderTransactionsTable(filtered);
        }

        // HTML escaping helper to prevent XSS
        function escapeHTML(str) {
            return str.replace(/[&<>'"]/g, 
                tag => ({
                    '&': '&amp;',
                    '<': '&lt;',
                    '>': '&gt;',
                    "'": '&#39;',
                    '"': '&quot;'
                }[tag] || tag)
            );
        }
    </script>
</body>
</html>
)rawliteral";

#endif
