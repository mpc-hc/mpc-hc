/* jshint forin:true, noarg:true, noempty:true, eqeqeq:true, bitwise:true, camelcase:true, trailing:true,
   strict:false, undef:false, boss:true, unused:true, curly:true, browser:true, indent:4, maxerr:100 */

var filePath;
var curPos;
var length;
var state;
var pbr;
var eta;
var volume;
var muted; /*-1 no sound*/
var startTime = (new Date()).getTime();
var sliderSize = 500;
var sliderButtonWidth = 15;
var vsb = 10;
var vss = 100;
var sc = 0;
var rdirt;
var AP;
var RL;
var rpt;
var etaup = false;

if (eta === 0) {
	eta = (state < 0 && filePath.length > 0) ? 2 : 120;
}

function init(_filePath, _curPos, _length, _state, _pbr, _eta, _volume, _muted) {
	filePath = _filePath;
	curPos = _curPos;
	length = _length;
	state = _state;
	pbr = _pbr;
	eta = _eta;
	volume = _volume;
	muted = _muted;

	if (eta > 0) {
		RL = setTimeout("etaup=true; if (re.checked===true) {postForm(0,'null',0);}", 1000 * eta);
	}
	Live = (length < 1);
	startTime = startTime - curPos;
	rdirt = length * pbr / sliderSize;
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

function autoplay(a) {
	if (etaup && re.checked === true) {
		etaup = false;
		RL = setTimeout("etaup=true; if (re.checked===true) {postForm(0,'null',0);}", 5000);
	}
	AP = setTimeout(autoplay, rdirt);
	var ct = (new Date()).getTime();
	var cap = pbr * (ct - startTime);
	if (cap > length && !Live) {
		if (re.checked === true) {
			RL = setTimeout("window.location=window.location;", 5000);
		}
	}
	cap = ((cap > length && !Live) ? length : (cap < 0 ? 0 : cap));
	if (sas.checked === true || a === true) {
		update(cap, true);
		cpf.value = secondsToTS(cap, 5, false);
	}
	var gg = " " + secondsToTS(cap, 5, true) + " ";
	cp.innerHTML = gg;
	rpt = cap;
	return true;
}

function pad(number, length) {
	var str = "" + number;
	while (str.length < length) {
		str = "0" + str;
	}
	return str;
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

function parseTime(y) {
	ts = timeSyntax(y);
	t = 0;
	p1 = ts.indexOf(".");
	p2 = ts.indexOf(":");
	p3 = ts.indexOf(":", p2 + 1);
	p4 = ts.indexOf(":", p3 + 1);
	if (p4 !== -1 || (p1 !== -1 && p2 !== -1 && p2 > p1) || (p1 !== -1 && p3 !== -1 && p3 > p1)) {
		return -2000;
	}
	p1 = (p1 === -1 ? ts.length + 1 : p1);
	if (p2 === -1) {
		t = parseFloat((ts + " ").substring(0, p1 + 4));
	}
	if (p2 !== -1 && p3 === -1) {
		t = parseInt(ts.substring(0, p2)) * 60 + parseFloat("0" + (ts + " ").substring(p2 + 1, p1 + 4));
	}
	if (p2 !== -1 && p3 !== -1) {
		t = parseInt(ts.substring(0, p2)) * 3600 + parseInt(ts.substring(p2 + 1, p3)) * 60 + parseFloat("0" + (ts + " ").substring(p3 + 1, p1 + 4));
	}
	return t;
}

function update(a, b) {
	if (a === -2000) {
		return false;
	}
	if (b) {
		m = (curPos = ((a > length && !Live) ? length : (a < 0 ? 0 : a))) * sliderSize / length;
	} else {
		curPos = (m = (a > sliderSize ? sliderSize : (a < 0 ? 0 : a))) * length / sliderSize;
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

function sliderClick(e) {
	update((window.event ? window.event.clientX - 3 : e.clientX) + document.body.scrollLeft - getOffsetX(s) - Math.floor(sliderButtonWidth / 2) + sc, false);
	cpf.value = secondsToTS(curPos, 5, false);
	sas.checked = false;
	return true;
}

function getOffsetX(m) {
	var x = m.offsetLeft;
	while (m.offsetParent) {
		x += (m = m.offsetParent).offsetLeft;
	}
	return x;
}

function positionUpdate() {
	if (event.keyCode < 46 || event.keyCode > 58 || event.keyCode === 47) {
		return false;
	}
	self.setTimeout("update(parseFloat(parseTime(cpf.value)),true)", 1);
	return true;
}

function timeSyntax(ts) {
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
	return volumeUpdate((window.event ? window.event.clientX - 3 : e.clientX) + document.body.scrollLeft - getOffsetX(vs) - Math.floor(vsb / 2) + sc, false);
}

function postForm(wmc, ext, extv) {
	document.getElementById("fwmc").value = wmc;
	document.getElementById("fextra").value = extv;
	document.getElementById("fextra").name = ext;
	document.getElementById("ef").submit();
	return true;
}


/*player.html*/
function getXMLHTTP() {
	try {
		return new ActiveXObject("Msxml2.XMLHTTP");
	} catch (e) {
		try {
			return new ActiveXObject("Microsoft.XMLHTTP");
		} catch (e) {}
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

function getOffsetX(m) {
	var x = m.offsetLeft;
	while (m.offsetParent) {
		x += (m = m.offsetParent).offsetLeft;
	}
	return x;
}

OnStatus = function (title, status, pos, posStr, dur, durStr, muted, volume, filePath) {
	var maxTitle = 70;
	if (title.length > maxTitle) {
		title = title.substr(0, maxTitle - 3) + "...";
	}
	var timestr = dur > 0 && posStr && durStr ? posStr + "&nbsp;/&nbsp;" + durStr : "&nbsp;";
	if (!dur || dur === 0) {
		dur = 1;
	}
	var sbpercent = Math.floor(100 * pos / dur);
	if (e = document.getElementById("title")) {
		e.innerHTML = title;
	}
	if (e = document.getElementById("seekbarchleft")) {
		e.width = sbpercent > 0 ? sbpercent + "%" : "1px";
	}
	if (e = document.getElementById("seekbarchright")) {
		e.width = sbpercent < 100 ? (100 - sbpercent) + "%" : "1px";
	}
	if ((e = document.getElementById("status")) && e.innerHTML !== status) {
		e.innerHTML = status;
	}
	if ((e = document.getElementById("timer")) && e.innerHTML !== timestr) {
		e.innerHTML = timestr;
	}
	if (e = document.getElementById("controlvolumemute")) {
		url = "url(img/controlvolume" + (muted ? "off" : "on") + ".png)";
		if (e.style.backgroundImage !== url) {
			e.style.backgroundImage = url;
		}
	}
	if (e = document.getElementById("controlvolumegrip")) {
		volume = (document.getElementById("controlvolumebar").offsetWidth - e.offsetWidth) * volume / 100;
		e.style.position = "relative";
		e.style.top = "2px";
		e.style.left = Math.floor(volume) + "px";
	}
};

var httpRequestStatus;
var statusRegExp = /OnStatus\("(.*)", "(.*)", (\d+), "(.*)", (\d+), "(.*)", (\d+), (\d+), "(.*)"\)/;

function OnReadyStateChange() {
	if (httpRequestStatus && httpRequestStatus.readyState === 4 && httpRequestStatus.responseText) {
		if (httpRequestStatus.responseText.charAt(0) !== "<") {
			var params = statusRegExp.exec(httpRequestStatus.responseText);
			OnStatus(params[1], params[2], parseInt(params[3]), params[4], parseInt(params[5]), params[6], parseInt(params[7]), parseInt(params[8]), params[9]);
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
	if (img = document.getElementById("snapshot")) {
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
	left = right = 0;
	if (sb = document.getElementById("seekbarchleft")) {
		left = getOffsetX(sb);
	}
	if (sb = document.getElementById("seekbarchright")) {
		right = getOffsetX(sb) + sb.offsetWidth;
	}
	if (sb = document.getElementById("seekbargrip")) {
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
	left = right = 0;
	if (cv = document.getElementById("controlvolumebar")) {
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
	if (e = document.getElementById("seekbar")) {
		e.onclick = OnSeek;
	}
	if (e = document.getElementById("controlvolumebar")) {
		e.onclick = OnVolume;
	}
}
