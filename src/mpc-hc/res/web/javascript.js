/* jshint browser:true, camelcase:true, curly:true, es3:true, eqeqeq:true,
   immed:true, indent:4, latedef:true, newcap:false, quotmark:double,
   strict:false, undef:true, unused:true */

/* global ActiveXObject */
/* exported init, positionUpdate, OnLoadSnapshot, OnAbortErrorSnapshot,
   OnCommand, playerInit */


var filePath;
var curPos;
var len;
var state;
var pbr;
var eta;
var volume;
var muted; /*-1 no sound*/
var startTime = new Date().getTime();
var sliderSize = 500;
var sliderButtonWidth = 15;
var vsb = 10;
var vss = 100;
var vs;
var sc = 0;
var rdirt;
var Live;
var re;
var sas;
var cpf;
var cp;
var s;
var m;
var AP;
var RL;
var rpt;
var sb1;
var sb2;
var sb3;
var vs1;
var vs2;
var vs3;
var etaup = false;
var httpRequestStatus;


function pad(number, len) {
    var str = "" + number;
    while (str.length < len) {
        str = "0" + str;
    }
    return str;
}

function timeSyntax(ts) {
    var a;
    var b = "";
    for (a = 0; a < ts.length; a++) {
        switch (ts.charAt(a)) {
        case "0":
            b += "0";
            break;
        case "1":
            b += "1";
            break;
        case "2":
            b += "2";
            break;
        case "3":
            b += "3";
            break;
        case "4":
            b += "4";
            break;
        case "5":
            b += "5";
            break;
        case "6":
            b += "6";
            break;
        case "7":
            b += "7";
            break;
        case "8":
            b += "8";
            break;
        case "9":
            b += "9";
            break;
        case ".":
            b += ".";
            break;
        case ":":
            b += ":";
            break;
        default:
            break;
        }
    }
    return b;
}

function parseTime(y) {
    var ts = timeSyntax(y);
    var t = 0;
    var p1 = ts.indexOf(".");
    var p2 = ts.indexOf(":");
    var p3 = ts.indexOf(":", p2 + 1);
    var p4 = ts.indexOf(":", p3 + 1);

    if (p4 !== -1 || (p1 !== -1 && p2 !== -1 && p2 > p1) || (p1 !== -1 && p3 !== -1 && p3 > p1)) {
        return -2000;
    }
    p1 = (p1 === -1 ? ts.length + 1 : p1);
    if (p2 === -1) {
        t = parseFloat((ts + " ").substring(0, p1 + 4));
    }
    if (p2 !== -1 && p3 === -1) {
        t = parseInt(ts.substring(0, p2), 10) * 60 + parseFloat("0" + (ts + " ").substring(p2 + 1, p1 + 4));
    }
    if (p2 !== -1 && p3 !== -1) {
        t = parseInt(ts.substring(0, p2), 10) * 3600 + parseInt(ts.substring(p2 + 1, p3), 10) * 60 + parseFloat("0" + (ts + " ").substring(p3 + 1, p1 + 4));
    }
    return t;
}

function update(a, b) {
    if (a === -2000) {
        return false;
    }
    if (b) {
        m = (curPos = ((a > len && !Live) ? len : (a < 0 ? 0 : a))) * sliderSize / len;
    } else {
        curPos = (m = (a > sliderSize ? sliderSize : (a < 0 ? 0 : a))) * len / sliderSize;
    }
    if (m > sb1.width) {
        sb3.width = sliderSize - Math.floor(m);
        sb1.width = m;
    } else {
        sb1.width = m;
        sb3.width = sliderSize - sb1.width;
    }
    return true;
}

function secondsToTS(a, b) {
    var a1 = Math.floor(a / 3600000);
    var a2 = Math.floor(a / 60000) % 60;
    var a3 = Math.floor(a / 1000) % 60;
    var a4 = Math.floor(a) % 1000;
    var a1s = pad(a1.toString(), 2);
    var a2s = pad(a2.toString(), 2);
    var a3s = pad(a3.toString(), 2);
    var a4s = pad(a4.toString(), 3);
    switch (b) {
    case 1:
        return a1s;
    case 2:
        return a2s;
    case 3:
        return a3s;
    case 4:
        return a4s;
    case 5:
    case 6:
    case 7:
        return a1s + ":" + a2s + ":" + a3s;
    default:
        return ((a1 > 0 ? (a1s + ":") : "") + a2s + ":" + a3s);
    }
    return "bahh";
}

function postForm(wmc, ext, extv) {
    document.getElementById("fwmc").value = wmc;
    document.getElementById("fextra").value = extv;
    document.getElementById("fextra").name = ext;
    document.getElementById("ef").submit();
    return true;
}

