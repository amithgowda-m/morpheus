#!/usr/bin/env python3
"""
Quick start information for Morpheus Dashboard

Usage: python README_DASHBOARD.py
"""

README = """
╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║               🎯 MORPHEUS INTERACTIVE BENCHMARK DASHBOARD 🎯              ║
║                                                                            ║
║                   Real-Time Performance Monitoring System                 ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝

QUICK START (2 minutes)
═══════════════════════════════════════════════════════════════════════════════

1. Start the Dashboard Server
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   
   $ cd /home/amithgowda/morpheus/python
   $ python dashboard_server.py
   
   You'll see:
   INFO - Starting Morpheus Dashboard Server on http://localhost:8080
   INFO - Open your browser to http://localhost:8080

2. Open Dashboard
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   
   Open your web browser to: http://localhost:8080
   
   You'll see 4 interactive visualizations:
   • Live Speedup Gauge (D3.js animated dial)
   • Performance Timeline (24-hour history)
   • Cache Efficiency Radar (5-axis comparison)
   • Algorithm Comparison (bar chart with breakdown)

3. Stream Benchmark Results
   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   
   In another terminal:
   
   $ python dashboard_examples.py
   
   Choose an example (1-6) to see data streaming live.

WHAT YOU GET
═══════════════════════════════════════════════════════════════════════════════

✅ 4 Professional Visualizations

   1. LIVE SPEEDUP GAUGE
      • D3.js animated gauge with color zones
      • Green: 1.0-1.5× speedup
      • Yellow: 1.5-2.0× speedup
      • Red: >2.0× speedup
      • Real-time needle animation
      • Current/target/best values

   2. PERFORMANCE TIMELINE CHART
      • Chart.js line chart
      • 24-hour execution time history
      • Multiple algorithms overlaid
      • Tooltip: hover for exact values
      • Interactive legend: click to show/hide
      • Zoom & pan: drag to explore trends

   3. CACHE EFFICIENCY RADAR
      • 5-dimensional radar comparison
      • Axes: L1/L2/L3 Hit Rate, IPC, Branch Accuracy
      • Morpheus vs Baseline overlay
      • Larger polygon = better performance
      • Great for publications

   4. ALGORITHM COMPARISON BAR CHART
      • Grouped bars: Baseline vs Morpheus
      • Three algorithms: BFS, PageRank, Betweenness
      • Click bars for detailed breakdown
      • Shows speedup, improvement %, significance
      • Color-coded confidence intervals

✅ Summary Statistics Table

   Complete statistical analysis for each algorithm:
   • Baseline and Morpheus execution times (±std dev)
   • Speedup factor and 95% confidence interval
   • p-value (statistical significance)
   • Cohen's d (effect size)

✅ Real-Time Updates

   • WebSocket connection for live data
   • Updates every 5 seconds (configurable)
   • Auto-reconnect on connection loss
   • Visual connection indicator in header

✅ Professional Features

   • Light/Dark theme toggle
   • Responsive design (desktop, tablet, mobile)
   • Export data as JSON
   • Keyboard shortcuts (Ctrl+R refresh, Ctrl+T theme)
   • Publication-ready colors (colorblind-friendly)
   • No external dependencies (besides Chart.js/D3.js)

FILE LOCATIONS
═══════════════════════════════════════════════════════════════════════════════

dashboard.html
   └─ Frontend dashboard interface
   └─ D3.js gauge + Chart.js visualizations
   └─ 4 interactive charts
   └─ Light/dark theme support
   └─ ~800 lines of code + CSS

dashboard_server.py
   └─ Backend server (aiohttp)
   └─ WebSocket support
   └─ REST API endpoints
   └─ Real-time data broadcasting
   └─ ~400 lines of code

dashboard_examples.py
   └─ Integration examples
   └─ 6 example scripts (simple to advanced)
   └─ Streaming benchmark results
   └─ Load from JSON files
   └─ Continuous monitoring

DASHBOARD_GUIDE.md
   └─ Complete user guide
   └─ How to use each visualization
   └─ Data interpretation
   └─ Real-time updates explained
   └─ Customization options
   └─ Troubleshooting

README_DASHBOARD.py (this file)
   └─ Quick start information

FEATURES COMPARED TO STATIC FIGURES
═══════════════════════════════════════════════════════════════════════════════

Static Figures (acm_publication_figures.py):
   ✓ Publication-quality PDFs
   ✓ 300 DPI, embedded fonts
   ✓ Perfect for papers
   ✓ One-time snapshot

Interactive Dashboard (dashboard.html):
   ✓ Real-time updates
   ✓ Zoom and pan
   ✓ Interactive legends
   ✓ Live data streaming
   ✓ Multiple visualization types
   ✓ Export capability
   ✓ Great for presentations
   ✓ Useful during development
   ✓ Monitor benchmarks as they run

USE CASES
═══════════════════════════════════════════════════════════════════════════════

RESEARCH & DEVELOPMENT
   • Monitor benchmark progress in real-time
   • Identify performance issues immediately
   • Test different algorithms side-by-side
   • Share results with team members

PRESENTATIONS
   • Display live benchmarks during talk
   • Interactive exploration of results
   • Professional appearance
   • Light theme for projectors

PAPER WRITING
   • Export publication figures (see acm_publication_figures.py)
   • Use dashboard during development
   • Verify results before submission

CONTINUOUS MONITORING
   • Run benchmarks and stream results
   • Track performance over time
   • Detect regressions quickly
   • Archive historical data

API ENDPOINTS
═══════════════════════════════════════════════════════════════════════════════

GET /
   └─ Serve dashboard HTML

GET /api/data
   └─ Get current benchmark data (JSON)

POST /api/update
   └─ Update benchmark result
   └─ Example: {"algorithm": "BFS", "morpheus_ms": 195}

GET /api/history
   └─ Get historical data
   └─ Optional: ?limit=100

WebSocket /ws
   └─ Real-time updates via WebSocket
   └─ Receive broadcasts when data changes

COMMAND-LINE USAGE
═══════════════════════════════════════════════════════════════════════════════

Start server on default port (8080):
   $ python dashboard_server.py

Start on custom port:
   $ python dashboard_server.py --port 9000

Start on different host (accessible from network):
   $ python dashboard_server.py --host 0.0.0.0 --port 8080

Update benchmark via CLI:
   $ python dashboard_server.py --update \\
       --algorithm BFS \\
       --morpheus-time 195 \\
       --baseline-time 245

Run examples:
   $ python dashboard_examples.py

INTEGRATION WITH YOUR BENCHMARKS
═══════════════════════════════════════════════════════════════════════════════

Option 1: REST API (HTTP POST)

   import asyncio
   import aiohttp
   
   async def report_benchmark():
       async with aiohttp.ClientSession() as session:
           await session.post(
               'http://localhost:8080/api/update',
               json={
                   'algorithm': 'BFS',
                   'baseline_ms': 250,
                   'morpheus_ms': 198,
                   'morpheus_std': 12,
                   'baseline_std': 15
               }
           )
   
   asyncio.run(report_benchmark())

Option 2: WebSocket (Real-time streaming)

   import asyncio
   import websockets
   import json
   
   async def stream():
       async with websockets.connect('ws://localhost:8080/ws') as ws:
           msg = {
               'type': 'update',
               'algorithm': 'PageRank',
               'morpheus_ms': 250
           }
           await ws.send(json.dumps(msg))
   
   asyncio.run(stream())

Option 3: Shell/Curl

   curl -X POST http://localhost:8080/api/update \\
       -H "Content-Type: application/json" \\
       -d '{"algorithm":"BFS","morpheus_ms":195}'

BROWSER COMPATIBILITY
═══════════════════════════════════════════════════════════════════════════════

✅ Chrome 90+        (Recommended)
✅ Firefox 88+       (Excellent)
✅ Safari 14+        (Good)
✅ Edge 90+          (Chromium-based)
⚠️  Mobile browsers  (Touch interactions may differ)

PERFORMANCE
═══════════════════════════════════════════════════════════════════════════════

• Dashboard load time: < 1 second
• WebSocket latency: < 100 ms
• Chart update time: < 500 ms
• Memory usage: ~ 50 MB
• CPU usage: < 5% idle

KEYBOARD SHORTCUTS
═══════════════════════════════════════════════════════════════════════════════

Ctrl+R          Refresh data
Ctrl+T          Toggle theme (dark/light)
F11             Full screen
Ctrl+P          Print / Save as PDF
F12             Developer console (troubleshooting)

TROUBLESHOOTING
═══════════════════════════════════════════════════════════════════════════════

Q: Dashboard won't load at http://localhost:8080

A: Make sure the server is running in another terminal:
   $ python dashboard_server.py
   
   Also verify no other app is using port 8080.

Q: WebSocket connection drops

A: This is normal in long sessions - it auto-reconnects.
   Check F12 → Console for error messages.

Q: Charts not showing data

A: Click the 🔄 Refresh button to manually update.
   If still blank, check server logs for errors.

Q: Port 8080 already in use

A: Use a different port:
   $ python dashboard_server.py --port 9000
   
   Then open: http://localhost:9000

Q: Slow performance / lag

A: Try Chrome or Firefox instead of Safari.
   Disable browser extensions.
   Check your network connection.

NEXT STEPS
═══════════════════════════════════════════════════════════════════════════════

1. ✅ Start the dashboard
   $ python dashboard_server.py

2. 🌐 Open in browser
   http://localhost:8080

3. 📊 Try the examples
   $ python dashboard_examples.py

4. 📖 Read the full guide
   See DASHBOARD_GUIDE.md

5. 🔗 Integrate with your code
   See Integration examples above

6. 🎯 Stream live benchmarks
   Modify dashboard_examples.py for your benchmarks

DOCUMENTATION
═══════════════════════════════════════════════════════════════════════════════

Quick start (this file):
   README_DASHBOARD.py / README_DASHBOARD.md

Complete user guide:
   DASHBOARD_GUIDE.md
   
   Sections:
   • Dashboard components overview
   • How to interpret each visualization
   • Real-time update mechanisms
   • Customization options
   • Advanced usage (WebSocket streaming)
   • Architecture and data flow
   • Troubleshooting

Examples:
   dashboard_examples.py
   
   6 examples from simple to advanced:
   1. Single result update
   2. Multiple algorithms
   3. With cache metrics
   4. Live streaming simulation
   5. Load from JSON
   6. Continuous monitoring

Source code:
   dashboard.html (900 lines)
   └─ Frontend: D3.js + Chart.js visualizations
   
   dashboard_server.py (400 lines)
   └─ Backend: aiohttp server + WebSocket

SUPPORT FILES
═══════════════════════════════════════════════════════════════════════════════

For publication figures (static PDF):
   See acm_publication_figures.py

For statistical analysis:
   See benchmark_parser.py and speedup_analysis.py

For complete documentation:
   See README_PUBLICATION_FIGURES.md

═════════════════════════════════════════════════════════════════════════════════

📧 Questions? See DASHBOARD_GUIDE.md for comprehensive documentation.

🚀 Ready to get started? Run:
   python dashboard_server.py

═════════════════════════════════════════════════════════════════════════════════
"""

if __name__ == '__main__':
    print(README)
