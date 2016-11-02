

//监听开关信息
function listen_switch() {
	var user_switch = document.getElementsByClassName('mico_peripherals_switch');

	for(var i = 0; i < user_switch.length; i++) {
		//alert('i = ' + i + ',id = ' + user_switch[i].id);

		user_switch[i].addEventListener('toggle', function(event) {
			//alert(this.id); //可以正常拿到id
			//发送消息给设备
			var app_command_id = get_app_command_id();
			var data_array = new Array();
			
			var arr = this.id.split('_');
			
			var pid = arr[0];
			var type = arr[1];
			
			data_array[0] = {
				PID: pid,
				T: type,
				D: event.detail.isActive
			}

			var app_command = {
				command_id: app_command_id,
				CMD: C2D_SEND_COMMANDS,
				G: data_array
			}

			//alert(JSON.stringify(app_command));
			send_message_to_device(JSON.stringify(app_command));
		});
	}
}

//解析设备端上传的数据 根据CMD来分类处理
function process_device_status_data_by_CMD(device_status) {
	try {
		if(device_status.CMD == D2C_ACK_TEMPLATE) {
			//如果有模板,并且相同,就不加载
			
			if(golobal_template_json_obj == null){
				get_peripherals_template(device_status);
			}else{
				mui.toast("已有模板");
			}
		} else if(device_status.CMD == D2C_SEND_PACKETS) {
			if(golobal_template_json_obj != null) {
				change_list_value(device_status);
			}
		}
	} catch(e) {
		alert("数据包错误" + e.message);
	}
}

//在模板中根据pid查找type
function get_type_by_pid(pid) {
	try {
		var i = 0;

		var template_G = golobal_template_json_obj.G;

		for(i = 0; i < template_G.length; i++) {
			if(template_G[i].PID == pid) {
				return template_G[i].T;
			}
		}

		return false;
	} catch(e) {
		alert("PID解析错误" + e.message);
		return false;
	}
}

//改变只读型和开关型外设的UI显示
function change_list_value(device_status) {
	try {
		//alert(JSON.stringify(device_status));

		var data_group = device_status.G;
		var i = 0;

		for(i = 0; i < data_group.length; i++) {
			var pid = data_group[i].PID;
			var data = data_group[i].D;
			var type = data_group[i].T;
			var li_pid = pid + '_' + type; 
			var template_type = get_type_by_pid(pid);
			
			if(type	!= template_type){
				mui.toast("收到的PID&T和模板中的PID&T不匹配" + "收到PID&T:" + PID + ' ' +type + "模板PID&T" + PID + ' '+ template_type);
				continue;
			}
				
			if(document.getElementById(li_pid) == null){
				mui.toast("未找到相应PID, PID = " + pid);
				continue;
			}
			
			if(type >= _TYPE_READ_MIN && type <= _TYPE_READ_MAX) {
				if(typeof(data) == "boolean" || typeof(data) == "object" || typeof(data) == "undefined" || typeof(data) == "function"){
					mui.toast("数据格式错误, li_pid" + li_pid);
					continue;
				}
				
				document.getElementById(li_pid).innerHTML = data;
			} else if(type >= _TYPE_BOOL_CTRL_MIN && type <= _TYPE_BOOL_CTRL_MAX) {
				
				if(typeof(data) != "boolean"){
					mui.toast("数据格式错误, li_pid" + li_pid);
					continue;
				}
				
				//alert(li_pid);
				
				//先获得当前状态
				var isActive = document.getElementById(li_pid).classList.contains("mui-active");

				//alert("li_pid:" + li_pid +",isActive:" + isActive);

				//如果值不同,则跳变一次
				if(isActive != data) {
					mui("#" + li_pid).switch().toggle(); //改变开关状态
				} else {
					//do nothing
				}
			}
		}

	} catch(e) {
		alert("数据异常: " + e.message);
	}

}

//返回0-999999之间的随机数
function get_app_command_id() {
	var Range = 999999;
	var Rand = Math.random();

	return(Math.round(Rand * Range));
}

//发送数据到设备端
function send_message_to_device(app_command) {
	var device_id = localStorage.getItem(_DEVICE_ID);
	var device_pw = localStorage.getItem(_DEVICE_PW);
	var is_sub = localStorage.getItem(_IS_SUB_DEVICE);

	var app_token = check_app_token();

	if(monitor_net() == "none") {
		return false;
	}

	if(device_id == null || device_pw == null || is_sub == null || is_sub == "undefined") {
		mui.toast("读取设备信息错误");
		back_device_list_html();
	}

	
	if(is_sub == "false") {
		var param = {
			deviceid: device_id,
			devicepw: device_pw,
			command: app_command,
			token: app_token
		};

		//alert(JSON.stringify(param));

		mico2.sendCommand(param, function(ret, err) {
			try {
				if(ret && ret.meta.code == 0) {
					//mui.toast("发送数据成功");
				} else {
					alert("发送数据失败[err]" + JSON.stringify(err) + JSON.stringify(ret));
				}
			} catch(e) {
				//alert("发送数据失败[ret]:" + JSON.stringify(ret));
			}

		});
	} else if(is_sub == "true") {
//		var param = {
//			subdeviceid: device_id,
//			devicepw: device_pw,
//			flag: 3,
//			payload: app_command,
//			format: "json"
//		};
//		
//		console.log(JSON.stringify(param));
//		
//		user_http_post("/enduser/sendCommandSub/", param, function(ret, err) {
//			try {
//				if(ret && ret.meta.code == 0) {
//					mui.toast("发送数据成功");
//				} else {
//					alert("发送数据失败[err]" + JSON.stringify(err) + JSON.stringify(ret));
//				}
//			} catch(e) {
//				alert("发送数据失败[ret]:" + JSON.stringify(ret));
//			}
//		});

		var param = {
			subdeviceid: device_id,
			devicepw: device_pw,
			flag: 3,
			command: app_command,
			format: "json",
			token:app_token
		};
		
		//console.log(JSON.stringify(param));
		
		mico2.sendCommandSub(param, function(ret, err) {
			try {
				if(ret && ret.meta.code == 0) {
					//mui.toast("发送数据成功");
				} else {
					alert("发送数据失败[err]" + JSON.stringify(err) + JSON.stringify(ret));
				}
			} catch(e) {
				alert("发送数据失败[ret]:" + JSON.stringify(ret));
			}
		});

	
	}else{
		alert("is_sub error!" + is_sub);
	}
}