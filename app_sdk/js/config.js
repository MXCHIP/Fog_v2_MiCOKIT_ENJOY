const _ID = "A6924362872707"; //APICLOUD专用的 工程id,用户关闭当前app的时候使用
const _APPID = "a457250e-8969-11e6-9d95-00163e103941"; //fogcloud的新建app得到的appid

const _TOKEN = "token"; //用户登录token对应的key
const _CLINET_ID = "client_id"; //用户登录token对应的clinetid
const _USERNAME = "username";
const _PASSWORD = "password";

const _DEVICE_ID = "device_id"; //记录点击的设备的设备id
const _DEVICE_NAME = "device_name"; //记录点击的设备的设备name
const _DEVICE_PW = "device_pw" //记录点击的设备的设备密码
const _IS_SUB_DEVICE = "is_sub"; //是否是子设备
const _PARENT_ID = "parent_id";  //父设备id 如果为子设备,则存在父设备
const _DEVICE_TYPE = "device_type";  //设备类型  'S':子设备   'G':gateway  'C':普通设备

const _FOG_HTTPS_HOST_NAME = "https://v2.fogcloud.io"; //fog HTTP服务器域名
const _FOG_MQTT_HOST_NAME = "mqtt.fogcloud.io"; //fog MQTT服务器域名
const _FOG_MQTT_HOST_PORT = "1883"; //MQTT端口号
const _FOG_MQTT_HOST_PORT_SSL = "8443"; //MQTT端口号 (SSL) IOS库内部只能用带SSL的连接
const _FOG_MQTT_SSL_CHOOSE = true;

const _FOG_PUBLISH_DEVICE_ONLINE_CODE = 2001; 		//父设备在线
const _FOG_PUBLISH_DEVICE_OFFLINE_CODE = 2002;		//父设备离线
const _FOG_PUBLISH_SUB_DEVICE_ONLINE_CODE = 2004; 	//子设备在线通知code
const _FOG_PUBLISH_SUB_DEVICE_OFFLINE_CODE = 2005; 	//子设备离线线通知code

const _FOG_PUBLISH_DEVICE_RECOVERY_CODE = 2003; 	//父设备解除授权
const _FOG_PUSBLISH_SUB_DEVICE_UNREGISTER_CODE = 2007; 	//子设备注销

const _FOG_PUSBLISH_SUB_DEVICE_REGISTER_CODE = 2006;    //子设备注册
const _FOG_PUSBLISH_SUB_DEVICE_ADD_TIMEOUT_CODE = 2000;	//添加子设备超时

const FILTER_DEVICE_PROTOCOL = "fog"; //fog2.0.1

var mico2 = null; //fog模块对象初始值

//toast消息提醒
function msg(content, times) {
	api.toast({
		msg: content,
		duration: times,
		location: 'bottom'
	});
}

//设置手机状态栏字体颜色//light:白色；dark:黑色
function setStatusBar(style) {
	api.setStatusBarStyle({
		style: style
	});
}

//设置页面顶部标题
var title = ["设备列表", "个人中心"]

//设置子页面数组
var frames = [{
	name: 'device_list',
	url: 'widget://html/device_list.html'
}, {
	name: 'user',
	url: 'widget://html/personal.html'
}];

//创建子页面组，并初始化打开指定子页面
function openFrameGroup() {
	api.openFrameGroup({
		name: 'group',
		background: '#efeff4',
		scrollEnabled: false,
		rect: {
			x: 0,
			y: 65,
			w: 'auto',
			h: 'auto',
			marginBottom: 50
		},
		index: 0,
		frames: frames
	}, function(ret, err) {
		//....
	});
}

//关闭子页面组
function closeFrameGroup(name) {
	api.closeFrameGroup({
		name: name
	});
}

//设置子页面组要显示的子页面
function setFrameGroupIndex(index) {
	api.setFrameGroupIndex({
		name: 'group',
		index: index,
		scroll: false,
		reload: false
	});
}

//打开新页面/窗口
function openWin(name, url, isReload, args) {
	api.openWin({
		name: name, //要打开的新页面去掉后面的.html
		url: url, //子页面路径
		reload: isReload, //打开新页面是否重新加载/刷新页面
		slidBackEnabled: false, //只对IOS有效//禁止滑动关闭
		delay: 0,
		bounces: false,
		vScrollBarEnabled: false,
		hScrollBarEnabled: false,
		bgColor: 'white',
		pageParam: args,
	});
}

