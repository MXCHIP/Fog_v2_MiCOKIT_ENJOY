var device_online_listen = false;
var add_subdevice_timer;
var recv_data_count = 0;
var is_mqtt_client_connect = false;

//获取透传模式开关的状态
function get_data_transmission_switch_state() {
	var isActive = document.getElementById("data_transmission_switch").classList.contains("mui-active");
	return isActive;
}

//把接收到 数据显示到text接收区域
function display_recv_data_to_recv_area(payload){
	document.getElementById("recv_area").value += '[' + recv_data_count + '] '+ JSON.stringify(payload) + '\r\n';
	
	recv_data_count ++;
	
	document.getElementById("recv_area").scrollTop = document.getElementById("recv_area").scrollHeight;
}

//透传模式开关处理函数
function add_data_transmission() {

	document.getElementById("data_transmission_switch").addEventListener("toggle", function(event) {
		if(event.detail.isActive) {	
			document.getElementById("template").style.display = "none";
			document.getElementById("data_transmission_div").style.display = "block";
			document.getElementById("recv_area").value = "";
			recv_data_count = 0;
			mui.toast("清空接收区！");
		} else {
			document.getElementById("template").style.display = "block";
			document.getElementById("data_transmission_div").style.display = "none";
		}
	})

}

//数据发送
function add_data_transmission_send(){
	var send_area =	document.getElementById("send_area");
	
	if(send_area.value == '')
	{
		alert("数据为空，取消发送");
		return;
	}
	
	var send_data = {
		data : send_area.value
	}
	
	send_message_to_device(JSON.stringify(send_data));
	
	return;
}


//返回设备列表页面 在内部调用stopListenDevice 可能会导致Android app 无响应
function back_device_list_html() {
//	mico2.stopListenDevice(function(ret, err) {
//		if(ret) {
//			//alert("停止监听成功：" + JSON.stringify(ret));
//			send_event('refesh_device_list', 'peripherals');
//			closeWin();
//		} else {
//			alert("停止监听失败：" + JSON.stringify(err));
//			send_event('refesh_device_list', 'peripherals');
//			closeWin();
//		}
//	});
	
	send_event('refesh_device_list', 'peripherals');
	closeWin();
}

//处理设备在线离线通知
function process_device_cmd(mqtt_payload) {
	try 
	{
		var cmd_code = mqtt_payload.code;
		var storage_device_id = localStorage.getItem(_DEVICE_ID);
		var is_sub = localStorage.getItem(_IS_SUB_DEVICE);
		
		if(	cmd_code != _FOG_PUBLISH_DEVICE_ONLINE_CODE && cmd_code != _FOG_PUBLISH_DEVICE_OFFLINE_CODE && 
			cmd_code != _FOG_PUBLISH_DEVICE_RECOVERY_CODE && cmd_code != _FOG_PUBLISH_SUB_DEVICE_ONLINE_CODE &&
			cmd_code != _FOG_PUBLISH_SUB_DEVICE_OFFLINE_CODE && cmd_code != _FOG_PUSBLISH_SUB_DEVICE_ADD_TIMEOUT_CODE &&
			cmd_code != _FOG_PUSBLISH_SUB_DEVICE_ADD_TIMEOUT_CODE && cmd_code != _FOG_PUSBLISH_SUB_DEVICE_REGISTER_CODE &&
			cmd_code != _FOG_PUSBLISH_SUB_DEVICE_UNREGISTER_CODE )
		{
			alert("cmd code is error" + JSON.stringify(mqtt_payload));
		}
		
		
		//在线离线判断
		if(cmd_code == _FOG_PUBLISH_DEVICE_ONLINE_CODE || cmd_code == _FOG_PUBLISH_DEVICE_OFFLINE_CODE ||
				cmd_code == _FOG_PUBLISH_SUB_DEVICE_ONLINE_CODE || cmd_code == _FOG_PUBLISH_SUB_DEVICE_OFFLINE_CODE) 
		{
		
			var device_id = mqtt_payload.data.deviceid;
			if(device_id != storage_device_id) {
				alert("[上下线推送]device id 错误 " + JSON.stringify(mqtt_payload));
				return;
			}

			if(is_sub == "false") {
				if(cmd_code == _FOG_PUBLISH_DEVICE_ONLINE_CODE) {
					document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-green">在线</span>';
				} else if(cmd_code == _FOG_PUBLISH_DEVICE_OFFLINE_CODE) {
					document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-danger">离线</span>';
				}
			} else if(is_sub == "true") {
				if(cmd_code == _FOG_PUBLISH_SUB_DEVICE_ONLINE_CODE) {
					document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-green">在线</span>';
				} else if(cmd_code == _FOG_PUBLISH_SUB_DEVICE_OFFLINE_CODE) {
					document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-danger">离线</span>';
				}
			}else{
				alert("is_sub is error");
			}
		}

		//当前设备被删除 通知
		if(cmd_code == _FOG_PUBLISH_DEVICE_RECOVERY_CODE || cmd_code == _FOG_PUSBLISH_SUB_DEVICE_UNREGISTER_CODE) 
		{
			mui.toast("当前设备已被解绑");
			//back_device_list_html();
		}
		
		//当前设备被删除 通知
		if(cmd_code == _FOG_PUSBLISH_SUB_DEVICE_REGISTER_CODE || cmd_code == _FOG_PUSBLISH_SUB_DEVICE_ADD_TIMEOUT_CODE) 
		{
			if(cmd_code == _FOG_PUSBLISH_SUB_DEVICE_REGISTER_CODE)
			{
				alert("子设备添加成功");
			}else if(cmd_code == _FOG_PUSBLISH_SUB_DEVICE_ADD_TIMEOUT_CODE){
				alert("子设备添加失败");
			}
			
			set_add_subdevice_active();
		}
	}catch(e) {
		alert("云端推送消息错误" + JSON.stringify(mqtt_payload));
	}
}

