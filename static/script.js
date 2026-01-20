// Configuration
// IMPORTANT: Replace this IP with your Raspberry Pi's IP address
var ipAddress = "172.20.10.2"; 
var port = "8000";

// Create WebSocket connection
var ws = new WebSocket("ws://" + ipAddress + ":" + port);

var statusText = document.getElementById("status");

// Connection successful
ws.onopen = function() {
    statusText.innerHTML = "System connected";
    statusText.style.color = "#4caf50";
};

// Error handling
ws.onerror = function() {
    statusText.innerHTML = "Connection error";
    statusText.style.color = "#f44336";
};

// Connection closed
ws.onclose = function() {
    statusText.innerHTML = "Connection closed";
    statusText.style.color = "#888";
};

// Send command to server
function send(command) {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(command);
        console.log("Sent: " + command);
    } else {
        alert("No connection to server!");
    }
}

// Send dimmer command
function sendDimmer(lamp, value) {
    var command = "<Dim" + lamp + ":" + value + ">";
    send(command);
}

// Send target temperature command
function sendTargetTemp(value) {
    var command = "<SetTemp:" + value + ">";
    send(command);
}

// Receive message from server
ws.onmessage = function(event) {
    var message = event.data;
    console.log("Received: " + message);

    // Parse response: "Temp:22.5;AlarmArmed:1;AlarmTriggered:0"
    var parts = message.split(";");
    
    for (var i = 0; i < parts.length; i++) {
        if (parts[i].includes("Temp:")) {
            var temperature = parts[i].split(":")[1]; 
            document.getElementById("tempDisplay").innerHTML = temperature + " °C";
        }
        else if (parts[i].includes("AlarmArmed:")) {
            var armed = parts[i].split(":")[1];
            var statusElement = document.getElementById("alarmArmedStatus");
            if (armed === "1") {
                statusElement.innerHTML = "ARMED";
                statusElement.style.backgroundColor = "#ff9800";
                statusElement.style.color = "#fff";
            } else {
                statusElement.innerHTML = "UNARMED";
                statusElement.style.backgroundColor = "#4caf50";
                statusElement.style.color = "#fff";
            }
        }
        else if (parts[i].includes("AlarmTriggered:")) {
            var triggered = parts[i].split(":")[1];
            var triggerElement = document.getElementById("alarmTrigger");
            if (triggered === "1") {
                triggerElement.innerHTML = "ALARM!";
                triggerElement.style.backgroundColor = "#f44336";
                triggerElement.style.color = "#fff";
            } else {
                triggerElement.innerHTML = "OK";
                triggerElement.style.backgroundColor = "#4caf50";
                triggerElement.style.color = "#fff";
            }
        }
    }
};

// Poll server every 2 seconds for status updates
setInterval(function() {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send("<GetStatus>"); 
    }
}, 2000);