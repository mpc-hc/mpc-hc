function getXMLHTTP() {
	try {
		return new ActiveXObject("Msxml2.XMLHTTP");
	} catch (e) {
		try {
			return new ActiveXObject("Microsoft.XMLHTTP");
		} catch (e) {}
	}
	if (typeof XMLHttpRequest != "undefined") {
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

OnStatus = function (title, status, pos, posstr, dur, durstr, muted, volume, filepath) {
	var maxtitle = 70;
	if (title.length > maxtitle)
		title = title.substr(0, maxtitle - 3) + "...";
	var timestr = dur > 0 && posstr && durstr ? posstr + "&nbsp;/&nbsp;" + durstr : "&nbsp;";
	if (!dur || dur == 0)
		dur = 1;
	var sbpercent = Math.floor(100 * pos / dur);
	if (e = document.getElementById("title"))
		e.innerHTML = title;
	if (e = document.getElementById("seekbarchleft"))
		e.width = sbpercent > 0 ? sbpercent + "%" : "1px";
	if (e = document.getElementById("seekbarchright"))
		e.width = sbpercent < 100 ? (100 - sbpercent) + "%" : "1px";
	if ((e = document.getElementById("status")) && e.innerHTML != status)
		e.innerHTML = status;
	if ((e = document.getElementById("timer")) && e.innerHTML != timestr)
		e.innerHTML = timestr;
	if (e = document.getElementById("controlvolumemute")) {
		url = "url(images/controlvolume" + (muted ? "off" : "on") + ".png)";
		if (e.style.backgroundImage != url)
			e.style.backgroundImage = url;
	}
	if (e = document.getElementById("controlvolumegrip")) {
		volume = (document.getElementById("controlvolumebar").offsetWidth - e.offsetWidth) * volume / 100;
		e.style.position = "relative";
		e.style.top = "2px";
		e.style.left = Math.floor(volume) + "px";
	}
}

var httpRequestStatus;

function OnReadyStateChange() {
	if (httpRequestStatus && httpRequestStatus.readyState == 4 && httpRequestStatus.responseText) {
		if (httpRequestStatus.responseText.charAt(0) != "<") {
			eval(httpRequestStatus.responseText.replace(/\\/g, "\\\\"));
		} else {
			alert(httpRequestStatus.responseText);
		}
		httpRequestStatus = null;
	}
}

function StatusLoop() {
	if (!httpRequestStatus || httpRequestStatus.readyState == 0) {
		httpRequestStatus = getXMLHTTP();
		try {
			httpRequestStatus.open("GET", "status.html", true);
			httpRequestStatus.onreadystatechange = OnReadyStateChange;
			httpRequestStatus.send(null);
		} catch (e) {}
	}
	setTimeout("StatusLoop()", 500);
}

var snapshotcounter = 0;

function LoadSnapShot() {
	if (img = document.getElementById("snapshot")) {
		img.src = "snapshot.jpg" + "?" + snapshotcounter++;
	}
}

function OnLoadSnapShot() {
	setTimeout("LoadSnapShot()", 5000);
}

function OnAbortErrorSnapShot(e) {
	setTimeout("LoadSnapShot()", 10000);
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

function Init() {
	StatusLoop();
	LoadSnapShot();
	if (e = document.getElementById("seekbar")) {
		e.onclick = OnSeek;
	}
	if (e = document.getElementById("controlvolumebar")) {
		e.onclick = OnVolume;
	}
}