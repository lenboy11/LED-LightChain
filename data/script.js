var ip = "http://192.168.178.94/";
var xhr = new XMLHttpRequest();
var v_red = 0;
var v_green = 0;
var v_blue = 0;

// To load the Picker and start the ongoing communication
function main() {
  let colorIndicator = document.getElementById("color-indicator");
  var colorPicker = new iro.ColorPicker("#color-picker", {
    width: 320,
    color: "#fff",
  });
  colorPicker.on("color:change", function (color) {
    colorIndicator.style.backgroundColor = color.hexString;
    changeColor(
      colorPicker.color.red,
      colorPicker.color.green,
      colorPicker.color.blue
    );
  });

  getColor();
}

function ledSwitch(elem) {
  var val = elem.value.toLowerCase();
  if (val == "on") {
    elem.value = "Off";
    elem.classList.remove("button-on");
    elem.classList.add("button-off");
  } else {
    elem.value = "On";
    elem.classList.remove("button-off");
    elem.classList.add("button-on");
  }
  xhr.open("GET", ip + val, true);
  xhr.send();
}

function updateSlider(element) {
  var sliderValue = element.value;
  console.log(sliderValue);
  xhr.open("GET", ip + "slider?" + element.id + "=" + sliderValue, true);
  xhr.send();
}

function blink() {
  stop = 0;
  for (i = 0; i < 1000; i++) {
    changeColor(255, 0, 0);
    sleep(500);
    changeColor(0, 255, 0);
    sleep(500);
    changeColor(0, 0, 255);
    sleep(500);
    if (stop == 1) {
      stop = 0;
      break;
    }
  }
}

function fadeColor(element) {
  xhr.open(
    "GET",
    ip +
      "fade?color=" +
      element.id +
      "&reps=" +
      document.getElementById("repSilder").value,
    true
  );
  xhr.send();
}

function fadeAll() {
  xhr.open(
    "GET",
    ip +
      "fade?color=" +
      element.id +
      "&reps=" +
      document.getElementById("repSilder").value,
    true
  );
  xhr.send();
}

function fadeRandom() {
  xhr.open(
    "GET",
    ip + "fade?color=Random&reps=" + document.getElementById("repSilder").value,
    true
  );
  xhr.send();
}

function changeColor(r, g, b) {
  var p = "";
  if (document.getElementById("fadeCheckBox").checked) {
    p = "&fade=" + document.getElementById("fadeSlider").value;
  }
  xhr.open(
    "GET",
    ip + "slider?red=" + r + "&green=" + g + "&blue=" + b + "" + p,
    true
  );
  xhr.send();
}

function changeColor(r, g, b, fade) {
  xhr.open(
    "GET",
    ip + "slider?red=" + r + "&green=" + g + "&blue=" + b + "&fade=" + fade,
    true
  );
  xhr.send();
}

function updatePicker(picker) {
  var r = picker.channel("R");
  var g = picker.channel("G");
  var b = picker.channel("B");
  changeColor(r, g, b);
}

var stop = 0;

function visibleSlider(checker) {
  if (checker.checked) {
    document.getElementById("fadeSlider").disabled = false;
  } else {
    document.getElementById("fadeSlider").disabled = true;
  }
}

function componentToHex(c) {
  let hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}
function rgbToHex(r, g, b) {
  return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

// Get Color Listener:
// Request Color
function getColor() {
  console.log("1");
  xhr.open("GET", ip + "getColor", true);
  console.log("2");
  xhr.send();
  console.log("3");
}
// Listen For Response
xhr.onload = function () {
  var code = getResponse(xhr.response);
  console.log("6");
  if (code == "rgb") {
    setTimeout(getColor, 500);
  }
};
// Parse Response
function getResponse(resp) {
  console.log("response:");
  console.log(resp);

  if (resp != "" && resp != "Ok") {
    var red = resp.indexOf("r=");
    var green = resp.indexOf("g=");
    var blue = resp.indexOf("b=");
    console.log("red:" + red);
    console.log("green:" + green);
    console.log("blue:" + blue);
    var len = resp.length;
    var sub = "";
    if (red != -1 && green != -1 && blue != -1) {
      sub = resp.substring(red + 2, green - 1);
      v_red = parseInt(sub);
      sub = resp.substring(green + 2, blue - 1);
      v_green = parseInt(sub);
      sub = resp.substring(blue + 2, len);
      v_blue = parseInt(sub);
      console.log(v_red);
      console.log(v_green);
      console.log(v_blue);
      synchronizePage();
      return "rgb";
    }
  }

  return "false";
}

// Synchronize Page Values to LED (add syncFunction here)
function synchronizePage() {
  syncButton();
  syncSlider();
  syncPicker();
}

function syncButton() {
  var but = document.getElementById("onOFF");
  if (v_red + v_green + v_blue == 0) {
    but.classList.remove("button-off");
    but.classList.add("button-on");
    but.value = "On";
  } else {
    but.classList.remove("button-on");
    but.classList.add("button-off");
    but.value = "Off";
  }
}

function syncSlider() {
  var elems = document.querySelectorAll(".slider");
  for (var i = 0; i < elems.length; i++) {
    if (elems.id == "red") {
      elems[i].value = v_red;
    } else if (elems.id == "green") {
      elems[i].value = v_green;
    } else if (elems.id == "blue") {
      elems[i].value = v_blue;
    }
  }
}
