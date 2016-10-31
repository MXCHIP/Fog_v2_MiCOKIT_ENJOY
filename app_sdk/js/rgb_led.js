//add the colorpicker to the canvas
//亮度和饱和度
var bsliderMask;
var ssliderMask;
var width = 0.69 * document.body.clientWidth;
//图片像素是80px，一半为40px
var switchbttop = width / 2;
var switchbleft = (document.body.clientWidth / 2) - 40;
//上次的color指数
var lastrgbcode;

//确定拾色器的大小
$('#canvas_colorPicker').attr('width', width).attr('height', width);
//确定开关的位置
$('#rgb_Switch').css('top', switchbttop).css('left', switchbleft);

//返回当前页面的hsv值 h:0-360 s:0-100 v:0-100
function get_rgb_led_hsv() {
	var rgb_led_h = parseInt(colorPicker.getColorHSL().h * 360);
	var rgb_led_s = parseInt(document.getElementById("rgb_saturation").value);
	var rgb_led_v = parseInt(document.getElementById("rgb_brightness").value);

	return [rgb_led_h, rgb_led_s, rgb_led_v]
}

//实例化
var colorPicker = new colorPicker(document.getElementById('canvas_colorPicker'), {
	onFinishClick: function() { //完成点击后发送控制命令
		if(get_switch_onoff() == false) {
			alert("请先打开中心开关");
			return;
		}

		send_rgb_command_to_device();
	}
});

//获取当前开关状态  
function get_switch_onoff() {
	var btnsrc = $("#rgb_Switch").attr("src");

	if("../img/switchon.svg" == btnsrc) {
		return true;
	} else {
		return false;
	}

}

//rgb开关控制
$("#rgb_Switch").click(function() {
	var rgb_data = get_rgb_led_hsv();
	var btnsrc = $("#rgb_Switch").attr("src");
	var app_command_id;
		
	if("../img/switchon.svg" == btnsrc) {
		$("#rgb_Switch").attr("src", "../img/switchoff.svg");
		
		//发送数据到设备
		send_rgb_command_to_device();
	} else {
		$("#rgb_Switch").attr("src", "../img/switchon.svg");
		
		//发送数据到设备
		send_rgb_command_to_device();
	}
});

//亮度范围：0-100
function brightness_slider_onChanged(brightness_obj) {
	if(get_switch_onoff() == false) {
		alert("请先打开中心开关");
		return;
	}

	send_rgb_command_to_device();

//	mui.toast("brightness=" + brightness_obj.value);
}

//饱和度范围：0-100
function saturation_slider_onChanged(saturation_obj) {
	if(get_switch_onoff() == false) {
		alert("请先打开中心开关");
		return;
	}

	send_rgb_command_to_device();
//	mui.toast("saturation=" + saturation_obj.value);
}

//关闭当前页面
function quit_rgbled_html() {
	closeWin();
}

//发送RGB命令到设备端
function send_rgb_command_to_device() {
	var rgb_data = get_rgb_led_hsv();

	//发送数据到设备
	var app_command_id = get_app_command_id();
	var rgb_led_onoff = get_switch_onoff();

	var data_array = new Array();
	data_array[0] = {
		PID: api.pageParam.pid,
		T: api.pageParam.type,
		D: {
			switch:rgb_led_onoff,
			h:rgb_data[0],
			s:rgb_data[1],
			v:rgb_data[2]
		}
	}
	
	var app_command = {
		command_id: app_command_id,
		CMD: C2D_SEND_COMMANDS,
		G: data_array
	}
	
	//alert(JSON.stringify(app_command));
	send_message_to_device(JSON.stringify(app_command));
}

//RGBLED页面监听Android返回按键
function rgbled_page_listen_keyback(){
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		quit_rgbled_html();
	});
}