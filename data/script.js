var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);

function onload(event) {
  initWebSocket();
}

function getValues() {
  websocket.send("getValues");
}

function initWebSocket() {
  console.log('Trying to open a WebSocket connection…');
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
}

function onOpen(event) {
  console.log('Connection opened');
  getValues();
}

function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}

function updateSliderPWM(element) {
  var sliderNumber = element.id.charAt(element.id.length - 1);
  var sliderValue = document.getElementById(element.id).value;
  document.getElementById("out2").innerHTML = sliderValue;
  console.log(parseInt(sliderValue));
  websocket.send(sliderNumber + "s" + sliderValue.toString());
}

function onMessage(event) {
  console.log(event.data);
  var myObj = JSON.parse(event.data);
  var keys = Object.keys(myObj);

  for (var i = 0; i < 4; i++) {
    var key = keys[i];
    //console.log(keys[i]);
    //console.log(myObj[key]);
    //document.getElementById(key).innerHTML = myObj[key];
    document.getElementById("slider" + (i + 1).toString()).value = myObj[key];
  }

  if (myObj["power"] == "1") { //powerOn value
    console.log("power: on");
    document.getElementById("toggle_power").checked = true;
  } else {
    document.getElementById("toggle_power").checked = false;
    console.log("power: off");
  }
  if (myObj["reverse"] == "1") { //powerOn value
    console.log("drive: reverse");
    document.getElementById("toggle_reverse").checked = true;
  } else {
    document.getElementById("toggle_reverse").checked = false;
    console.log("drive: forwards");
  }
}

function powerToggle() {
  var checkBox = document.getElementById("toggle_power");
  websocket.send("switch_power" + checkBox.checked.toString());
}
function reverseToggle() {
  var checkBox = document.getElementById("toggle_reverse");
  websocket.send("switch_reverse" + checkBox.checked.toString());
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  source.addEventListener('open', function (e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function (e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('message', function (e) {
    console.log("message", e.data);
  }, false);

  source.addEventListener('input1', function (e) {
    console.log("input1", e.data);
    document.getElementById("in1").innerHTML = parseInt(e.data);
    document.getElementById("act1_1").style.height = parseInt(e.data) + 'px';
    document.getElementById("act1_1").style.width = parseInt(e.data) + 'px';
    document.getElementById("act1_1").style.top = (65 - parseInt(e.data) / 2) + 'px';
    document.getElementById("act1_1").style.left = (55 - parseInt(e.data) / 2) + 'px';
    console.log(parseInt(e.data));
  }, false);
  source.addEventListener('input2', function (e) {
    console.log("input2", e.data);
    document.getElementById("in2").innerHTML = parseInt(e.data);
    document.getElementById("act1_2").style.height = parseInt(e.data) + 'px';
    document.getElementById("act1_2").style.width = parseInt(e.data) + 'px';
    document.getElementById("act1_2").style.top = (290 - parseInt(e.data) / 2) + 'px';
    document.getElementById("act1_2").style.left = (55 - parseInt(e.data) / 2) + 'px';
    console.log(parseInt(e.data));
  }, false);

  source.addEventListener('output1', function (e) {
    console.log("output1", e.data);
    document.getElementById("out1").innerHTML = parseInt(e.data);
    document.getElementById("act2_1").style.height = parseInt(e.data) + 'px';
    document.getElementById("act2_1").style.width = parseInt(e.data) + 'px';
    document.getElementById("act2_1").style.top = (65 - parseInt(e.data) / 2) + 'px';
    document.getElementById("act2_1").style.right = (55 - parseInt(e.data) / 2) + 'px';
    console.log(parseInt(e.data));

  }, false);

  source.addEventListener('output2', function (e) {
    console.log("output2", e.data);
    document.getElementById("out2").innerHTML = parseInt(e.data);
    document.getElementById("act2_2").style.height = parseInt(e.data) + 'px';
    document.getElementById("act2_2").style.width = parseInt(e.data) + 'px';
    document.getElementById("act2_2").style.top = (290 - parseInt(e.data) / 2) + 'px';
    document.getElementById("act2_2").style.right = (55 - parseInt(e.data) / 2) + 'px';
    console.log(parseInt(e.data));
  }, false);
  source.addEventListener('power', function (e) {
    console.log("input1", e.data);
  }, false);
  source.addEventListener('reverse', function (e) {
    console.log("input1", e.data);
    console.log(parseInt(e.data));
  }, false);
}