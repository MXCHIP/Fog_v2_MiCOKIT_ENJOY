//退出当前pwm控制页面
function quit_pwm_html() {
	closeWin();
}

//pwm改变回调函数
function pwm_change(pwm_obj) {
	//mui.toast(pwm_obj.value);
	
	var data_array = new Array();
	data_array[0] = {
		PID: api.pageParam.pid,
		T: api.pageParam.type,
		D: parseInt(pwm_obj.value)
	}
	
	app_command_id = get_app_command_id();
	
	var app_command = {
		command_id: app_command_id,
		CMD: C2D_SEND_COMMANDS,
		G: data_array
	}

	//alert(JSON.stringify(app_command));
	send_message_to_device(JSON.stringify(app_command));
}

//PWM页面监听Android返回按键
function pwm_page_listen_keyback(){
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		quit_pwm_html();
	});
}