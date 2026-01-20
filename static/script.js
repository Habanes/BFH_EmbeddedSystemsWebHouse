// --- KONFIGURATION ---
// WICHTIG: Ersetzen Sie diese IP mit der Ihres Raspberry Pi
// Wenn Sie das HTML lokal öffnen: IP-Adresse des Raspberry Pi
// Wenn über Apache: "localhost" oder die IP
var ipAdresse = "localhost"; 
var port = "8000";

// WebSocket Verbindung erstellen
var ws = new WebSocket("ws://" + ipAdresse + ":" + port);

var statusText = document.getElementById("status");

// 1. Verbindung erfolgreich
ws.onopen = function() {
    statusText.innerHTML = "System verbunden";
    statusText.style.color = "#4caf50"; // Helles Grün für Text
};

// 2. Fehlerbehandlung
ws.onerror = function() {
    statusText.innerHTML = "Verbindungsfehler";
    statusText.style.color = "#f44336"; // Helles Rot für Text
};

// 3. Verbindung geschlossen
ws.onclose = function() {
    statusText.innerHTML = "Verbindung getrennt";
    statusText.style.color = "#888";
};

// 4. Funktion zum Senden (Client -> Server)
function send(befehl) {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(befehl);
        console.log("Gesendet: " + befehl);
    } else {
        alert("Keine Verbindung zum Server!");
    }
}

// Hilfsfunktion für Dimmer
function sendDimmer(lampe, wert) {
    var befehl = "<Dim" + lampe + ":" + wert + ">";
    send(befehl);
}

// Hilfsfunktion für Temp-Regler
function sendTempRegler(wert) {
    var befehl = "<SetTemp:" + wert + ">";
    send(befehl);
}

// 5. Nachricht empfangen (Server -> Client)
ws.onmessage = function(event) {
    var nachricht = event.data;
    console.log("Empfangen: " + nachricht);

    // Parse response: "Temp:22.5;AlarmArmed:1;AlarmTriggered:0"
    var parts = nachricht.split(";");
    
    for (var i = 0; i < parts.length; i++) {
        if (parts[i].includes("Temp:")) {
            var temperatur = parts[i].split(":")[1]; 
            document.getElementById("tempDisplay").innerHTML = temperatur + " °C";
        }
        else if (parts[i].includes("AlarmArmed:")) {
            var armed = parts[i].split(":")[1];
            var statusElement = document.getElementById("alarmArmedStatus");
            if (armed === "1") {
                statusElement.innerHTML = "SCHARF";
                statusElement.style.color = "#ff9800"; // Orange
                statusElement.style.fontWeight = "bold";
            } else {
                statusElement.innerHTML = "UNSCHARF";
                statusElement.style.color = "#888";
                statusElement.style.fontWeight = "normal";
            }
        }
        else if (parts[i].includes("AlarmTriggered:")) {
            var triggered = parts[i].split(":")[1];
            var triggerElement = document.getElementById("alarmTrigger");
            if (triggered === "1") {
                triggerElement.innerHTML = "ALARM!";
                triggerElement.style.backgroundColor = "#f44336"; // Red
                triggerElement.style.color = "#fff";
            } else {
                triggerElement.innerHTML = "OK";
                triggerElement.style.backgroundColor = "#4caf50"; // Green
                triggerElement.style.color = "#fff";
            }
        }
    }
};

setInterval(function() {
    if (ws.readyState === WebSocket.OPEN) {
        // Wir senden einen "leeren" Befehl, damit der Server antwortet
        // Der C-Server schickt bei JEDEM Befehl die Temperatur zurück.
        ws.send("<GetStatus>"); 
    }
}, 2000);