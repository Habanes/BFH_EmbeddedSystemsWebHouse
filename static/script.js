// --- KONFIGURATION ---
// WICHTIG: Ersetzen Sie diese IP mit der Ihres Raspberry Pi
var ipAdresse = "192.168.1.100"; 
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

    // Temperatur parsen (Erwartet Format "Temp:22.5")
    if (nachricht.includes("Temp:")) {
        var temperatur = nachricht.split(":")[1]; 
        document.getElementById("tempDisplay").innerHTML = temperatur + " °C";
    }
};

setInterval(function() {
    if (ws.readyState === WebSocket.OPEN) {
        // Wir senden einen "leeren" Befehl, damit der Server antwortet
        // Der C-Server schickt bei JEDEM Befehl die Temperatur zurück.
        ws.send("<GetStatus>"); 
    }
}, 2000);