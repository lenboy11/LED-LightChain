var ip = "http://192.168.178.94/";
var xhr = new XMLHttpRequest();
xhr.timeout = 5000;
var v_red = 0;
var v_green = 0;
var v_blue = 0;

function updateCube() {
  changeCubeColor(v_red, v_green, v_blue);
}

function getColor() {
  console.log("1");
  xhr.open("GET", ip + "getColor", true);
  console.log("2");
  xhr.send();
  console.log("3");
}

xhr.onload = function () {
  getResponse(xhr.response);
  console.log("6");
  setTimeout(getColor, 500);
};

xhr.ontimeout = function () {
  setTimeout(getColor, 10000);
};

function getResponse(resp) {
  console.log("response:");
  console.log(resp);

  if (resp != "") {
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
    }
  }
  updateCube();
  console.log("5");
}

function changeCubeColor(r, g, b) {
  var temp = document.querySelectorAll(".cube-site");
  var colRGB = temp[0].style.backgroundColor;
  console.log("colHex:" + colRGB);

  var red = colRGB.indexOf("(");
  var green = colRGB.indexOf(",");
  var blue = colRGB.lastIndexOf(",");
  var len = colRGB.indexOf(")");

  var colRed = parseInt(colRGB.substring(red + 1, green));
  var colGreen = parseInt(colRGB.substring(green + 1, blue));
  var colBlue = parseInt(colRGB.substring(blue + 1, len));
  console.log("colRed:" + colRed);
  console.log("colGreen:" + colGreen);
  console.log("colBlue:" + colBlue);
  if (
    Number.isFinite(colRed) &&
    Number.isFinite(colGreen) &&
    Number.isFinite(colBlue)
  ) {
    if (colRed != r || colGreen != g || colBlue != b) {
      cubeFade(
        temp,
        colRed,
        colGreen,
        colBlue,
        r,
        g,
        b,
        1,
        Math.max(
          Math.floor(
            (Math.abs(colRed - r) +
              Math.abs(colGreen - g) +
              Math.abs(colBlue - b)) /
              15
          ),
          15
        )
      );
    }
  } else {
    for (var i = 0; i < temp.length; i++) {
      temp[i].style.backgroundColor = rgbToHex(r, g, b);
    }
  }
}

function cubeFade(elems, r_Old, g_Old, b_Old, r_New, g_New, b_New, iter, max) {
  for (var i = 0; i < elems.length; i++) {
    elems[i].style.backgroundColor = rgbToHex(
      Math.ceil((r_Old * (max - iter)) / max + (r_New * iter) / max),
      Math.ceil((g_Old * (max - iter)) / max + (g_New * iter) / max),
      Math.ceil((b_Old * (max - iter)) / max + (b_New * iter) / max)
    );
  }
  if (iter < max) {
    setTimeout(
      cubeFade,
      33,
      elems,
      r_Old,
      g_Old,
      b_Old,
      r_New,
      g_New,
      b_New,
      iter + 1,
      max
    );
  }
}

// Conversion of RGB Color Code to Hex Color Code
function componentToHex(c) {
  let hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}
function rgbToHex(r, g, b) {
  return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}
