/*
 * Project: Software Defined Blocks
 * Copyright (C) 2023 alf.labs gmail com.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INC_SDB_MOD_WIFI_AP_INDEX_H
#define __INC_SDB_MOD_WIFI_AP_INDEX_H

const char _ap_index_html[] PROGMEM = R"rawliteral(<html>
<body>
<h1>Software Defined Blocks</h1>
<h2>Current Wifi Configuration</h2>
<p>Wifi Network: <span id="wn">$WN$</span></p>
<p>Wifi Password: <span id="wp">$WP$</span></p>

<h2>New Wifi Configuration</h2>
<p>List of available Wifi networks: $WL$</p>
<script>
function init() {
    const wn = document.querySelector("#wn");
    wn.textContent = "window load";

    fetch("/get").then((response) => {
        if (response.ok) {
            const wp = document.querySelector("#wp");
            wp.textContent = response.text;
        }
    });
}
window.addEventListener("load", (event) => init());
</script>
</body></html>)rawliteral";

#endif // __INC_SDB_MOD_WIFI_AP_INDEX_H