//设置UI显示的 设备在线还是离线
function set_device_UI_offline(status) {
	if(status == true) {
		document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-green">在线</span>';
		//mui.toast("设备在线");
	} else if(status == false) {
		document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-danger">离线</span>';
		alert("设备离线");
	}
}


//设置UI是否显示添加子设备条目
function set_add_sub_device_UI(status) {
	if(status == 1) { //网关设备
		document.getElementById('add_sub_device').style.display = "block";
	} else {
		document.getElementById('add_sub_device').style.display = "none";
	}
}


//停止定时器
function stop_add_subdevice_timer(){
	if(add_subdevice_timer != null) {
		clearTimeout(add_subdevice_timer);
	}
}

//复原添加设备的button
function set_add_subdevice_active() {
	//获取验证码按钮对象节点
	var add_subdevice_button = document.getElementById("add_sub_device_button");

	add_subdevice_button.innerHTML = "添加";
	add_subdevice_button.disabled = false; //设置按钮有效
	add_subdevice_button.setAttribute("onclick", "sub_device_add()");
	
	stop_add_subdevice_timer();
	
	return;
}

//设置按钮倒计时
function set_add_subdevice_timer(timer, button_id) {
	//获取验证码按钮对象节点
	var add_subdevice_button = document.getElementById(button_id);

	if(timer > 0) {
		add_subdevice_button.disabled = true; //设置按钮无效
		add_subdevice_button.innerHTML = timer + "秒";
		timer--;
	} else {
		add_subdevice_button.innerHTML = "添加";	
		add_subdevice_button.disabled = false; //设置按钮有效
		add_subdevice_button.setAttribute("onclick", "sub_device_add()");
		
		return true;
	}

	add_subdevice_timer = setTimeout(function() { //每1000毫秒重新调用此方法
		set_add_subdevice_timer(timer, button_id);
	}, 1000);
}

////发送数据到fog
//function send_add_subdevice_msg(timeout, callback) {
//	var device_id = localStorage.getItem(_DEVICE_ID);
//
//	if(monitor_net() == "none") {
//		return false;
//	}
//
//	if(device_id == null) {
//		alert("读取设备device id错误");
//		back_device_list_html();
//	}
//
//	var param = {
//		deviceid: device_id,
//		productid: "",
//		timeout: timeout,
//		extend:""
//	};
//	
//	user_http_post("/enduser/addSubDevice/", param, function(ret, err) {
//		try {
//			if(ret && ret.meta.code == 0) {
//				//alert("发送数据成功");
//				set_add_subdevice_timer(timeout, "add_sub_device_button");				
//			} else {
//				alert("[发送失败]addsubdevice" + JSON.stringify(err) + JSON.stringify(ret));
//			}
//		} catch(e) {
//			alert("[发送失败]addsubdevice" + JSON.stringify(err) + JSON.stringify(ret));
//		}
//	});
//	
//}

//发送数据到fog
function send_add_subdevice_msg(timeout, callback) {
	var device_id = localStorage.getItem(_DEVICE_ID);
	var app_token = check_app_token();

	if(monitor_net() == "none") {
		return false;
	}

	if(device_id == null) {
		alert("读取设备device id错误");
		back_device_list_html();
	}

	var param = {
		deviceid: device_id,
		productid: "deb2616e-9c33-11e6-9d95-00163e103941",
		timeout: timeout,
		extend: "",
		token: app_token
	};
	
	console.log("addSubDevice() 参数" + JSON.stringify(param));
	
	mico2.addSubDevice(param, function(ret, err) {
		try {
			if(ret && ret.meta.code == 0) {
				//alert("发送数据成功");
				set_add_subdevice_timer(timeout, "add_sub_device_button");
			} else {
				alert("[发送失败]addsubdevice" + JSON.stringify(err) + JSON.stringify(ret));
			}
		} catch(e) {
			alert("[发送失败]addsubdevice" + JSON.stringify(err) + JSON.stringify(ret));
		}
	});

}


