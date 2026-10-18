#pragma once

#include <Arduino.h>

/*
 * Smart Greenhouse Controller
 * Dashboard UI resources
 *
 * Static HTML and CSS are stored in flash (PROGMEM) instead of being kept
 * inside web_server.cpp. Browser-side live-update logic is stored separately
 * in dashboard_script.h and served as /dashboard.js.
 */

static const char DASHBOARD_PAGE_START[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Smart Greenhouse</title>

<style>
body{
margin:0;
font-family:Arial,sans-serif;
background:#f4f7f5;
color:#1f2933;
}

.container{
max-width:900px;
margin:auto;
padding:20px;
}

header{
background:#ffffff;
padding:22px;
border-radius:14px;
margin-bottom:18px;
box-shadow:0 2px 10px rgba(0,0,0,.08);
}

h1{
margin:0 0 6px 0;
font-size:28px;
}

.subtitle{
color:#667085;
}

.grid{
display:grid;
grid-template-columns:repeat(auto-fit,minmax(190px,1fr));
gap:14px;
margin-bottom:18px;
}

.card{
background:#ffffff;
padding:18px;
border-radius:14px;
box-shadow:0 2px 10px rgba(0,0,0,.07);
}

.label{
font-size:13px;
color:#667085;
margin-bottom:8px;
}

.value{
font-size:25px;
font-weight:bold;
}

.section-title{
font-size:18px;
font-weight:bold;
margin:22px 0 10px;
}

.status{
font-weight:bold;
}

.footer{
text-align:center;
color:#667085;
font-size:12px;
padding:18px;
}

.controls{
display:flex;
gap:10px;
flex-wrap:wrap;
}

.btn{
border:0;
border-radius:10px;
padding:12px 18px;
font-size:16px;
font-weight:bold;
cursor:pointer;
}

.btn-primary{
background:#1f2933;
color:white;
}

.btn-secondary{
background:#e5e7eb;
color:#1f2933;
}

.btn-danger{
background:#b42318;
color:white;
}

.btn-success{
background:#067647;
color:white;
}

.btn:disabled{
opacity:.4;
cursor:not-allowed;
}

form{
margin:0;
}

.state-badge{
display:inline-block;
padding:8px 16px;
border-radius:999px;
font-size:22px;
font-weight:bold;
}

.state-on{
background:#d1fae5;
color:#067647;
}

.state-off{
background:#e5e7eb;
color:#344054;
}

.btn-active-on{
background:#067647;
color:white;
}

.btn-active-off{
background:#344054;
color:white;
}

.btn-inactive{
background:#e5e7eb;
color:#667085;
}

/*
 * Growth-stage buttons are wider than the standard controls.
 * Allow them to share the available row space and wrap cleanly
 * on narrower screens.
 */
.btn-stage{
flex:1 1 220px;
text-align:center;
}

.soil-badge{
display:inline-block;
padding:6px 12px;
border-radius:999px;
font-size:14px;
font-weight:bold;
}

.soil-very-dry{
background:#fee4e2;
color:#b42318;
}

.soil-dry{
background:#fef0c7;
color:#b54708;
}

.soil-good{
background:#d1fae5;
color:#067647;
}

.soil-wet{
background:#dbeafe;
color:#175cd3;
}

.soil-very-wet{
background:#e4e7ec;
color:#344054;
}

</style>
</head>

<body>
<div class='container'>

<header>
<h1>Smart Greenhouse</h1>
<div class='subtitle'>Local Monitoring Dashboard</div>
</header>
)HTML";


/*
 * Close the page and load the dashboard JavaScript as a separate resource.
 * The script itself is embedded in firmware and served by web_server.cpp.
 */
static const char DASHBOARD_PAGE_END[] PROGMEM = R"HTML(
</div>
<script src='/dashboard.js'></script>
</body>
</html>
)HTML";