//关闭页面/窗口
function closeWin(name) {
	api.closeWin({
		name: name
	});
}

//监听底部菜单点击事件，执行子页面切换
function switchBar() {
	//监听底部菜单点击事件，执行子页面切换
	mui(".mui-bar-tab").on('tap', 'a', function() {
		//获取被点击节点的父节点
		var parent = this.parentNode;
		//获取所有兄弟节点
		var brothers = parent.children;
		//循环判断被点击节点的位置，决定切换的子页面，index为兄弟节点的数组中对应的索引，对应子节点id
		for(var index in brothers) {
			if(brothers[index] == this) {
				document.getElementById('title').innerText = title[index];
				//调用页面切换方法
				setFrameGroupIndex(index);

				//显示添加按钮
				var add_device = document.getElementById('add_device');
				var QR_code = document.getElementById('QR_code');
				if(index == 0) {
					add_device.style.display = "block";
					QR_code.style.display = "block";

					send_event('refesh_device_list', 'join_device_list_page');
				} else {
					add_device.style.display = "none";
					QR_code.style.display = "none";
				}
			}
		}
	});
}

function refesh_token() {
	app_token = check_app_token();

	if(monitor_net() == "none") {
		alert("网络异常！");
		
		openWin('login', 'widget://html/login.html', true);
		return false;
	}

	var param = {
		token: app_token
	};

	mico2.refreshToken(param, function(ret, err) {
		if(ret && ret.token) {
			mui.toast("刷新token成功");
			localStorage.setItem(_TOKEN, ret.token);
			localStorage.setItem(_CLINET_ID, ret.clientid);
			return true;
		} else {
			mui.toast("刷新token失败");
			openWin('login', 'widget://html/login.html', true);
			return false;
		}
	});
}

//判断用户是否已登录
function checkToken() {
	var token = localStorage.getItem(_TOKEN);
	if(token != null) {
		openWin('index', 'widget://html/index.html', true);
	}
}

//关闭APP
function closeWidget() {
	api.closeWidget({
		id: _ID,
		silent: true
	});
}

//监听安卓手机返回键
function ListenKeyBack() {
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		var url = location.href;
		var name = url.substr(url.length - 10, url.length);
		if(name == "login.html" || name == "index.html") {
			msg("再按一次退出应用");

			setTimeout(function() {
				ListenKeyBack();
			}, 4000); 

			api.addEventListener({
				name: 'keyback'
			}, function(ret, err) {
				closeWidget();
			});
		}
	});

}

//监控网络变化
function monitor_net() {
	var connectionType = api.connectionType;
	if(connectionType == "none") {
		mui.toast("网络不给力,请检查网络连接~");
	}
	return connectionType;
}

//检查app token
function check_app_token() {
	var app_token = localStorage.getItem(_TOKEN);
	if(app_token == null) {
		openWin('login', 'widget://html/login.html', true);
		return false;
	}

	return app_token;
}

//发送事件
function send_event(name, extra) {
	api.sendEvent({
		name: name,
		extra: extra //扩展信息,JSON格式
	});
}

//监听事件
function add_event_listener(name, fun) {
	//html页面a
	api.addEventListener({
		name: name
	}, function(ret, err) {
		fun(ret, err);
	});
}

//ajax的http get
function user_http_get(URI, callback) {
    var param = {
        url: _FOG_HTTPS_HOST_NAME + URI,
        method: "get",
        cache:false,
        headers: {
            "Content-Type": "application/json",
            "Authorization": "JWT " + localStorage.getItem(_TOKEN)
        }
    };
    
    //console.log(JSON.stringify(param));
    
    api.ajax(param, function (ret, err) {
        console.log("ajax 返回值, ret:" + JSON.stringify(ret) + " err:" + JSON.stringify(err));
        if (callback) {
            callback(ret, err);
        }
    });
}

//ajax的http post
function user_http_post(URI, data, callback) {
	var param = {
		url: _FOG_HTTPS_HOST_NAME + URI,
		method: "post",
		dataType: "json",
		cache:false,
		data: {
			body: data
		},
		headers: {
			"Content-Type": "application/json",
			"Authorization": "JWT " + localStorage.getItem(_TOKEN)
		}
	};
	
	//console.log(JSON.stringify(param));
	
    api.ajax(param, function (ret, err) {
        if (callback) {
            callback(ret, err)
        }
    });
}
