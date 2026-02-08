// PebbleWallet Gemini - Optimized Phone Logic
// Offline-first architecture

// Note: Ensure this URL points to the location of your NEW config/index.html
// If testing locally/Codespaces, update this URL.
var CONFIG_URL = "https://mitokafander.github.io/PebbleWallet_Gemini/config/index.html";

Pebble.addEventListener('ready', function() {
    console.log('Gemini Wallet JS Ready');
});

Pebble.addEventListener('showConfiguration', function() {
    var cards = [];
    try {
        cards = JSON.parse(localStorage.getItem('cards') || '[]');
    } catch(e) {}
    
    // Pass cards in URL hash
    var url = CONFIG_URL + '#' + encodeURIComponent(JSON.stringify(cards));
    console.log('Opening config: ' + url);
    Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
    if (!e.response || e.response === 'CANCELLED') return;
    
    var cards;
    try {
        cards = JSON.parse(decodeURIComponent(e.response));
        localStorage.setItem('cards', JSON.stringify(cards));
        syncToWatch(cards);
    } catch(err) {
        console.log('Config Parse Error: ' + err);
    }
});

function syncToWatch(cards) {
    // Offline-first sync strategy:
    // 1. Send SYNC_START (clears watch DB in memory, prepares for overwrite)
    // 2. Send each card
    // 3. Send SYNC_COMPLETE
    
    var dictStart = { 'CMD_SYNC_START': 1 };
    
    Pebble.sendAppMessage(dictStart, function() {
        sendNextCard(cards, 0);
    }, function(e) {
        console.log('Sync Start Failed: ' + JSON.stringify(e));
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
        // For Aztec/PDF417, c.data is now a long string "w,h,hex..."
        'KEY_DATA': c.data, 
        'KEY_FORMAT': parseInt(c.format)
    };
    
    console.log('Sending Card ' + index + ' (' + c.name + ') len=' + c.data.length);

    Pebble.sendAppMessage(dict, function() {
        sendNextCard(cards, index + 1);
    }, function(e) {
        console.log('Card ' + index + ' failed, retrying... Error: ' + JSON.stringify(e));
        setTimeout(function() { sendNextCard(cards, index); }, 1000);
    });
}