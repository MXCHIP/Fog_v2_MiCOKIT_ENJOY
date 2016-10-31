function click_register() {	
	openWin("register", "widget://html/register.html", true);
}

//用户登录
function login() {
	var login_name = null;
	var login_pwd = null;

	login_name = document.getElementById('login_phone_num').value;
	login_pwd = document.getElementById('login_passwd').value;

	if((check_phone_number(login_name) == false))
		return false;

	var param = {
		loginname: login_name,
		password: login_pwd,
		appid: _APPID
	};
	
//	alert(JSON.stringify(param));

	if(monitor_net() == "none"){
		return false;
	}

	mico2.login(param, function(ret, err) {
		if(ret && ret.token) {
//			mui.toast("登录成功");
			localStorage.setItem(_USERNAME, login_name);
			localStorage.setItem(_PASSWORD, login_pwd);
			localStorage.setItem(_TOKEN, ret.token);
			localStorage.setItem(_CLINET_ID, ret.clientid);
//			closeWin('login');
			openWin('index', 'widget://html/index.html', true);
			return true;
		} else {
			document.getElementById('login_passwd').value = '';
			localStorage.removeItem(_PASSWORD);
			localStorage.removeItem(_TOKEN);
			alert("登录失败，账号或密码错误");
			return false;
		}
	});
}

//读取存储在本地的用户名和密码,填充到输入框中
function get_storage_data() {
	var phone_num;
	var passwd;

	phone_num = localStorage.getItem(_USERNAME);
	passwd = localStorage.getItem(_PASSWORD);

	if(phone_num != null)
		document.getElementById('login_phone_num').value = phone_num;

	if(passwd != null)
		document.getElementById('login_passwd').value = passwd;	
}

function on_click_forget_password(){
	alert("重新注册即可找回密码");
	click_register();
}

//退出登录
function un_login() {
    mui.confirm("确定要退出当前账号吗？", "退出提示", ["取消", "确定"], function(ret) {
        if (ret.index == 1) {
            localStorage.removeItem(_TOKEN);
            closeWin('index');
        }
    });
}

function quit_login() {
	un_login();
}

