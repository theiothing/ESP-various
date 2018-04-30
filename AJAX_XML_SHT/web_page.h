static const char WebsiteTemplate[] PROGMEM = // #xxxxxxxx# are later to be changed in c function WebsiteContent()
R"=====(
  <!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <style>

  body{
    background-color: #373c44;
    color: #ccc;
    font-family: Nexa, sans-serif;
  }
  span{
    font-size: 80%;
  }
  p{
    font-size: 150%;
    padding: 0;
    margin-bottom: 5px;
  }
  .col-container {
    position: absolute;
    top:0;
    bottom: 0;
    left: 0;
    right: 0;
    margin: auto;
    display: table;
    width: 100%;
  }
  .col {
    margin:3px;
    text-align: center;
    display: table-cell;
    padding: 20px;
    background-color: #282a2d;
  }
</style>
</head>
<body onload='xml_process()'>
  <div class="col-container" >
    <div class="col">
      <p id="div_001">#value_001#</p>
      <span class="measure">rel. humidity</span>
    </div>
    <div class="col">
      <p id="div_002">#value_002#</p>
      <span class="measure">temperature</span>
    </div>
  </div>  

  <SCRIPT> // here starts the Java script for AJAX method

    var xmlHttp = new XMLHttpRequest(); // for IE7+, Firefox, Chrome, Opera, Safari (older IE not supported)

    function response()
      {
        if(xmlHttp.readyState==4 && xmlHttp.status==200)
        {
          xmlResponse = xmlHttp.responseXML;
          xmldoc = xmlResponse.getElementsByTagName('xml_tag_001');
          message = xmldoc[0].firstChild.nodeValue;
          document.getElementById('div_001').innerHTML=message;
          xmldoc = xmlResponse.getElementsByTagName('xml_tag_002');
          message = xmldoc[0].firstChild.nodeValue;
          document.getElementById('div_002').innerHTML=message;         
        }
      }

    function xml_process()
    {
      if(xmlHttp.readyState==0 || xmlHttp.readyState==4)
      {
        xmlHttp.open('PUT', 'xml', true); // execute c function XMLcontent()
        xmlHttp.onreadystatechange = response;
        xmlHttp.send(null); //Request with no data in request body
      }
      setTimeout('xml_process()', 1000);
    }

  </SCRIPT>
  </body>
</html>
)=====";


const char XML_Template[] PROGMEM = // #xxxxxxxx# are later to be changed in c function XML_Content()
R"=====(<?xml version='1.0'?>
  <Variables>
    <xml_tag_001>#value_001#</xml_tag_001>
    <xml_tag_002>#value_002#</xml_tag_002>
  </Variables>
)=====";


void WebsiteContent()
{
  server.send(200,"text/html", WebsiteTemplate);
}


