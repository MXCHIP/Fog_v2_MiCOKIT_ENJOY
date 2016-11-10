var get_ssid_timer_id = null;
var golobal_wifi_ssid = null;
var easylink_button_timer_id = null;
var mdns_list_timer_id = null;
var is_easylink_flag = false;

//退出easylink页面
function quit_easylink_page() {
	//关闭定时器
	if(easylink_button_timer_id != null) {
		clearTimeout(easylink_button_timer_id);
	}

	if(mdns_list_timer_id != null) {
		clearTimeout(mdns_list_timer_id);
	}

	//关闭wifi监控定时器
	stop_wifi_ssid_monitor();

	//关闭easylink
	stopEasylink();

	//关闭设备发现
	stopSearchDevice();

	send_event('refesh_device_list', 'easylink');

	//关闭当前窗口,进入到新窗口
	closeWin('easylink');
}

//得到wifi的ssid
function get_wifi_ssid() {
	mico2.getSSID(function(ret, err) {
		if(golobal_wifi_ssid != ret.ssid) //检测到wifi名称变化则把密码清空、关闭配网
		{
			stopEasylink();
			document.getElementById("easylink_wifi_key").value = "";
		}

		if(ret.ssid != '') {
			if(document.getElementById("easylink_wifi_name").value != ret.ssid) {
				startSearchDevices();
			}
		} else {
			//mui.toast("wifi已关闭！");
			stopSearchDevice();

			document.getElementById('search_devices').innerHTML = '';
			document.getElementById('search_devices').style.display = 'none';
		}

		document.getElementById("easylink_wifi_name").value = ret.ssid;

		//mui.toast(ret.ssid);
		golobal_wifi_ssid = ret.ssid;
	});
}

function stop_wifi_ssid_monitor() {
	//关闭定时器
	if(get_ssid_timer_id != null) {
		clearTimeout(get_ssid_timer_id);
	}
}

function wifi_ssid_monitor() {
	setTimeout(function() {
		get_wifi_ssid();
		get_ssid_timer_id = setTimeout("wifi_ssid_monitor()", 1500); //1s检测一次wifi的ssid
	}, 0);
}

//设置配网按钮属性
function set_easylink_button_attr(easylink_is_open, time_value) {

	if(easylink_is_open == true) {
		is_easylink_flag = false;

		document.getElementById('button_easylink').innerHTML = "开启配网";
		document.getElementById("button_easylink").style.background = "#007AFF";
		document.getElementById("button_easylink").style.border = "none";
		document.getElementById("button_easylink").setAttribute("onclick", "startEasyLink()");

		document.getElementById("easylink_wifi_key").disabled = false; //禁止输入密码
	} else {
		is_easylink_flag = true;

		document.getElementById('button_easylink').innerHTML = "关闭配网 (" + time_value + "s)";
		document.getElementById("button_easylink").style.background = "#DD524D";
		document.getElementById("button_easylink").style.border = "none";
		document.getElementById("button_easylink").setAttribute("onclick", "stopEasylink()");

		document.getElementById("easylink_wifi_key").disabled = true; //允许输入密码

		time_value = time_value - 1;

		if(time_value > 0) {
			easylink_button_timer_id = setTimeout(function() { //每1000毫秒重新调用此方法
				set_easylink_button_attr(false, time_value);
			}, 1000);

			//mui.toast(easylink_button_timer_id);
		} else {
			set_easylink_button_attr(true, 0);

			if(document.getElementById('search_devices').innerHTML == "") {
				mui.toast("请长按设备easylink按钮\n确保设备处于配网状态");
			}
		}

	}
}

//停止配网
function stopEasylink() {
	//关闭定时器
	if(easylink_button_timer_id != null) {
		clearTimeout(easylink_button_timer_id);
	}

	mico2.stopEasyLink(function(ret, err) {
		if(ret) {
			//mui.toast(JSON.stringify(ret));
		} else {
			//mui.toast(JSON.stringify(err));
		}
	});

	set_easylink_button_attr(true, 0); //设置按钮属性
	//mui.toast("配网已关闭");
}

function start_easylink_step() {
	//1.关闭定时器
	if(easylink_button_timer_id != null) {
		clearTimeout(easylink_button_timer_id);
	}

	var param = {
		ssid: document.getElementById('easylink_wifi_name').value,
		password: document.getElementById('easylink_wifi_key').value,
		runSecond: 600000 //600秒后自动停止发包,实际开了定时器,最多60s就可以停止
	};

	if(monitor_net() != "wifi") {
		alert("手机未连接Wi-Fi,请进入系统Wi-Fi设置页面选取合适的网络！");
		return false;
	}

	//2.关闭配网
	mico2.stopEasyLink(function(ret, err) {
		if(ret) {
			//mui.toast(JSON.stringify(ret));
		} else {
			//mui.toast(JSON.stringify(err));
		}
	});

	if(monitor_net() != "wifi") {
		alert("手机未连接Wi-Fi,请进入系统Wi-Fi设置页面选取合适的网络！");
		return false;
	}

	//3.开启easylink
	mico2.startEasyLink(param, function(ret, err) {
		if(ret) {
			//mui.toast(JSON.stringify(ret));
			//开启60s定时器,并实现时间提示
			set_easylink_button_attr(false, 60); //配网按钮设置

		} else {
			mui.toast(JSON.stringify(err));
		}
	});
}