//进入添加子设备页面
function sub_device_add() {
	if(monitor_net() == "none") {
		back_device_list_html();
	}

	if(document.getElementById('per_device_online').innerText == "离线") {
		alert("当前设备离线,无法添加子设备");
		return;
	}
	
	send_add_subdevice_msg(30);
}


function goto_pwm_ctrl_html(li_obj) {
	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var arr = li_obj.id.split('_'); //id格式:PID+','+type
	var type = parseInt(arr[1]); //type
	var param = {
		pid: arr[0],
		type: arr[1],
		data_min: arr[2],
		data_max: arr[3]
	};

	switch(type) {
		case _MICO_TEMPLATE_TYPE_PWM_MOTOR:
			{
				openWin('pwm', 'widget://html/pwm.html', true, param);
			}
			break;
		default:
			break;
	}
}

function goto_private_html(li_obj) {
	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var arr = li_obj.id.split('_'); //id格式:PID+'_'+type
	var type = parseInt(arr[1]); //type
	var param = {
		pid: arr[0],
		type: arr[1]
	};

	//alert("param:" + JSON.stringify(param));

	switch(type) {
		case _MICO_TEMPLATE_TYPE_RGB_LED:
			{
				openWin('reb_led', 'widget://html/rgb_led.html', true, param);
			}
			break;
		default:
			break;
	}
}

//处理resume事件
function deal_resume_event(){
	get_device_info(false);
}

//获取设备信息
function get_device_info(is_listen_device) {
	var app_token = check_app_token();

	golobal_template_json_obj = null; //模板清零
	//alert("golobal_template_json_obj:" + JSON.stringify(golobal_template_json_obj));

	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var device_id = localStorage.getItem(_DEVICE_ID);
	if(device_id == null) {
		back_device_list_html();
	}

	var device_info_param = {
		deviceid: device_id,
		token: app_token
	};
	
	//console.log(JSON.stringify(device_info_param));

	mico2.getDeviceInfo(device_info_param, function(ret, err) {
		if(ret && ret.meta.code == 0) {
			//alert(JSON.stringify(ret));
			
			//再次获取设备详情 update操作
			var device_pw = ret.data.devicepw;
			var device_online = ret.data.online;
			var device_alias = ret.data.alias;
			var is_sub = ret.data.is_sub;
			var parent_id = ret.data.parentid;
			var device_type = ret.data.gatewaytype;
			

			localStorage.setItem(_DEVICE_NAME, device_alias);
			localStorage.setItem(_DEVICE_PW, device_pw);
			localStorage.setItem(_DEVICE_ID, device_id);
			localStorage.setItem(_IS_SUB_DEVICE, is_sub); //bool类型
			localStorage.setItem(_PARENT_ID, parent_id);
			
			if(device_type == 0){ //普通设备
				localStorage.setItem(_DEVICE_TYPE, "C");
			}else if(device_type == 1){//网关设备
				localStorage.setItem(_DEVICE_TYPE, "G");
			}else if(device_type == 2){//子设备
				localStorage.setItem(_DEVICE_TYPE, "S");
			}else {//错误情况
				localStorage.setItem(_DEVICE_TYPE, "NULL");
			}
	
			document.getElementById("per_device_name").innerHTML = device_alias;

			set_device_UI_offline(device_online);
			
			set_add_sub_device_UI(device_type); //device_type = 0, 1, 2
	
			if(is_listen_device == true){
				listen_device(); //开启监听设备信息 
			}
			
		} else {
			console.log("获取设备详情错误:" + JSON.stringify(ret) + JSON.stringify(err));
			get_device_info(is_listen_device);
		}
	});
}

//获取设备模板,向设备端发送请求
function get_device_template(ret, err) {
	if(ret.value == 'mqtt_connect_success') {
		if(golobal_template_json_obj == null) {
			if(get_data_transmission_switch_state() == false){//透传模式为关闭状态
				send_template_commands(); //向设备端请求数据模板
			}
		}
	}
}


//监听设备 先停止监听 再开启监听   ps:如果不先停止监听,可能会导致Android APP死掉
function listen_device(){
	mico2.stopListenDevice(function(ret, err) {
		if(ret) {
			//alert("停止监听成功：" + JSON.stringify(ret));
		} else {
			//alert("停止监听失败：" + JSON.stringify(err));
		}
		
		
	});
	
	start_listen_device();
}

