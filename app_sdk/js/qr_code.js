//二维码组件
var FNScanner = null;

//打开二维码扫描
function openScanner() {
	send_event('refesh_device_list', 'device_add');

	//网络检查
	if(monitor_net() == "none") {
		return false;
	}

	FNScanner.openScanner({
		sound: "widget://notify/notify.wav",
		autorotation: false,
	}, function(ret, err) {
		if(ret && ret.eventType == "success") {
			try {
				var device_info = JSON.parse(ret.content); //todo... 首先需要判断字符串是否为json格式！！！
			} catch(e) {
				alert("图片错误");
				return;
			}

			try {
				if(device_info.appid == null || device_info.bind_vercode == null || device_info.device_id == null || device_info.device_pw == null || device_info.role != 3) {
					alert("图片信息错误,请重试");
					return;
				}
			} catch(e) {
				alert("图片错误");
				return;
			}
			
			if(device_info.appid != _APPID){
				alert("APP属于不同APPID,添加失败");
				return;
			}
			
			var app_token = check_app_token();

			//向云端发起绑定请求
			var param = {
				deviceid: device_info.device_id,
				devicepw: device_info.device_pw,
				bindvercode: device_info.bind_vercode,
				role: device_info.role,
				bindingtype: "home",
				token: app_token,
				iscallback: false,
				granttimes:1
			};
			
			mico2.addDeviceByVerCode(param, function(ret, err) {
				try {
					if(ret && (ret.meta.code == 0)) {
						alert("添加成功");
						send_event('refesh_device_list', 'device_add');
					} else {
						if(ret.meta.code == 23102) {
							alert("您已经是超级用户,无需添加此设备");
						} else {
							alert('添加设备失败:' + JSON.stringify(err) + JSON.stringify(err));
						}
					}
				} catch(e) {
					alert("添加失败,请重试");
				}
			});

		} else if(ret.eventType == "fail") {
			alert("图片识别失败,请重试");
		} else {
			//....
		}
	});
}

//生成二维码
function encodeImg(param) {
	var arr = param.info.split(',');

	var app_token = check_app_token();

	if(monitor_net() == "none") {
		return false;
	}

	var vercode_param = {
		deviceid: arr[0],
		token: app_token
	};

	mico2.getShareVerCode(vercode_param, function(ret, err) {
		if(ret && ret.meta.code == 0) {
			//获取vercode成功
			var vercode = ret.data.vercode;

			var encode_param = {
				appid:_APPID,
				device_id: arr[0],
				device_pw: arr[1],
				//device_name: arr[2], //设备名称不需要放在二维码中，并且有中文时,从相册中添加会失败
				bind_vercode: vercode,
				role: 3 //授权等级 3普通用户 2管理员
			}

			var qr_code = JSON.stringify(encode_param);
			
			//alert(JSON.stringify(encode_param));

			FNScanner.encodeImg({
				content: qr_code, //二维码内容 string类型
				saveToAlbum: true,
				saveImg: {
					path: 'fs://mico_QR_pic/album.png',
					w: 220,
					h: 220
				}
			}, function(ret, err) {
				if(ret.status) {
					var imgPth = ret.imgPath; //二维码图片路径
					document.getElementById('device_share_qr_img').src = imgPth;
					document.getElementById('share_qr_img_txt').innerHTML = "二维码图片已保存在您的相册中<br/>可分享二维码给其他用户";
					return true;
				} else {
					mui.toast(JSON.stringify(err)); //失败回调
					return false;
				}
			});

		} else {
			//alert("分享二维码失败\n" + ret.meta.code + ':' + ret.meta.message); 
			alert("分享二维码失败\n1.网络异常\n2.权限不足");

			return false;
		}
	});

}