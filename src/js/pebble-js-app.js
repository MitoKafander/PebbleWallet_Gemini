// PebbleWallet Gemini - Binary Sync with Invert Support
var CONFIG_URL = "https://mitokafander.github.io/PebbleWallet_Gemini/config/index.html";

Pebble.addEventListener('showConfiguration', function() {
    var data = {
        cards: JSON.parse(localStorage.getItem('cards') || '[]'),
        invert: JSON.parse(localStorage.getItem('invert') || 'false')
    };
    Pebble.openURL(CONFIG_URL + '#' + encodeURIComponent(JSON.stringify(data)));
});

Pebble.addEventListener('webviewclosed', function(e) {
    if (!e.response || e.response === 'CANCELLED') return;
    var data = JSON.parse(decodeURIComponent(e.response));
    
    localStorage.setItem('cards', JSON.stringify(data.cards));
    localStorage.setItem('invert', JSON.stringify(data.invert));
    
    syncToWatch(data.cards, data.invert);
});

function syncToWatch(cards, invert) {
    // Send SYNC_START with the Invert setting
    Pebble.sendAppMessage({ 
        'CMD_SYNC_START': 1,
        'KEY_INVERT': invert ? 1 : 0
    }, function() {
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

    if (c.data.indexOf(',') > -1) {
        var parts = c.data.split(',');
        dict['KEY_WIDTH'] = parseInt(parts[0]);
        dict['KEY_HEIGHT'] = parseInt(parts[1]);
        var hex = parts[2];
        var bytes = [];
        for (var i = 0; i < hex.length; i += 2) bytes.push(parseInt(hex.substr(i, 2), 16));
        dict['KEY_DATA'] = bytes;
    } else {
        dict['KEY_WIDTH'] = 0; dict['KEY_HEIGHT'] = 0;
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