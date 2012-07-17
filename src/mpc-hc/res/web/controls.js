var filepath;
var curpos;
var length;
var state;
var pbr;
var eta;
var volume;
var muted; /*-1 no sound*/
var starttime = (new Date()).getTime();
var slidersize = 500;
var sliderbuttonwidth = 15;
var vsb = 10;
var vss = 100;
var sc = 0
var rdirt;
var AP;
var RL;
var rpt;
var etaup = false;
if (eta == 0) eta = (state < 0 && filepath.length > 0) ? 2 : 120;

function init(_filepath, _curpos, _length, _state, _pbr, _eta, _volume, _muted) {
	filepath = _filepath;
	curpos = _curpos;
	length = _length;
	state = _state;
	pbr = _pbr;
	eta = _eta;
	volume = _volume;
	muted = _muted;

	if (eta > 0) RL = setTimeout("etaup=true; if (re.checked==true) postForm(0,'null',0);", 1000 * eta);
	Live = (length < 1);
	starttime = starttime - curpos;
	rdirt = length * pbr / slidersize;
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
	document.getElementById("muted").innerHTML = muted == -1 ? "X" : muted == 1 ? "M" : "&nbsp;&nbsp;";
	s.height = sb1.height = sb2.height = sb3.height = vs.height = vs1.height = vs2.height = vs3.height = 20;
	s.width = slidersize + (sb2.width = sliderbuttonwidth);
	vs.width = vss + (vs2.width = vsb);
	sb1.onclick = sb2.onclick = sb3.onclick = sliderClick;
	vs1.onclick = vs2.onclick = vs3.onclick = volSliderClick;
	sas.checked = true;
	/*g = " " + secondsToTS(curpos, 0, true) + " " + x < 0 ? ("Buffering %" + (-x - 1).toString()):"";*/
	cp.innerHTML = cpf.value = secondsToTS(curpos, 5, false);
	rpt = curpos;
	if (state == 2 && pbr != 0) autoplay();
	volumeUpdate(volume, true);
	return update(curpos, true);
}

function autoplay(a) {
	if (etaup && re.checked == true) {
		etaup = false;
		RL = setTimeout("etaup=true; if (re.checked==true) postForm(0,'null',0);", 5000);
	}
	AP = setTimeout("autoplay()", rdirt);
	var ct = (new Date()).getTime();
	var cap = pbr * (ct - starttime);
	if (cap > length && !Live) if (re.checked == true) RL = setTimeout("window.location=window.location", 5000);
	cap = ((cap > length && !Live) ? length : (cap < 0 ? 0 : cap));
	if (sas.checked == true || a == true) {
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
	while (str.length < length) str = "0" + str;
	return str;
}

function secondsToTS(a, b, c) {
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
		/*return a1s+":"+a2s+":"+a3s+"."+a4s;*/
	case 6:
		/*return ((a1>0?(a1s+":"):"")+a2s+":"+a3s+"."+a4s);*/
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
	if (p4 != -1 || (p1 != -1 && p2 != -1 && p2 > p1) || (p1 != -1 && p3 != -1 && p3 > p1)) return -2000;
	p1 = (p1 == -1 ? ts.length + 1 : p1);
	if (p2 == -1) t = parseFloat((ts + " ").substring(0, p1 + 4));
	if (p2 != -1 && p3 == -1) t = parseInt(ts.substring(0, p2)) * 60 + parseFloat("0" + (ts + " ").substring(p2 + 1, p1 + 4));
	if (p2 != -1 && p3 != -1) t = parseInt(ts.substring(0, p2)) * 3600 + parseInt(ts.substring(p2 + 1, p3)) * 60 + parseFloat("0" + (ts + " ").substring(p3 + 1, p1 + 4));
	return t;
}

function update(a, b) {
	if (a == -2000) return false;
	if (b) {
		m = (curpos = ((a > length && !Live) ? length : (a < 0 ? 0 : a))) * slidersize / length;
	} else {
		curpos = (m = (a > slidersize ? slidersize : (a < 0 ? 0 : a))) * length / slidersize;
	}
	if (m > sb1.width) {
		sb3.width = slidersize - Math.floor(m);
		sb1.width = m;
	} else {
		sb1.width = m;
		sb3.width = slidersize - sb1.width;
	}
	return true;
}

function sliderClick(e) {
	update((window.event ? window.event.clientX - 3 : e.clientX) + document.body.scrollLeft - getOffsetX(s) - Math.floor(sliderbuttonwidth / 2) + sc, false);
	cpf.value = secondsToTS(curpos, 5, false);
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
	if (event.keyCode < 46 || event.keyCode > 58 || event.keyCode == 47) return false;
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