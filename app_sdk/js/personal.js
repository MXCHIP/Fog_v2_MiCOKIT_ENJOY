//显示反馈信息
function display_info_feedback(){
	var info = "请前往www.mico.io反馈意见";
	
	mui.confirm(info, "反馈", ["确定"], function(ret) {
	});
}

//显示版本信息
function display_info_version(){
	var info = "MiCO总动员 V2.1";
	
	mui.confirm(info, "版本", ["确定"], function(ret) {
	});
}