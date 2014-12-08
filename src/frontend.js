var timezonedb_key = 'AFC1QTW65WWZ';

var xhrRequest = function (url, type, callback, errorCallback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.onerror = function(e) {
        callback(e.target.status);
    };
    xhr.open(type, url);
    xhr.send();
};

var sendCurrentTimeZoneToPebble = function() {
    navigator.geolocation.getCurrentPosition(
        function(pos) {
            var lat = pos.coords.latitude;
            var lon = pos.coords.longitude;
            var url = 'http://api.timezonedb.com/?lat=' + lat + '&lng=' + lon + '&key=' + timezonedb_key;
            
            console.log('Making a request to TimeZoneDB: ' + url);
            
            xhrRequest(
                url,
                'GET',
                function(text) {
                    var getElem = function(name) {
                        var re = new RegExp('<' + name + '>(.*)</' + name + '>');
                        return re.exec(text)[1];  
                    };
                    var status = getElem('status');
                    if (status != 'OK') {
                        var errorText = getElem('message');
                        console.log('TimeZoneDB returned an error: ' + errorText);
                        return;
                    }
                    
                    var dictionary = {
                        'TZ_NAME': getElem('zoneName'),
                        'TZ_UTC_OFFSET': parseInt(getElem('gmtOffset')),
                    };
                    
                    console.log('Sending ' + JSON.stringify(dictionary) + ' to Pebble');
                        
                    Pebble.sendAppMessage(
                        dictionary,
                        function() {},
                        function(e) {
                            console.log('Failed to send data to Pebble');
                        });
                },
                function(errorStatus) {
                    console.log('Failed to query TimeZoneDB: ' + errorStatus);
                });
        },
        function() {
            console.log('Failed to determine the location');
        }
    );
};

Pebble.addEventListener('ready', function(e) {
    sendCurrentTimeZoneToPebble();
});