function autoplay(a) {
    if (etaup && re.checked === true) {
        etaup = false;
        RL = setTimeout(function () {
            etaup = true;
            if (re.checked === true) {
                postForm(0, "null", 0);
            }
        }, 5000);
    }
    AP = setTimeout(autoplay, rdirt);
    var ct = new Date().getTime();
    var cap = pbr * (ct - startTime);
    if (cap > len && !Live) {
        if (re.checked === true) {
            RL = setTimeout(function () {
                window.location = window.location;
            }, 5000);
        }
    }
    cap = ((cap > len && !Live) ? len : (cap < 0 ? 0 : cap));
    if (sas.checked === true || a === true) {
        update(cap, true);
        cpf.value = secondsToTS(cap, 5, false);
    }
    var gg = " " + secondsToTS(cap, 5, true) + " ";
    cp.innerHTML = gg;
    rpt = cap;
    return true;
}

function getOffsetX(m) {
    var x = m.offsetLeft;
    while (m.offsetParent) {
        x += (m = m.offsetParent).offsetLeft;
    }
    return x;
}

function sliderClick(e) {
    update((window.event ? window.event.clientX - 3 : e.clientX) + document.body.scrollLeft - getOffsetX(s) - Math.floor(sliderButtonWidth / 2) + sc, false);
    cpf.value = secondsToTS(curPos, 5, false);
    sas.checked = false;
    return true;
}

function volumeUpdate(a, b) {
    if (b) {
        m = (volume = ((a > 100) ? 100 : (a < 0 ? 0 : a))) * vss / 100;
    } else {
        volume = (m = (a > vss ? vss : (a < 0 ? 0 : a))) * 100 / vss;
    }
    volume = Math.ceil(volume);
    vs1.width = m;
    vs3.width = vss - vs1.width;
    return true;
}

function volSliderClick(e) {
    var ret = volumeUpdate((window.event ? window.event.clientX - 3 : e.clientX) + document.body.scrollLeft - getOffsetX(vs) - Math.floor(vsb / 2) + sc, false);
    return ret;
}


if (eta === 0) {
    eta = (state < 0 && filePath.length > 0) ? 2 : 120;
}

function init(_filePath, _curPos, _length, _state, _pbr, _eta, _volume, _muted) {
    filePath = _filePath;
    curPos = _curPos;
    len = _length;
    state = _state;
    pbr = _pbr;
    eta = _eta;
    volume = _volume;
    muted = _muted;

    if (eta > 0) {
        RL = setTimeout(function () {
            etaup = true;
            if (re.checked === true) {
                postForm(0, "null", 0);
            }
        }, 1000 * eta);
    }
    Live = len < 1;
    startTime = startTime - curPos;
    rdirt = len * pbr / sliderSize;
    rdirt = Math.floor(rdirt > 1000 ? 1000 : (rdirt < 300 ? 300 : rdirt));
    cpf = document.getElementById("pos");
    cp = document.getElementById("time");
    sas = document.getElementById("SliderAutoScroll");
    re = document.getElementById("reloadenabled");
    s = document.getElementById("slider");
    sb1 = document.getElementById("c1");
    sb2 = document.getElementById("c2");
    sb3 = document.getElementById("c3");
    vs = document.getElementById("v");
    vs1 = document.getElementById("v1");
    vs2 = document.getElementById("v2");
    vs3 = document.getElementById("v3");
    document.getElementById("muted").innerHTML = muted === -1 ? "X" : muted === 1 ? "M" : "&nbsp;&nbsp;";
    s.height = sb1.height = sb2.height = sb3.height = vs.height = vs1.height = vs2.height = vs3.height = 20;
    s.width = sliderSize + (sb2.width = sliderButtonWidth);
    vs.width = vss + (vs2.width = vsb);
    sb1.onclick = sb2.onclick = sb3.onclick = sliderClick;
    vs1.onclick = vs2.onclick = vs3.onclick = volSliderClick;
    sas.checked = true;
    cp.innerHTML = cpf.value = secondsToTS(curPos, 5, false);
    rpt = curPos;
    if (state === 2 && pbr !== 0) {
        autoplay();
    }
    volumeUpdate(volume, true);
    return update(curPos, true);
}

function positionUpdate() {
    if (event.keyCode < 46 || event.keyCode > 58 || event.keyCode === 47) {
        return false;
    }
    setTimeout(function () {
        update(parseFloat(parseTime(cpf.value)), true);
    }, 1);
    return true;
}



//player.html
function getXMLHTTP() {
    try {
        return new ActiveXObject("Msxml2.XMLHTTP");
    } catch (e) {
        try {
            return new ActiveXObject("Microsoft.XMLHTTP");
        } catch (err) {}
    }
    if (typeof XMLHttpRequest !== "undefined") {
        return new XMLHttpRequest();
    }
    return null;
}

function MakeRequest(req) {
    var httpRequest = getXMLHTTP();
    try {
        httpRequest.open("GET", req, true);
        httpRequest.send(null);
    } catch (e) {}
}

