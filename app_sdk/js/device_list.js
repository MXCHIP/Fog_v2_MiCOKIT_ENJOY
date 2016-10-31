const SUPER_ADMIN = 1;
const ADMIN = 2;
const GENERAL = 3;

//下拉刷新配置
var refresh = {
	visible: true,
	loadingImg: 'widget://icon/wifi.svg',
	bgColor: '#efeff4',
	textColor: '#333333',
	textDown: '下拉刷新...',
	textUp: '松开刷新...',
	showTime: true
}

//关闭当前页面的所有view_cell元素
function close_view_cell() {
	var view_cell = document.getElementsByClassName("mui-table-view-cell");

	for(var i = 0; i < view_cell.length; i++) {
		mui.swipeoutClose(view_cell[i]); //关闭此类型标签
	}
}

//监听刷新列表事件
function refresh_event_listener(ret, err) {
	//alert(JSON.stringify(ret) + '   ' + JSON.stringify(err));

	if(ret.value == 'easylink' || ret.value == 'peripherals' || ret.value == 'device_add' || ret.value == 'join_device_list_page') {
		//mui.toast("刷新设备列表");
		getDeviceList();
		close_view_cell();
	}

	if(ret.value == 'device_management' || ret.value == 'device_share') {
		close_view_cell();
	}
}

//设备重命名
function device_list_rename(element) {
	var id = element.parentNode.parentNode.id;

	var arr = id.split(',');
	var device_id = arr[0]; //拿到device id

	var app_token = check_app_token(); //拿到token

	if(monitor_net() == "none") {
		return false;
	}

	//----MUI样式----

	//	var btnArray = ['取消', '确定'];
	//	mui.prompt('请输入新的设备名', '', 'MICO', btnArray, function(e) {
	//		if(e.index == 1) {
	//			var param = {
	//				deviceid: device_id,
	//				devicename: e.value,
	//				token: app_token
	//			};
	//
	//			alert(JSON.stringify(param));
	//
	//			mico2.updateDeviceAlias(param, function(ret, err) {
	//				if(ret && ret.meta.code == 0) {
	//					mui.toast("下拉刷新设备设备列表~");
	//				} else {
	//					mui.toast('重命令失败,err:' + JSON.stringify(err));
	//				}
	//			});
	//		} else {
	//			//mui.toast("取消重命名");
	//		}
	//		close_view_cell();
	//	});

	//弹出框对设备重命名
	api.prompt({
		title: "设备重命名",
		buttons: ['取消', '确定']
	}, function(ret, err) {
		if(ret && ret.buttonIndex == 2) {
			//不能为空
			if(ret.text == "") {
				alert("设备名不可为空");
				close_view_cell();
				return;
			}

			//长度最长为10
			if(ret.text.length > 10) {
				alert("设备名超过最大限制,已取消");
				close_view_cell();
				return;
			}

			var param = {
				deviceid: device_id,
				devicename: ret.text,
				token: app_token
			};

			//			alert(JSON.stringify(param));

			mico2.updateDeviceAlias(param, function(ret, err) {
				if(ret && ret.meta.code == 0) {
					getDeviceList();
				} else {
					mui.toast('重命令失败,err:' + JSON.stringify(err));
				}
			});
		} else {
			//mui.toast("取消重命名");
		}
		close_view_cell();
	});
}

//分享设备
function device_list_share(element) {
	var id = element.parentNode.parentNode.id;
	var param = {
		info: id
	};

	if(monitor_net() == "none") {
		return false;
	}

	openWin('device_share', 'widget://html/device_share.html', true, param);
}

//设备授权管理
function device_list_management(element) {
	var id = element.parentNode.parentNode.id;
	var arr = id.split(',');
	var param = {
		info: arr[0]
	};

	if(monitor_net() == "none") {
		return false;
	}

	openWin('device_share', 'widget://html/device_management.html', true, param);
}

//删除设备
function device_list_delete(element) {
	var id = element.parentNode.parentNode.id;
	var arr = id.split(',');

	device_id = arr[0];
	device_pw = arr[1];
	device_name = arr[2];

	//	mui.toast("device_id:" + device_id + ",device_pw:" + device_pw + ",devive_name:" + device_name);

	if(monitor_net() == "none") {
		return false;
	}

	var app_token = check_app_token();

	var param = {
		deviceid: device_id,
		token: app_token
	};

	mico2.unBindDevice(param, function(ret, err) {
		if(ret) {
			if(ret.meta.code == 0 || ret.meta.code == 23100) {
				mui.toast("删除成功");
				//element.parentNode.parentNode.parentNode.removeChild(element.parentNode.parentNode);
				getDeviceList();
			}
		} else {
			mui.toast("删除失败" + JSON.stringify(ret) + "   " + JSON.stringify(err));
		}

		close_view_cell();
	});

}

//打开设备信息页面
function openDeviceInfo(element) {
	var id = element.parentNode.id;
	var arr = id.split(',');

	device_id = arr[0];
	device_pw = arr[1];
	device_name = arr[2];

	//mui.toast("device_id:" + device_id + ",device_pw:" + device_pw + ",devive_name:" + device_name);

	localStorage.setItem(_DEVICE_ID, device_id);
	localStorage.setItem(_DEVICE_NAME, device_name);
	localStorage.setItem(_DEVICE_PW, device_pw);
	openWin("peripherals", "widget://html/peripherals.html");
}

function notice_offline() {
	mui.toast("设备离线<br/>请检查设备状态或刷新设备列表");
}

