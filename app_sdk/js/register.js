var timer_id = null;

function back_login_html() {
	if(timer_id != null) {
		clearTimeout(timer_id);
	}

	closeWin("register");
}

//验证获取60秒倒计时
function settime(timer, button_id) {
	//获取验证码按钮对象节点
	var get_vercode_button = document.getElementById(button_id);

	if(timer > 0) {
		get_vercode_button.innerHTML = timer + "秒";
		timer--;
	} else {
		get_vercode_button.innerHTML = "获取验证码";
		//get_vercode_button.disabled = false; //设置按钮有效
		document.getElementById(button_id).setAttribute("onclick", "getCode(60, this.id)");
		return true;
	}

	timer_id = setTimeout(function() { //每1000毫秒重新调用此方法
		settime(timer, button_id);
	}, 1000);
}

//获取验证码
function getCode(timer, button_id) {
	var loginname = null;

	loginname = document.getElementById("register_phone_num").value;

	var param = {
		loginname: loginname, //拿到手机号
		appid: _APPID //_APPID
	};

	//	alert(JSON.stringify(param));

	if(monitor_net() == "none") {
		return false;
	}

	if(check_phone_number(loginname) == true) {

		document.getElementById("register_vercode").value = "";
		document.getElementById("register_passwd").value = "";
		
		//alert(JSON.stringify(param));
		
		mico2.getVerifyCode(param, function(ret, err) {
			if(ret && ret.meta.code == 0) {
				var get_vercode_button = document.getElementById(button_id);
				//get_vercode_button.disabled = true; //设置按钮失效
				get_vercode_button.removeAttribute("onclick");

				settime(timer, button_id); //验证码获取成功，60秒倒计时

			} else {
				alert("获取验证码错误:" + JSON.stringify(ret) + JSON.stringify(err));
			}
		});
	}

}

//设置密码
function setPassword(password, token, register_name, register_pwd, client_id) {
	var param = {
		password: password,
		token: token
	};

	if(monitor_net() == "none") {
		return false;
	}

	mico2.setPassword(param, function(ret, err) {
		try {
			if(ret && ret.meta.code == 0) {
				if(timer_id != null) {
					clearTimeout(timer_id);
				}

				localStorage.setItem(_USERNAME, register_name);
				localStorage.setItem(_PASSWORD, register_pwd);
				localStorage.setItem(_TOKEN, token);
				localStorage.setItem(_CLINET_ID, client_id);
				openWin('login', 'widget://html/login.html', true);

			} else {
				alert("注册失败");
				alert(JSON.stringify(ret) + JSON.stringify(err));
			}
		} catch(e) {
			alert("注册失败,请重试");
		}

	});
}

//用户注册
function do_register() {
	var register_name = document.getElementById('register_phone_num').value;
	var register_pwd = document.getElementById('register_passwd').value;
	var register_pwd_repeat = document.getElementById('register_passwd_repeat').value;
	var register_code = document.getElementById('register_vercode').value;
	
	if(register_pwd != register_pwd_repeat){
		alert("两次输入的注册密码不同！请重新输入");
		document.getElementById('register_passwd').value = "";
		document.getElementById('register_passwd_repeat').value = "";
		return false;
	}
	
	if((check_phone_number(register_name) == false) || (check_password(register_pwd) == false))
		return false;

	var param = {
		loginname: register_name,
		vercode: register_code,
		appid: _APPID
	};

	if(monitor_net() == "none") {
		return false;
	}

	mico2.checkVerifyCode(param, function(ret, err) {
		if(ret.token) {
			setPassword(register_pwd, ret.token, register_name, register_pwd, ret.clientid);
		} else {
			alert("验证码错误或已过期");
		}
	});

}

//注册页面监听Android的系统返回键
function register_page_listen_keyback(){
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		back_login_html();
	});
}