//开启配网
function startEasyLink() {
	if(golobal_wifi_ssid == "") {
		alert("手机未连接Wi-Fi,请进入系统Wi-Fi设置页面选取合适的网络！");
		return false;
	}

	//	mui.confirm("硬件设备指示灯是否闪烁?", "配网提示", ["是", "否"], function(ret) {
	//		if(ret.index == 0) {
	//			start_easylink_step();
	//		}
	//	});

	start_easylink_step();
}

//开启搜索设备
function startSearchDevices() {
	var div_search_devices = document.getElementById('search_devices');
	var li_total_is_bind = "";
	var li_total_not_bind = "";
	var filter_device_protocol = "";
	var filter_MAC = "";

//	//1. 先停止搜索设备
//	mico2.stopSearchDevices(function(ret, err) {
//		if(ret) {
//			//mui.toast(JSON.stringify(ret));
//		} else {
//			//mui.toast(JSON.stringify(err));
//		}
//	});

	//2.开启搜索设备
	mui.toast("正在搜索设备");
	mico2.startSearchDevices(function(ret, err) {
		li_total_is_bind = "";
		li_total_not_bind = "";

		//alert(JSON.stringify(ret));

		//alert(getNowFormatDate());

		if(ret.devices != null) {
			var devices = ret.devices;
			//mui.toast("总长度:"+ devices.length);

			if(typeof(devices) == "undefined") {
				mui.toast("[0]ret.device is undefined");
				return;
			}

			//alert(JSON.stringify(devices));

			for(var filter_index in devices) {
				filter_device_protocol = devices[filter_index].Protocol;
				filter_MAC = devices[filter_index].MAC;

				if(typeof(filter_device_protocol) == 'undefined' || typeof(filter_MAC) == 'undefined') {
					devices.splice(filter_index, 1); //删除不符合规则的数据
					continue;
				}

				if((filter_device_protocol.substr(0, 3) != FILTER_DEVICE_PROTOCOL)) {
					devices.splice(filter_index, 1); //删除不符合规则的数据
					continue;
				}

				if(devices[filter_index].hasOwnProperty("MAC") == false) {
					devices.splice(filter_index, 1); //删除不符合规则的数据
					continue;
				}

				if((filter_MAC == "")) {
					mui.toast("检查到MAC为空的非法设备");
					continue;
				}
			}

			if(devices.length == 0) {
				mui.toast("[1]刷新mdns列表");
				//alert(JSON.stringify(ret)); //库里面偶尔返回的devices为空。。。
				div_search_devices.innerHTML = "";
				div_search_devices.style.display = "block";
				return;
			}

			//按照MAC地址对数组成员做排序处理
			devices.sort(
				function(a, b) {    
					return (a.MAC.localeCompare(b.MAC));
				}
			);

			//mui.toast(JSON.stringify(devices));

			//遍历后合成HTML显示
			for(var index in devices) {

				filter_device_protocol = devices[index].Protocol;

				if(typeof(filter_device_protocol) == 'undefined' || filter_device_protocol == null) {
					continue;
				}
				
				if((filter_device_protocol.substr(0, 3) != FILTER_DEVICE_PROTOCOL)) {
					//alert("协议错误");
					continue;
				}

				var device_name = devices[index].Name;
				var device_mac = devices[index].MAC;
				var device_ip = devices[index].IP;
				var device_port = devices[index].Port;
				var ip_prot = device_ip + "," + device_port;
				var device_super = devices[index].IsHaveSuperUser;

				if(device_name == null || device_mac == null || device_ip == null || device_port == null || device_super == null) {
					mui.toast("检测到不符合协议的设备");
					continue;
				}

				var list_div1 = '<div style="float: left; color: #007AFF"><span class="mui-icon device_search icon-device2"></span></div>';

				var special_characters_index = device_name.indexOf('#');
				if(special_characters_index != -1) {
					device_name = device_name.substr(0, special_characters_index);
				}

				var list_div2 = '<div class="mui-media-body" style="float: left; margin-left: 20px;">' + device_name + '<p class="mui-ellipsis">MAC: ' + device_mac + '</p></div>';

				if(device_super == "true") {
					var list_div3 = '<div style="float: right; margin-right: 10px; margin-top: 8px;"><span class="mui-badge mui-badge-danger">已绑定</span></div>';
					var list_div = '<li style="background: #DFDFDF;" class="mui-table-view-cell mui-media" id="' + ip_prot + '">' + list_div1 + list_div2 + list_div3 + '</li>';
					li_total_is_bind += list_div;
				} else if(device_super == "false") {
					var list_div3 = '<div style="float: right; margin-right: 10px; margin-top: 8px;"><span class="mui-badge mui-badge-yellow">待绑定</span></div>';
					var list_div = '<li onclick="bindDevice(this.id)" class="mui-table-view-cell mui-media" id="' + ip_prot + '">' + list_div1 + list_div2 + list_div3 + '</li>';
					li_total_not_bind += list_div;
				} else if(device_super == "UNCHECK") {
					var list_div3 = '<div style="float: right; margin-right: 10px; margin-top: 8px;"><span class="mui-badge mui-badge-green">激活中</span></div>';
					var list_div = '<li style="background: #AEEAA9;" class="mui-table-view-cell mui-media" id="' + ip_prot + '">' + list_div1 + list_div2 + list_div3 + '</li>';
					li_total_not_bind += list_div;

					//mui.toast("发现新设备,自动停止配网");

					if(is_easylink_flag == true) {
						stopEasylink(); //若发现新设备,则自动停止配网
					}
				} else {
					alert("绑定标志位错误");
				}
			}

			mui.toast("[2]刷新mdns列表");
			div_search_devices.innerHTML = li_total_not_bind + li_total_is_bind;
			div_search_devices.style.display = "block";

			clearTimeout(mdns_list_timer_id); //停止上次的定时器
			if(devices.length == 1) {
				mdns_list_timer_id = setTimeout("clean_mdns_list()", 7000); //设置7s之后清空列表
			}

			return;
		}
	});

}

