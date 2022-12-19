const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" rel="stylesheet">
    <style>
      html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
      }
      h1 { font-size: 2.0rem; }
      p { font-size: 1.5rem; }
    </style>
  </head>
  <body>
    <h1>Lolin NodeMCU V3 ESP8266/ESP12E RFID RC522</h1>
    <p style="color:#2e86c1;">THE UID OF THE SCANNED CARD IS : <span style="color:#17a589;" id="UIDResult">Waiting...</span></p>
  <script>
    setInterval(function() {
      // Call a function repetatively with 0.1 (100ms) Second interval
      getTemperatureData();
    }, 100); 

    function getTemperatureData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("UIDResult").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "UIDResult", true);
      xhttp.send();
    }
  </script>
  </body>
</html>
)=====";
