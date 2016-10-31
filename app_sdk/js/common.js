/*校验手机号是否合法*/
function check_phone_number(phone_num) {
	if((!/^1[34578]\d{9}$/.test(phone_num))) {
		alert("手机号码有误，请重填");
		return false;
	}
	return true;
}

/*校验密码是否合法*/
function check_password(passwd) {
	if(!/^[a-zA-Z]\w{5,17}$/.test(passwd)) {
		alert("密码以字母开头，长度在6-18之间，只能包含字符、数字和下划线");
		return false;
	}

	return true;
}

function hslToRgb(h, s, l) {
	var r, g, b;

	if(s == 0) {
		r = g = b = l; // achromatic
	} else {
		function hue2rgb(p, q, t) {
			if(t < 0) t += 1;
			if(t > 1) t -= 1;
			if(t < 1 / 6) return p + (q - p) * 6 * t;
			if(t < 1 / 2) return q;
			if(t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
			return p;
		}

		var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
		var p = 2 * l - q;
		r = hue2rgb(p, q, h + 1 / 3);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1 / 3);
	}

	return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}