#pragma once

#include <Arduino.h>

/*
 * Smart Greenhouse Controller
 * Dashboard JavaScript
 *
 * The browser loads this script from the ESP8266 as /dashboard.js.
 * It polls the local JSON status API and updates only the affected DOM
 * elements, so live values can change without reloading the full page.
 *
 * The script also keeps the visual state of AUTO / MANUAL and ON / OFF
 * controls synchronized with the actual controller state reported by
 * /api/status.
 */

static const char DASHBOARD_SCRIPT[] PROGMEM = R"JS(
const DASHBOARD_UPDATE_INTERVAL_MS = 2000;


/*
 * Update a text-only DOM element when it exists.
 */
function setText(id, value)
{
    const element = document.getElementById(id);

    if (element)
    {
        element.textContent = value;
    }
}


/*
 * Update the soil-moisture label and its visual severity class.
 */
function updateSoilBadge(status)
{
    const badge =
        document.getElementById('soilStatusBadge');

    if (!badge)
    {
        return;
    }

    badge.textContent = status;
    badge.className = 'soil-badge';

    switch (status)
    {
        case 'VERY DRY':
            badge.classList.add('soil-very-dry');
            break;

        case 'DRY':
            badge.classList.add('soil-dry');
            break;

        case 'GOOD':
            badge.classList.add('soil-good');
            break;

        case 'WET':
            badge.classList.add('soil-wet');
            break;

        case 'VERY WET':
            badge.classList.add('soil-very-wet');
            break;
    }
}


/*
 * Keep the controller Humidifier card synchronized with the actual relay
 * state reported by the controller, not merely with the latest user request.
 */
function updateHumidifierBadge(isOn)
{
    const badge =
        document.getElementById('humidifierBadge');

    const value =
        document.getElementById('humidifierValue');

    if (!badge || !value)
    {
        return;
    }

    value.textContent = isOn ? 'ON' : 'OFF';
    badge.className = isOn
        ? 'state-badge state-on'
        : 'state-badge state-off';
}


/*
 * Highlight the selected operating mode.
 */
function updateModeButtons(mode)
{
    const autoButton =
        document.getElementById('autoModeButton');

    const manualButton =
        document.getElementById('manualModeButton');

    if (!autoButton || !manualButton)
    {
        return;
    }

    if (mode === 'AUTO')
    {
        autoButton.className = 'btn btn-primary';
        manualButton.className = 'btn btn-secondary';
    }
    else
    {
        autoButton.className = 'btn btn-secondary';
        manualButton.className = 'btn btn-primary';
    }
}


/*
 * Manual actuator controls are available only in MANUAL mode.
 * Their highlight follows the actual relay output state.
 */
function updateHumidifierButtons(mode, isOn)
{
    const onButton =
        document.getElementById('humidifierOnButton');

    const offButton =
        document.getElementById('humidifierOffButton');

    if (!onButton || !offButton)
    {
        return;
    }

    const manualModeActive = mode === 'MANUAL';

    onButton.disabled = !manualModeActive;
    offButton.disabled = !manualModeActive;

    if (!manualModeActive)
    {
        onButton.className = 'btn btn-inactive';
        offButton.className = 'btn btn-inactive';
        return;
    }

    onButton.className = isOn
        ? 'btn btn-active-on'
        : 'btn btn-inactive';

    offButton.className = isOn
        ? 'btn btn-inactive'
        : 'btn btn-active-off';
}


/*
 * Fetch one controller snapshot and refresh all live dashboard fields.
 */
async function updateDashboard()
{
    try
    {
        const response = await fetch('/api/status', {
            cache: 'no-store'
        });

        if (!response.ok)
        {
            return;
        }

        const data = await response.json();

        if (!data.available)
        {
            return;
        }

        /*
         * A dashboard opened before the first measurement contains only the
         * waiting card. Reload once after data becomes available so the full
         * dashboard DOM can be generated.
         */
        if (!document.getElementById('temperatureValue'))
        {
            window.location.reload();
            return;
        }

        /* Environment */
        setText(
            'temperatureValue',
            data.temperature === null
                ? 'INVALID'
                : data.temperature.toFixed(2) + ' °C'
        );

        setText(
            'humidityValue',
            data.humidity === null
                ? 'INVALID'
                : data.humidity.toFixed(2) + ' %'
        );

        setText(
            'pressureValue',
            data.pressure === null
                ? 'INVALID'
                : data.pressure.toFixed(2) + ' hPa'
        );

        /* Soil moisture */
        setText(
            'soilValue',
            data.soilPercent.toFixed(1) + ' %'
        );

        updateSoilBadge(data.soilStatus);

        /* VPD */
        setText(
            'airVpdValue',
            data.airVPD === null
                ? 'INVALID'
                : data.airVPD.toFixed(2) + ' kPa'
        );

        setText(
            'targetVpdValue',
            data.targetVPD.toFixed(2) + ' kPa'
        );

        setText(
            'vpdRangeValue',
            data.minVPD.toFixed(2) +
            ' - ' +
            data.maxVPD.toFixed(2) +
            ' kPa'
        );

        setText(
            'vpdStatusValue',
            data.vpdStatus
        );

        /* Controller */
        setText('modeValue', data.mode);
        updateModeButtons(data.mode);

        setText(
            'growthStageValue',
            data.growthStage
        );

        updateHumidifierBadge(data.humidifier);
        updateHumidifierButtons(
            data.mode,
            data.humidifier
        );

        setText(
            'controlValue',
            data.controlValid
                ? 'ACTIVE'
                : 'FAIL-SAFE'
        );

        setText(
            'timingLockValue',
            data.timingLock
                ? 'ACTIVE'
                : 'OFF'
        );

        setText(
            'rhSafetyValue',
            data.rhSafety
                ? 'ACTIVE'
                : 'OFF'
        );

        /* Network */
        setText(
            'wifiValue',
            data.wifiConnected
                ? 'CONNECTED'
                : 'OFFLINE'
        );

        setText('ipValue', data.ip);

        setText(
            'rssiValue',
            data.rssi === null
                ? 'N/A'
                : data.rssi + ' dBm'
        );

        /* Latest measurement age */
        setText(
            'lastUpdateValue',
            data.lastUpdateSeconds
        );
    }
    catch (error)
    {
        console.log(
            'Dashboard update failed:',
            error
        );
    }
}


/*
 * Refresh immediately after page load, then continue polling at a moderate
 * interval that is suitable for the current ESP8266 prototype.
 */
updateDashboard();

setInterval(
    updateDashboard,
    DASHBOARD_UPDATE_INTERVAL_MS
);
)JS";