//获取设备列表
function getDeviceList() {
	if(monitor_net() == "none") {
		return false;
	}

	var app_token = check_app_token();

	var param = {
		token: app_token
	}

	mico2.getDeviceList(param, function(ret, err) {
		if(ret && ret.meta.code == 0) {
			data = ret.data;

			var html_delete_share = "";
			var html_text = "";
			var html_online = "";
			var html_device_type = "";
			var html_total = "";
			var device_list_html = "";

			for(var index in data) {
				var device_name = data[index].device_name;
				var device_id = data[index].device_id;
				var device_pw = data[index].device_pw;
				var device_role = data[index].role;
				var device_online = data[index].online;
				var device_mac = data[index].mac;
				var device_type = data[index].gatewaytype;

				var div_id = device_id + ',' + device_pw + ',' + device_name;

				//alert(JSON.stringify(data[index]));

				//1.判断是否在线
				if(device_online == true) {
					html_online = '<span class="mui-badge mui-badge-green" style="vertical-align: middle; margin-left: 5px;">在线</span>';
				} else {
					html_online = '<span class="mui-badge mui-badge-danger" style="vertical-align: middle; margin-left: 5px;">离线</span>';
				}
				
				
				if(device_type == 0) { 
					//普通设备
					html_device_type = '<span class="mui-badge mui-badge-warning" style="vertical-align: middle; margin-left: 5px;"></span>';
					html_device_type = '';
				} else if(device_type == 1) {
					//网关设备
					html_device_type = '<span class="mui-badge mui-badge-warning" style="vertical-align: middle; margin-left: 5px;">网关</span>';
				} else if(device_type == 2) { 
					//子设备
					html_device_type = '<span class="mui-badge mui-badge-warning" style="vertical-align: middle; margin-left: 5px;">子设备</span>';
				} else { 
					//错误情况
					html_device_type = '';
				}
				
				//2.组合删除和分享按钮div
				if(device_role == SUPER_ADMIN) { //超级管理员:重命名+分享+删除
					html_delete_share = '<div class="mui-slider-right mui-disabled"><a class="mui-btn mui-btn-blue mui-icon device_list_share_romove icon-rename" onclick="device_list_rename(this)"></a><a class="mui-btn mui-btn-green mui-icon device_list_share_romove icon-share2" onclick="device_list_share(this)"></a><a class="mui-btn mui-btn-yellow mui-icon device_list_share_romove icon-shouquan2" onclick="device_list_management(this)"></a><a class="mui-btn mui-btn-red mui-icon device_list_share_romove icon-delete" onclick="device_list_delete(this)"></a></div>';
				} else { //普通用户:删除
					html_delete_share = '<div class="mui-slider-right mui-disabled"><a class="mui-btn mui-btn-blue mui-icon device_list_share_romove icon-rename" onclick="device_list_rename(this)"></a><a class="mui-btn mui-btn-red mui-icon device_list_share_romove icon-delete" onclick="device_list_delete(this)"></a></div>';
				}
								
				//3.组合图片和文本区域
				if(device_mac != undefined) {
					if(device_online == true) {
						html_text = '<div class="mui-slider-handle" onclick="openDeviceInfo(this)"><div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div><div class="device_list_text"><div><span style="vertical-align: middle;">' + device_name + '</span>' + html_online + html_device_type + '</div><p><h6>' + 'MAC: ' + device_mac.toUpperCase() + '</h6></p></div></div>';
					} else {
						html_text = '<div class="mui-slider-handle" onclick="openDeviceInfo(this)"><div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div><div class="device_list_text"><div><span style="vertical-align: middle;">' + device_name + '</span>' + html_online + html_device_type + '</div><p><h6>' + 'MAC: ' + device_mac.toUpperCase() + '</h6></p></div></div>';
						//html_text = '<div class="mui-slider-handle" onclick="notice_offline()"><div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div><div class="device_list_text"><div><span style="vertical-align: middle;">' + device_name + '</span>' + html_online + html_device_type + '</div><p><h6>' + 'MAC: ' + device_mac.toUpperCase() + '</h6></p></div></div>';
					}
				} else {
					if(device_online == true) {
						html_text = '<div class="mui-slider-handle" onclick="openDeviceInfo(this)"><div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div><div class="device_list_text"><div><span style="vertical-align: middle;">' + device_name + '</span>' + html_online + html_device_type + '</div><p><h6>' + device_id.substr(0, 23) + '</h6></p></div></div>';
					} else {
						//html_text = '<div class="mui-slider-handle" onclick="openDeviceInfo(this)"><div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div><div class="device_list_text"><div><span style="vertical-align: middle;">' + device_name + '</span>' + html_online + html_device_type + '</div><p><h6>' + device_id.substr(0,23) + '</h6></p></div></div>';
						html_text = '<div class="mui-slider-handle" onclick="notice_offline()"><div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div><div class="device_list_text"><div><span style="vertical-align: middle;">' + device_name + '</span>' + html_online + html_device_type + '</div><p><h6>' + device_id.substr(0, 23) + '</h6></p></div></div>';
					}
				}
				
				//4.组合整体
				if(device_online == true) {
					html_total = '<li id=' + '"' + div_id + '"' + 'class="mui-table-view-cell">' + html_delete_share + html_text + '</li>';
				} else {
					html_total = '<li id=' + '"' + div_id + '"' + 'class="mui-table-view-cell" style="background: #DFDFDF;">' + html_delete_share + html_text + '</li>';
				}

				device_list_html += html_total;
			}

			//alert(data.length + "    "+ device_list_html);

			var deviceList = document.getElementById("deviceList");

			if(device_list_html == "") {
				deviceList.style.display = "none";
			} else {
				deviceList.style.display = "block";
			}
			deviceList.innerHTML = device_list_html;

		} else {
			mui.toast("获取设备列表失败 " + JSON.stringify(err) + JSON.stringify(ret));
		}

		console.log(JSON.stringify(ret));

		close_view_cell();
	});
}