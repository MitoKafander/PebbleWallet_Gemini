// PebbleWallet Gemini - Skunk-Style Binary Sync
var CONFIG_URL = "https://mitokafander.github.io/PebbleWallet_Gemini/config/index.html";

Pebble.addEventListener('showConfiguration', function() {
    var cards = JSON.parse(localStorage.getItem('cards') || '[]');
    Pebble.openURL(CONFIG_URL + '#' + encodeURIComponent(JSON.stringify(cards)));
});

Pebble.addEventListener('webviewclosed', function(e) {
    if (!e.response || e.response === 'CANCELLED') return;
    var cards = JSON.parse(decodeURIComponent(e.response));
    localStorage.setItem('cards', JSON.stringify(cards));
    syncToWatch(cards);
});

function syncToWatch(cards) {
    Pebble.sendAppMessage({ 'CMD_SYNC_START': 1 }, function() {
        sendNextCard(cards, 0);
    });
}

function sendNextCard(cards, index) {
    if (index >= cards.length) {
        Pebble.sendAppMessage({ 'CMD_SYNC_COMPLETE': 1 });
        return;
    }
    
    var c = cards[index];
    var dict = {
        'KEY_INDEX': index,
        'KEY_NAME': c.name,
        'KEY_DESCRIPTION': c.description || "",
        'KEY_FORMAT': parseInt(c.format)
    };

    // If it's a matrix (QR/Aztec/PDF417), convert "w,h,hex" to binary
    if (c.data.indexOf(',') > -1) {
        var parts = c.data.split(',');
        dict['KEY_WIDTH'] = parseInt(parts[0]);
        dict['KEY_HEIGHT'] = parseInt(parts[1]);
        
        // Convert Hex String to Byte Array
        var hex = parts[2];
        var bytes = [];
        for (var i = 0; i < hex.length; i += 2) {
            bytes.push(parseInt(hex.substr(i, 2), 16));
        }
        dict['KEY_DATA'] = bytes; // Sending raw bytes!
    } else {
        // 1D Barcode (just text)
        dict['KEY_WIDTH'] = 0;
        dict['KEY_HEIGHT'] = 0;
        // Convert text to bytes so KEY_DATA is always a byte array for consistency
        var bytes = [];
        for (var i = 0; i < c.data.length; i++) bytes.push(c.data.charCodeAt(i));
        dict['KEY_DATA'] = bytes;
    }

    Pebble.sendAppMessage(dict, function() {
        sendNextCard(cards, index + 1);
    }, function(e) {
        setTimeout(function() { sendNextCard(cards, index); }, 1000);
    });
}