//开启设备监听
function start_listen_device() {
	var client_id = null;
	var device_id = null;
	var passwd = null;
	var is_sub = null;
	var parent_id = null;

	device_online_listen = false; //处理该标志位

	device_id = localStorage.getItem(_DEVICE_ID);
	client_id = localStorage.getItem(_CLINET_ID);
	passwd = localStorage.getItem(_PASSWORD); //登录密码
	is_sub = localStorage.getItem(_IS_SUB_DEVICE); //是否为子设备 得到字符串true和false
	parent_id = localStorage.getItem(_PARENT_ID); //父设备id

	if(device_id == null || client_id == null || passwd == null) {
		back_device_list_html();
	}

	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var param = {
		host: _FOG_MQTT_HOST_NAME,
		port: _FOG_MQTT_HOST_PORT_SSL,
		username: client_id,
		password: passwd,
		deviceid: device_id,
		clientid: client_id,
		isencrypt: _FOG_MQTT_SSL_CHOOSE
	};

//	alert(JSON.stringify(param));
//	console.log(JSON.stringify(param));

	if(is_mqtt_client_connect == true){
		console.log("mqtt is already connect! is_mqtt_client_connect = " + is_mqtt_client_connect);
		return;
	}else{
		console.log("start mqtt client");
	}

	try {
		mico2.startListenDevice(param, function(ret, err) {
			if(ret) {
				console.log("ret" + JSON.stringify(ret));
				if(ret.code == 2001){
					is_mqtt_client_connect = true;
					add_device_online_listener(device_id);
				}if(ret.code == 2008){
					is_mqtt_client_connect = false;
					send_event('mqtt_client_restart', ''); //mqtt客户端重连
				}else{
					if(ret.topic != null){
						process_mqtt_data(device_id, ret); //MQTT消息
					}
				}
			} else {
				mui.toast("监听失败:" + JSON.stringify(err));
				console.log("监听失败:" + JSON.stringify(err));
				
				stop_listen_device();
			}
		});
	} catch(e) {
		alert("收到异常数据,请检查数据格式" + e.message);
	}
}

//处理mqtt数据
function process_mqtt_data(device_id, data) {
	var mqtt_topic = data.topic;
	var mqtt_payload = data.payload;
	var is_sub = localStorage.getItem(_IS_SUB_DEVICE);

//	alert("is_sub:" + is_sub);
//	console.log("topic:" + mqtt_topic + "<br/>" + "payload:" + JSON.stringify(mqtt_payload));

	if(mqtt_topic == ("d2c/" + device_id + "/status")) { //设备状态信息
		
		if(is_sub == "false") {
			if(get_data_transmission_switch_state() == true)
			{
				display_recv_data_to_recv_area(mqtt_payload);
			}else if(get_data_transmission_switch_state() == false){
				process_device_status_data_by_CMD(mqtt_payload);
			}
		} else if(is_sub == "true") {
			if(get_data_transmission_switch_state() == true)
			{
				display_recv_data_to_recv_area(mqtt_payload);
			}else if(get_data_transmission_switch_state() == false){
				//mui.toast(JSON.stringify(mqtt_payload));
				//console.log(JSON.stringify(mqtt_payload));
				process_device_status_data_by_CMD(mqtt_payload);
			}
		}else{
			alert("is_sub is error");
		}
		
	} else if(mqtt_topic == ("d2c/" + device_id + "/cmd")) { //云端推送给手机的其他信息
		//alert("收到cmd消息");
		process_device_cmd(mqtt_payload);
	}
}

//停止设备监听
function stop_listen_device() {
	mico2.stopListenDevice(function(ret, err) {
		if(ret) {
			mui.toast("取消监听成功：" + JSON.stringify(ret));
		} else {
			mui.toast("取消监听失败：" + JSON.stringify(err));
		}
		
		is_mqtt_client_connect = false;
	});
}

//监听设备是否在线
function add_device_online_listener(device_id) {
	var add_listen_param = {
		topic: "d2c/" + device_id + "/cmd",
		qos: 0
	};

	//alert("增加监听");
	mico2.addDeviceListener(add_listen_param, function(ret, err) {
		if(ret) {
			console.log("增加设备监听成功:" + JSON.stringify(ret));
		} else {
			alert("增加设备监听失败:" + JSON.stringify(err));
		}
		send_event('get_device_template', 'mqtt_connect_success'); //触发指定的监听事件
	});
}

//向设备端发起获取模板的请求
function send_template_commands() {
	mui.toast("请求数据模板");

	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var app_command_id = get_app_command_id();

	var app_command = {
		command_id: app_command_id,
		CMD: C2D_GET_TEMPLATE
	};

	//alert(JSON.stringify(app_command));
	send_message_to_device(JSON.stringify(app_command));
}

//在外设界面监听Android返回按键
function peripherals_listen_keyback() {
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		back_device_list_html();
	});
}