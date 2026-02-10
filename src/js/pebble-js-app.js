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
        var rawW = parseInt(parts[0]);
        var rawH = parseInt(parts[1]);
        var hex = parts[2];
        
        // OPTIMIZATION: Crop whitespace to allow larger scaling on watch
        var optimized = cropBitmap(rawW, rawH, hex);
        
        dict['KEY_WIDTH'] = optimized.width;
        dict['KEY_HEIGHT'] = optimized.height;
        var bytes = [];
        for (var i = 0; i < optimized.hex.length; i += 2) bytes.push(parseInt(optimized.hex.substr(i, 2), 16));
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

// Helper to remove white borders from bitmap data
function cropBitmap(width, height, hex) {
    var bytes = [];
    for (var i = 0; i < hex.length; i += 2) bytes.push(parseInt(hex.substr(i, 2), 16));
    
    var stride = Math.ceil(width / 8);
    if (bytes.length < stride * height) return { width: width, height: height, hex: hex };

    var minX = width, maxX = 0, minY = height, maxY = 0;
    var found = false;

    // Scan for black pixels (1)
    for (var y = 0; y < height; y++) {
        for (var x = 0; x < width; x++) {
            var byteIndex = y * stride + (x >> 3);
            var bitIndex = 7 - (x % 8);
            if ((bytes[byteIndex] >> bitIndex) & 1) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
                found = true;
            }
        }
    }

    if (!found) return { width: width, height: height, hex: hex };

    var newW = maxX - minX + 1;
    var newH = maxY - minY + 1;
    var newStride = Math.ceil(newW / 8);
    var newBytes = new Array(newStride * newH);
    for(var k=0; k<newBytes.length; k++) newBytes[k] = 0;
    
    for (var y = 0; y < newH; y++) {
        for (var x = 0; x < newW; x++) {
            var srcX = x + minX;
            var srcY = y + minY;
            var srcByte = srcY * stride + (srcX >> 3);
            var srcBit = 7 - (srcX % 8);
            if ((bytes[srcByte] >> srcBit) & 1) {
                newBytes[y * newStride + (x >> 3)] |= (1 << (7 - (x % 8)));
            }
        }
    }
    
    var newHex = "";
    for (var i = 0; i < newBytes.length; i++) {
        var h = newBytes[i].toString(16);
        if (h.length < 2) h = "0" + h;
        newHex += h;
    }
    
    return { width: newW, height: newH, hex: newHex };
}