function OnStatus (title, status, pos, posStr, dur, durStr, muted, volume) {
    var maxTitle = 70;
    var e;
    if (title.length > maxTitle) {
        title = title.substr(0, maxTitle - 3) + "...";
    }
    var timestr = dur > 0 && posStr && durStr ? posStr + "&nbsp;/&nbsp;" + durStr : "&nbsp;";
    if (!dur || dur === 0) {
        dur = 1;
    }

    e = document.getElementById("title");
    if (e) {
        e.innerHTML = title;
    }

    var sbpercent = Math.floor(100 * pos / dur);
    e = document.getElementById("seekbarchleft");
    if (e) {
        e.width = sbpercent > 0 ? sbpercent + "%" : "1px";
    }

    e = document.getElementById("seekbarchright");
    if (e) {
        e.width = sbpercent < 100 ? (100 - sbpercent) + "%" : "1px";
    }

    e = document.getElementById("status");
    if (e && e.innerHTML !== status) {
        e.innerHTML = status;
    }

    e = document.getElementById("timer");
    if (e && e.innerHTML !== timestr) {
        e.innerHTML = timestr;
    }

    e = document.getElementById("controlvolumemute");
    if (e) {
        var url = "url(img/controlvolume" + (muted ? "off" : "on") + ".png)";
        if (e.style.backgroundImage !== url) {
            e.style.backgroundImage = url;
        }
    }

    e = document.getElementById("controlvolumegrip");
    if (e) {
        volume = (document.getElementById("controlvolumebar").offsetWidth - e.offsetWidth) * volume / 100;
        e.style.position = "relative";
        e.style.top = "2px";
        e.style.left = Math.floor(volume) + "px";
    }
}

function OnReadyStateChange() {
    var statusRegExp = /OnStatus\("(.*)", "(.*)", (\d+), "(.*)", (\d+), "(.*)", (\d+), (\d+), "(.*)"\)/;
    if (httpRequestStatus && httpRequestStatus.readyState === 4 && httpRequestStatus.responseText) {
        if (httpRequestStatus.responseText.charAt(0) !== "<") {
            var params = statusRegExp.exec(httpRequestStatus.responseText);
            OnStatus(params[1], params[2], parseInt(params[3], 10), params[4], parseInt(params[5], 10), params[6], parseInt(params[7], 10), parseInt(params[8], 10), params[9]);
        } else {
            alert(httpRequestStatus.responseText);
        }
        httpRequestStatus = null;
    }
}

function StatusLoop() {
    if (!httpRequestStatus || httpRequestStatus.readyState === 0) {
        httpRequestStatus = getXMLHTTP();
        try {
            httpRequestStatus.open("GET", "status.html", true);
            httpRequestStatus.onreadystatechange = OnReadyStateChange;
            httpRequestStatus.send(null);
        } catch (e) {}
    }
    setTimeout(StatusLoop, 500);
}

var snapshotCounter = 0;

function LoadSnapshot() {
    var img = document.getElementById("snapshot");
    if (img) {
        img.src = "snapshot.jpg" + "?" + snapshotCounter++;
    }
}

function OnLoadSnapshot() {
    setTimeout(LoadSnapshot, 5000);
}

function OnAbortErrorSnapshot() {
    setTimeout(LoadSnapshot, 10000);
}

function OnSeek(e) {
    var left = 0;
    var right = 0;
    var percent;
    var sb;

    sb = document.getElementById("seekbarchleft");
    if (sb) {
        left = getOffsetX(sb);
    }

    sb = document.getElementById("seekbarchright");
    if (sb) {
        right = getOffsetX(sb) + sb.offsetWidth;
    }

    sb = document.getElementById("seekbargrip");
    if (sb) {
        left += sb.offsetWidth / 2;
        right -= sb.offsetWidth / 2;
    }
    if (left > 0 && left < right) {
        percent = 100 * ((window.event ? window.event.clientX : e.clientX) - left) / (right - left);
        if (percent < 0) {
            percent = 0;
        } else if (percent > 100) {
            percent = 100;
        }
        MakeRequest("command.html?wm_command=[setposcommand]&percent=" + percent);
    }
}

function OnVolume(e) {
    var left = 0;
    var right = 0;
    var percent;
    var cv = document.getElementById("controlvolumebar");

    if (cv) {
        left = getOffsetX(cv) + 3;
        right = getOffsetX(cv) + cv.offsetWidth - 3;
    }
    if (left > 0 && left < right) {
        percent = 100 * ((window.event ? window.event.clientX : e.clientX) - left) / (right - left);
        if (percent < 0) {
            percent = 0;
        } else if (percent > 100) {
            percent = 100;
        }
        MakeRequest("command.html?wm_command=[setvolumecommand]&volume=" + percent);
    }
}

function OnCommand(id) {
    MakeRequest("command.html?wm_command=" + id);
}

function playerInit() {
    StatusLoop();
    LoadSnapshot();

    var e = document.getElementById("seekbar");
    if (e) {
        e.onclick = OnSeek;
    }

    e = document.getElementById("controlvolumebar");
    if (e) {
        e.onclick = OnVolume;
    }
}