//停止发现设备
function stopSearchDevice() {
	mico2.stopSearchDevices(function(ret, err) {
		if(ret) {
			//mui.toast(JSON.stringify(ret));
		} else {
			//mui.toast(JSON.stringify(err));
		}
	});
}

//绑定设备
function bindDevice(ip_port) {
	var arr = ip_port.split(",");
	var device_ip = arr[0];
	var device_port = arr[1];

	if(device_ip == 'undefined' || device_port == 'undefined') {
		mui.toast("IP或者port为空,绑定失败");
		return;
	}

	var app_token = check_app_token();

	var param = {
		ip: device_ip,
		port: device_port,
		token: localStorage.getItem(_TOKEN)
	};

	//alert(JSON.stringify(param));

	if(monitor_net() != "wifi") {
		return false;
	}

	//关闭wifi名称监控 原因:导致绑定回调函数进不去
	stop_wifi_ssid_monitor();

	mui.confirm("确定要绑定该设备？", "绑定提示", ["取消", "确定"], function(ret) {
		if(ret.index == 1) {
			mico2.bindDevice(param, function(ret, err) {
				if(ret && ret.meta.code == 0) {
					//mui.toast("绑定成功");
					quit_easylink_page();
				} else {
					alert("未成功,请重试");
					//alert("[绑定失败]APPID:" + _APPID + "  client id:" + localStorage.getItem(_CLINET_ID));
					//alert("绑定失败" + JSON.stringify(ret) + JSON.stringify(err));
					wifi_ssid_monitor(); //打开wifi ssid监控
				}
			});
		} else {
			wifi_ssid_monitor(); //打开wifi ssid监控
		}
	});
}

//进入wifi的设置页面
function goto_sys_wifi_setting() {
	if(api.systemType === 'ios') {
		var param = {
			iosUrl: 'prefs:root=WIFI' //ios10会打开失败!!!
		};
	} else {
		var param = {
			androidPkg: 'android.settings.WIFI_SETTINGS'
		};
	}

	//alert(JSON.stringify(param));

	//安卓的api.openApp正确调用是有返回的
	api.openApp(param, function(ret, err) {
		if(ret) {
			//alert("ret:" + JSON.stringify(ret));
		} else {
			//alert("err:" + JSON.stringify(err));
		}
	});
}

//清空已发现的设备列表,ios设备在mdns列表只有一个的情况下,如果设备断电,库里面有bug,不会推送一次空的列表到应用层
function clean_mdns_list() {
	document.getElementById('search_devices').innerHTML = "";
	document.getElementById('search_devices').style.display = "block";
	mui.toast("clean mdns list");
}

//获取当前的日期时间 格式“yyyy-MM-dd HH:MM:SS”
function getNowFormatDate() {
	var date = new Date();
	var seperator1 = "-";
	var seperator2 = ":";
	var month = date.getMonth() + 1;
	var strDate = date.getDate();
	if(month >= 1 && month <= 9) {
		month = "0" + month;
	}
	if(strDate >= 0 && strDate <= 9) {
		strDate = "0" + strDate;
	}
	var currentdate = date.getFullYear() + seperator1 + month + seperator1 + strDate +
		" " + date.getHours() + seperator2 + date.getMinutes() +
		seperator2 + date.getSeconds();
	return currentdate;
}

//easylink界面监听keyback按键
function easylink_page_listen_keyback() {
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		quit_easylink_page();
	});
}