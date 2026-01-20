# Webhouse Setup - Apache Webserver Integration

## Übersicht

Dieses Projekt besteht aus zwei Komponenten:
1. **C-Server** (`webhouse`) - WebSocket-Server auf Port 8000 für Hardware-Steuerung
2. **Web-Interface** (HTML/JS/CSS) - Benutzeroberfläche über Apache Webserver

## Installation auf Raspberry Pi

### 1. Apache Webserver installieren

```bash
sudo apt-get update
sudo apt-get install apache2
```

### 2. Webhouse kompilieren

```bash
cd ~/BFH_EmbeddedSystemsWebHouse/webhouse
make clean
make webhouse
```

### 3. Web-Dateien nach Apache kopieren

```bash
# Static-Dateien ins Apache-Verzeichnis kopieren
sudo cp ~/BFH_EmbeddedSystemsWebHouse/static/* /var/www/html/

# Oder symbolischen Link erstellen
sudo ln -s ~/BFH_EmbeddedSystemsWebHouse/static /var/www/html/webhouse
```

### 4. IP-Adresse in script.js anpassen

Öffnen Sie `/var/www/html/script.js` und passen Sie die IP an:

```javascript
// Wenn Browser und Pi auf demselben Gerät:
var ipAdresse = "localhost";

// Wenn Browser auf anderem Gerät im Netzwerk:
var ipAdresse = "192.168.1.XXX";  // IP des Raspberry Pi
```

### 5. Webhouse-Server starten

```bash
cd ~/BFH_EmbeddedSystemsWebHouse/webhouse
sudo ./webhouse
```

**Wichtig:** `sudo` ist nötig für GPIO-Zugriff!

### 6. Webinterface öffnen

Im Browser auf einem beliebigen Gerät im Netzwerk:

```
http://<IP-des-Raspberry-Pi>/index.html
```

Oder wenn direkt auf dem Pi:

```
http://localhost/index.html
```

## Funktionen

### Steuerung
- **Heizung** - Ein/Aus + Temperaturregelung via Slider
- **LED 1** (Wohnzimmer) - Ein/Aus + Dimmer
- **LED 2** (Küche) - Ein/Aus + Dimmer  
- **Fernseher** - Ein/Aus
- **Alarmanlage** - Aktivieren/Deaktivieren (Status wird angezeigt)

### Anzeige
- **Temperatur** - Wird alle 2 Sekunden aktualisiert

## Befehls-Protokoll

### Client → Server (HTML Buttons)
- `<HeatOn>` / `<HeatOff>` - Heizung
- `<L1on>` / `<L1off>` - LED 1
- `<L2on>` / `<L2off>` - LED 2
- `<TVon>` / `<TVoff>` - Fernseher
- `<AlarmOn>` / `<AlarmOff>` - Alarm
- `<Dim1:0-100>` - Dimmer LED 1
- `<Dim2:0-100>` - Dimmer LED 2
- `<SetTemp:15-30>` - Soll-Temperatur
- `<GetStatus>` - Status abfragen

### Server → Client (WebSocket Antwort)
- `CmdOK;Temp:22.5` - Bestätigung + aktuelle Temperatur

## Autostart (Optional)

### Webhouse beim Booten starten

```bash
sudo nano /etc/rc.local
```

Vor `exit 0` einfügen:

```bash
/home/pi/BFH_EmbeddedSystemsWebHouse/webhouse/webhouse &
```

## Troubleshooting

### WebSocket-Verbindung schlägt fehl
- Prüfen ob webhouse-Server läuft: `ps aux | grep webhouse`
- Firewall-Regeln prüfen
- IP-Adresse in script.js korrekt?

### Temperatur wird nicht angezeigt
- Server-Terminal prüfen auf Fehler
- Browser-Console öffnen (F12) und nach Fehlern suchen

### Hardware reagiert nicht
- `sudo` beim Starten verwendet?
- `test_hardware` ausführen zum Testen: `sudo ./test_hardware`

## Hardware-Test

Vor dem ersten Einsatz Hardware testen:

```bash
cd ~/BFH_EmbeddedSystemsWebHouse/webhouse
make test_hardware
sudo ./test_hardware
```
